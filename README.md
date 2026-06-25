# Procedural Mountain Generation in Unreal Engine 5

**Real-time Terrain with Analytical Derivatives of Value Noise, FBm and Biome Blending**

*Max Sergueev — Graphics Programmer - ArtFx — June 2026*

<img width="2117" height="1135" alt="CPUNormalsCapture" src="https://github.com/user-attachments/assets/1cc78b9e-fc2d-4842-8380-d4c730b129d0" />

---


## Introduction

In computer graphics, there are two primary methods of rendering: offline and real-time. While offline renders can take time to create each frame in intricate detail, 
real-time solutions are expected to produce convincing scenes at 60 frames per second while competing for computational resources with physics and gameplay systems. 
This project aims to explore mountain and terrain generation in Unreal Engine 5, leveraging C++ thread pooling, chunk streaming and derivative-driven value noise. 
This research evaluates the viability of creating highly detailed, distinct biomes entirely on the CPU, and later identifying structural bottlenecks that can be resolved
with a modern GPU hybrid pipeline.

---

## State of The Art

Procedural terrain generation in real-time games has generally landed on a common architecture: a chunked infinite world where terrain is generated on demand as the player moves.
Minecraft's world generation system is a widely known example of this. Terrain is produced in 16x16x384 column chunks, generated asynchronously and loaded into the scene through a queue that 
is drained a bounded number of chunks per tick, which directly inspired my `TerrainChunkManager`. Minecraft's older biome system assigned distinct height and feature parameters to regions of the world that would blend smoothly at the borders, which motivated the two-biome smoothstep approach used in this project.

The standard noise for procedural terrain is Perlin Noise, which assigns gradient vectors to grid points and interpolates between them. 
Unlike Perlin noise, value noise uses simpler scalar values, making it faster to compute the exact derivatives simply by taking the differential of the polynomial used for interpolation.
Ken Perlin's improved noise formulation established a quintic polynomial $6x^5 - 15x^4 + 10x^3$ whose continuous second derivative enables unbroken slope vectors when differentiated 
(see *Figure 1*). Inigo Quilez formalized this observation in his article More Noise, showing that the exact dx and dy can be extracted without additional sampling of neighboring points by deriving the polynomial used to interpolate between the grid points.
This article was the main inspiration for this project, initially encountered through Josh's Channel's video on procedural mountain techniques.
I wanted to apply what I learned from it to an Unreal Engine 5 project.

<img width="690" height="433" alt="noise-discontinuity2" src="https://github.com/user-attachments/assets/e3d917aa-c2a0-4fc4-b6ea-fa0a1ecc824e" />

> *Figure 1: Artifacts along cell boundaries using smoothstep vs. quintic interpolation (Image source: Scratchapixel).*

---

## Approach

Value noise was chosen over Perlin noise because its analytical derivatives are a direct product of the quintic interpolation step. 
The function `ValueNoiseWithDerivatives()` computes the interpolation weights *u* and their derivatives *du* simultaneously, returning height, dx, and dy in a single pass.

These derivatives drive the fBm loop in `CalculateBiomeHeight()` in two ways:

- **Domain warping** displaces the sampling coordinates each octave by the accumulated derivative sum scaled by a warp strength,
  which can give an erosion-like effect on the slopes and ridges of the mountain.
  ```cpp
  FVector2D WarpedCoords = ScaledSampleCoords + DerivativeSum * Biome.WarpStrength;
  ```
  <img width="436" height="234" alt="Warping" src="https://github.com/user-attachments/assets/327a32ab-2a62-47c1-85ff-407f0f8b2ea9" />

  > *Figure 2: Demonstration of warping by changing the WarpStrength variable in editor.*
  
- **Slope-based amplitude suppression** divides each octave's contribution by one plus the squared magnitude of the derivative sum, mimicking how sediment does not stick to
  steep surfaces and settles on flat ground.
  ```cpp
  float SlopeDenom = 1.0f + FVector2D::DotProduct(DerivativeSum, DerivativeSum);
  AccumulatedHeight += Amplitude * RidgeShape / SlopeDenom;
  ```
  <img width="436" height="234" alt="AmplitudeDecay" src="https://github.com/user-attachments/assets/a025a47a-a5bb-46d8-802f-a051a999b180" />

  > *Figure 3: Demonstration of Slopes being less affected by changes to the Amplitude Decay variable, done in editor.*

Additionally, **ridge shaping** (inverting and squaring the noise) can help produce the sharp peaks of younger mountains rather than the smooth tops of hills or very old mountains.
The resulting mountains from the process described above can be seen below in *Figure 5*.
  ```cpp
  float RidgeShape = 1.0f - FMath::Abs(NoiseResult.X);
  RidgeShape = FMath::Max(0.0f, RidgeShape - Biome.SharpnessOffset);
  RidgeShape = RidgeShape * RidgeShape;
  ```
<img width="436" height="234" alt="Sharpness" src="https://github.com/user-attachments/assets/884a8cfe-b6ae-4245-bc25-e2ba66903562" />

> *Figure 4: Demonstration of how the ridge shape and height changes with the SharpnessOffset variable, done in editor.*

The system currently has two implemented biomes (see cover image) which can be easily extended. They are blended using a smoothstep-filtered noise value as a spatial weight. 
`ATerrainChunkManager` tracks the player position each tick, and queues new chunks that are now in range to spawn. It uses the variable `MaxChunksSpawnedPerFrame` to spread the load of
spawning new chunks across multiple ticks. Heavy mesh generation is offloaded to the Unreal thread pool asynchronously and only the final mesh upload runs on the game thread.

<img width="1318" height="813" alt="FarPic" src="https://github.com/user-attachments/assets/229f9300-05c2-4eb4-bf4c-e73bc4065df2" />

> *Figure 5: Procedural Mesh after Domain warping, amplitude suppression, ridge shaping and several octaves of noise*
---

## Analysis

### Normal Calculation Problem

Though the main focus of this project was to use analytical derivatives for every step, this approach could not be extended to computing the mesh normals. 
Despite `ValueNoiseWithDerivatives()` providing exact dx and dy, the normals in `CreateTerrainGeometry()` are computed with the typical finite difference approach, 
sampling neighbors on the vertex grid:

```cpp
float StepX = (x > 0 && x < LocalGridSize - 1) ? (2.0f * LocalGridSpacing) : LocalGridSpacing;
float StepY = (y > 0 && y < LocalGridSize - 1) ? (2.0f * LocalGridSpacing) : LocalGridSpacing;

FVector TangentX = FVector(StepX, 0.0f, Vertices[RightIdx].Z - Vertices[LeftIdx].Z);
FVector TangentY = FVector(0.0f, StepY, Vertices[UpIdx].Z - Vertices[DownIdx].Z);
FVector CalculatedNormal = FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
```

The reason for this is that the analytical derivatives are only valid for a single noise call at a single scale. 
The final vertex height is determined by linear interpolation when blending the biomes:

```cpp
FinalHeight = Lerp(PlainsHeight, MountainHeight, BiomeWeight)
```

Each of `PlainsHeight` and `MountainHeight` is the output of a multi-octave fBm loop in which:

- The input coordinates are warped each octave by the running derivative sum, creating a recursive dependency between octaves.
- The noise output is ridge-shaped via a non-linear operation (invert, clip, square) that introduces regions of non-differentiability.
- Each octave contribution is divided by the accumulated slope magnitude.

In addition, `BiomeWeight` is also an independently-evaluated, smoothstep-filtered noise value.

Propagating analytical derivatives through this composite would require applying the full chain rule across all nested operations, and would need to be re-derived entirely if the height noise function were to change.

This problem with the normals came from a flawed assumption about how the reference, Inigo Quilez's More Noise article, would apply to my context. I did not initially take into account the difference between a raymarched terrain vs. a discrete grid of vertices and tried to directly apply what I had learned from Inigo Quilez to a fundamentally mismatched system. This led me to the solution described above. However, for even better results there are some potential future improvements.

---

## Future Approaches

- **Decoupled Normal Map:** Generate a separate, high resolution normal pass evaluating the height function on a denser grid than the vertex grid used to construct the mesh.
  This normal map could then be sampled in a material. This decouples visual normal resolution from the mesh polygon count.
- **Hybrid GPU Compute Pipeline:** Height generation is an embarrassingly parallel problem as each vertex is independent of all other vertices.
  A GPU compute shader could process the same data orders of magnitude faster, at larger grid scales and more octave layers. Macro shapes of the terrain could be computed on the CPU for the collision,
  and the high-frequency micro detail can be offloaded to the GPU.

---

## Bibliography

- Josh's Channel. (2024, April 1). *Better Mountain Generators That Aren't Perlin Noise or Erosion*. YouTube. https://www.youtube.com/watch?v=gsJHzBTPG0Y
- Quilez, I. (n.d.). *More Noise*. https://iquilezles.org/articles/morenoise/
- Vivo, P. G., & Lowe, J. (2015). *The Book of Shaders, Chapter 13*. https://thebookofshaders.com/13/
- Scratchapixel. (n.d.). *Improved Perlin Noise*. https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/improved-perlin-noise.html
- Minecraft Wiki contributors. (n.d.). *World generation*. Minecraft Wiki. https://minecraft.wiki/w/World_generation

