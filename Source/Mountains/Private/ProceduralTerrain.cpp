#include "ProceduralTerrain.h"
#include "Async/ParallelFor.h"
#include "Async/Async.h"

AProceduralTerrain::AProceduralTerrain()
{
    PrimaryActorTick.bCanEverTick = false;
    CustomMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CustomMesh"));
    RootComponent = CustomMesh;

    // Enable async mesh collision cooking for better performance
    CustomMesh->bUseAsyncCooking = true;

    // Biome parameter setup
    PlainsBiome.GlobalHeightScale = 1200.0f;
    PlainsBiome.CoordinateScale = 0.0001f;
    PlainsBiome.GrassColor = FLinearColor(0.12f, 0.4f, 0.12f);
    PlainsBiome.RockColor = FLinearColor(0.25f, 0.2f, 0.15f);

    MountainBiome.GlobalHeightScale = 7500.0f;
    MountainBiome.CoordinateScale = 0.0003f;
    MountainBiome.GrassColor = FLinearColor(0.05f, 0.2f, 0.08f);
    MountainBiome.RockColor = FLinearColor(0.12f, 0.12f, 0.14f);
}

void AProceduralTerrain::BeginPlay()
{
    Super::BeginPlay();
    CreateTerrainGeometry();
}

void AProceduralTerrain::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    CreateTerrainGeometry();
}

// 2D hash for noise
float AProceduralTerrain::Hash2D(FVector2D p) {
    return FMath::Frac(FMath::Sin(FVector2D::DotProduct(p, FVector2D(127.1f, 311.7f))) * 43758.5453123f);
}

// Value noise with analytical derivatives for fBm
FVector3f AProceduralTerrain::ValueNoiseWithDerivatives(FVector2D p)
{
    FVector2D i = FVector2D(FMath::FloorToFloat(p.X), FMath::FloorToFloat(p.Y));
    FVector2D f = FVector2D(FMath::Frac(p.X), FMath::Frac(p.Y));

    // Quintic interpolation and its derivative
    FVector2D u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
    FVector2D du = 30.0f * f * f * (f * (f - 2.0f) + 1.0f);

    // Lerp 4 corners
    float a = Hash2D(i + FVector2D(0.0f, 0.0f));
    float b = Hash2D(i + FVector2D(1.0f, 0.0f));
    float c = Hash2D(i + FVector2D(0.0f, 1.0f));
    float d = Hash2D(i + FVector2D(1.0f, 1.0f));

    float Height = a + (b - a) * u.X + (c - a) * u.Y + (a - b - c + d) * u.X * u.Y;
    float dx = du.X * (b - a + (a - b - c + d) * u.Y);
    float dy = du.Y * (c - a + (a - b - c + d) * u.X);

    return FVector3f(Height, dx, dy);
}

// Multi-octave fBm with ridge shaping and warping for biome height
float AProceduralTerrain::CalculateBiomeHeight(FVector2D BaseCoords, const FBiomeSettings& Biome)
{
    FVector2D ScaledSampleCoords = BaseCoords * Biome.CoordinateScale;
    float AccumulatedHeight = 0.0f;
    float Amplitude = 1.0f;
    FVector2D DerivativeSum = FVector2D(0.0f, 0.0f);

    for (int32 Octave = 0; Octave < Biome.Octaves; Octave++)
    {
        // Domain warping for more natural terrain
        FVector2D WarpedCoords = ScaledSampleCoords + DerivativeSum * Biome.WarpStrength;
        FVector3f NoiseResult = ValueNoiseWithDerivatives(WarpedCoords);

        // Ridge noise: invert and sharpen
        float RidgeShape = 1.0f - FMath::Abs(NoiseResult.X);
        RidgeShape = FMath::Max(0.0f, RidgeShape - Biome.SharpnessOffset);
        RidgeShape = RidgeShape * RidgeShape;

        // Slope-based amplitude reduction
        float SlopeDenom = 1.0f + FVector2D::DotProduct(DerivativeSum, DerivativeSum);
        AccumulatedHeight += Amplitude * RidgeShape / SlopeDenom;

        DerivativeSum += FVector2D(NoiseResult.Y, NoiseResult.Z);
        Amplitude *= Biome.AmplitudeDecay;

        // Rotate and scale for decorrelation between octaves
        float RotatedX = (0.8f * ScaledSampleCoords.X + 0.6f * ScaledSampleCoords.Y) * 2.1f;
        float RotatedY = (-0.6f * ScaledSampleCoords.X + 0.8f * ScaledSampleCoords.Y) * 2.1f;
        ScaledSampleCoords = FVector2D(RotatedX, RotatedY);
    }

    return AccumulatedHeight * Biome.GlobalHeightScale;
}

void AProceduralTerrain::CreateTerrainGeometry()
{
    // Step 1: Capture state for thread safety
    const int32 LocalGridSize = GridSize;
    const float LocalGridSpacing = GridSpacing;
    const bool LocalbUseWorldLocationOffset = bUseWorldLocationOffset;
    const FVector2D LocalGlobalNoiseOffset = GlobalNoiseOffset;
    const FBiomeSettings LocalPlainsBiome = PlainsBiome;
    const FBiomeSettings LocalMountainBiome = MountainBiome;
    const float LocalBiomeScale = BiomeScale;
    const FTransform ActorTransform = GetActorTransform();
    const bool bIsGameWorld = GetWorld() && GetWorld()->IsGameWorld();

    // Step 2: Heavy mesh generation task (can run async)
    auto HeavyGenerationTask = [=, this]()
        {
            int32 TotalVertices = LocalGridSize * LocalGridSize;
            int32 TotalTriangles = (LocalGridSize - 1) * (LocalGridSize - 1) * 6;

            // Step 2.1: Pre-allocate arrays for parallel writes
            TArray<FVector> Vertices;     Vertices.SetNumUninitialized(TotalVertices);
            TArray<FVector2D> UV0;        UV0.SetNumUninitialized(TotalVertices);
            TArray<FVector> Normals;      Normals.SetNumUninitialized(TotalVertices);
            TArray<FColor> VertexColors;  VertexColors.SetNumUninitialized(TotalVertices);
            TArray<int32> Triangles;      Triangles.SetNumUninitialized(TotalTriangles);

            // OPTIMIZATION: Cache biome weights to avoid recomputation
            TArray<float> CachedBiomeWeights; CachedBiomeWeights.SetNumUninitialized(TotalVertices);

            // Step 2.2: Parallel vertex and height generation
            ParallelFor(TotalVertices, [&](int32 Index)
                {
                    int32 y = Index / LocalGridSize;
                    int32 x = Index % LocalGridSize;

                    float VertexX = x * LocalGridSpacing;
                    float VertexY = y * LocalGridSpacing;

                    FVector2D SampleCoords;
                    if (LocalbUseWorldLocationOffset)
                    {
                        FVector WorldPos = ActorTransform.TransformPosition(FVector(VertexX, VertexY, 0.0f));
                        SampleCoords = FVector2D(WorldPos.X, WorldPos.Y);
                    }
                    else
                    {
                        SampleCoords = FVector2D(VertexX + LocalGlobalNoiseOffset.X, VertexY + LocalGlobalNoiseOffset.Y);
                    }

                    // Biome blending: smoothstep between plains and mountain
                    float BiomeNoise = ValueNoiseWithDerivatives(SampleCoords * LocalBiomeScale).X;
                    float BiomeWeight = FMath::SmoothStep(0.35f, 0.65f, BiomeNoise);
                    CachedBiomeWeights[Index] = BiomeWeight;

                    float PlainsHeight = CalculateBiomeHeight(SampleCoords, LocalPlainsBiome);
                    float MountainHeight = CalculateBiomeHeight(SampleCoords, LocalMountainBiome);
                    float FinalHeight = FMath::Lerp(PlainsHeight, MountainHeight, BiomeWeight);

                    Vertices[Index] = FVector(VertexX, VertexY, FinalHeight);
                    UV0[Index] = FVector2D((float)x / (LocalGridSize - 1), (float)y / (LocalGridSize - 1));
                });

            // Step 2.3: Parallel normals and color calculation
            ParallelFor(TotalVertices, [&](int32 Index)
                {
                    int32 y = Index / LocalGridSize;
                    int32 x = Index % LocalGridSize;

                    int32 LeftIdx = (x > 0) ? (Index - 1) : Index;
                    int32 RightIdx = (x < LocalGridSize - 1) ? (Index + 1) : Index;
                    int32 DownIdx = (y > 0) ? (Index - LocalGridSize) : Index;
                    int32 UpIdx = (y < LocalGridSize - 1) ? (Index + LocalGridSize) : Index;

                    float StepX = (x > 0 && x < LocalGridSize - 1) ? (2.0f * LocalGridSpacing) : LocalGridSpacing;
                    float StepY = (y > 0 && y < LocalGridSize - 1) ? (2.0f * LocalGridSpacing) : LocalGridSpacing;

                    // Numerical/finite difference normal calculation
                    FVector TangentX = FVector(StepX, 0.0f, Vertices[RightIdx].Z - Vertices[LeftIdx].Z);
                    FVector TangentY = FVector(0.0f, StepY, Vertices[UpIdx].Z - Vertices[DownIdx].Z);
                    FVector CalculatedNormal = FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
                    Normals[Index] = CalculatedNormal;

                    // Use cached biome weight for color blending
                    float BiomeWeight = CachedBiomeWeights[Index];
                    FLinearColor BiomeGrass = FMath::Lerp(LocalPlainsBiome.GrassColor, LocalMountainBiome.GrassColor, BiomeWeight);
                    FLinearColor BiomeRock = FMath::Lerp(LocalPlainsBiome.RockColor, LocalMountainBiome.RockColor, BiomeWeight);

                    // Slope-based color blend
                    float SlopeIntensity = FVector::DotProduct(CalculatedNormal, FVector::UpVector);
                    float SlopeFactor = FMath::Clamp((SlopeIntensity - 0.6f) / 0.2f, 0.0f, 1.0f);
                    FLinearColor FinalColor = FMath::Lerp(BiomeRock, BiomeGrass, SlopeFactor);
                    VertexColors[Index] = FinalColor.ToFColor(false);
                });

            // Step 2.4: Generate triangle indices
            int32 TriIdx = 0;
            for (int32 y = 0; y < LocalGridSize - 1; y++)
            {
                for (int32 x = 0; x < LocalGridSize - 1; x++)
                {
                    int32 Row1 = x + (y * LocalGridSize);
                    int32 Row2 = x + ((y + 1) * LocalGridSize);

                    Triangles[TriIdx++] = Row1;
                    Triangles[TriIdx++] = Row2;
                    Triangles[TriIdx++] = Row1 + 1;

                    Triangles[TriIdx++] = Row1 + 1;
                    Triangles[TriIdx++] = Row2;
                    Triangles[TriIdx++] = Row2 + 1;
                }
            }

            // Step 2.5: Apply mesh on game thread
            auto ApplyMeshToComponentTask = [this,
                MoveVertices = MoveTemp(Vertices),
                MoveTriangles = MoveTemp(Triangles),
                MoveNormals = MoveTemp(Normals),
                MoveUVs = MoveTemp(UV0),
                MoveColors = MoveTemp(VertexColors)]() mutable
                {
                    if (IsValid(this) && CustomMesh)
                    {
                        CustomMesh->ClearAllMeshSections();
                        TArray<FProcMeshTangent> Tangents;
                        CustomMesh->CreateMeshSection(0, MoveVertices, MoveTriangles, MoveNormals, MoveUVs, MoveColors, Tangents, true);

                        if (TerrainMaterial != nullptr)
                        {
                            CustomMesh->SetMaterial(0, TerrainMaterial);
                        }
                    }
                };

            if (bIsGameWorld)
            {
                // OPTIMIZATION: Run mesh application on game thread after async generation
                AsyncTask(ENamedThreads::GameThread, MoveTemp(ApplyMeshToComponentTask));
            }
            else
            {
                // Editor: run synchronously
                ApplyMeshToComponentTask();
            }
        };

    // Step 3: Run heavy generation async in game, sync in editor
    if (bIsGameWorld)
    {
        // OPTIMIZATION: Offload mesh generation to thread pool
        Async(EAsyncExecution::ThreadPool, MoveTemp(HeavyGenerationTask));
    }
    else
    {
        HeavyGenerationTask();
    }
}
