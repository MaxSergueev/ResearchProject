#include "ProceduralTerrain.h"
#include "Async/ParallelFor.h"

AProceduralTerrain::AProceduralTerrain()
{
    PrimaryActorTick.bCanEverTick = false;
    CustomMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CustomMesh"));
    RootComponent = CustomMesh;

    // Distinct values for the two biomes
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

float AProceduralTerrain::Hash2D(FVector2D p) {
    return FMath::Frac(FMath::Sin(FVector2D::DotProduct(p, FVector2D(127.1f, 311.7f))) * 43758.5453123f);
}

FVector3f AProceduralTerrain::ValueNoiseWithDerivatives(FVector2D p)
{
    FVector2D i = FVector2D(FMath::FloorToFloat(p.X), FMath::FloorToFloat(p.Y));
    FVector2D f = FVector2D(FMath::Frac(p.X), FMath::Frac(p.Y));

    FVector2D u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
    FVector2D du = 30.0f * f * f * (f * (f - 2.0f) + 1.0f);

    float a = Hash2D(i + FVector2D(0.0f, 0.0f));
    float b = Hash2D(i + FVector2D(1.0f, 0.0f));
    float c = Hash2D(i + FVector2D(0.0f, 1.0f));
    float d = Hash2D(i + FVector2D(1.0f, 1.0f));

    float Height = a + (b - a) * u.X + (c - a) * u.Y + (a - b - c + d) * u.X * u.Y;

    float dx = du.X * (b - a + (a - b - c + d) * u.Y);
    float dy = du.Y * (c - a + (a - b - c + d) * u.X);

    return FVector3f(Height, dx, dy);
}

float AProceduralTerrain::CalculateBiomeHeight(FVector2D BaseCoords, const FBiomeSettings& Biome)
{
    FVector2D ScaledSampleCoords = BaseCoords * Biome.CoordinateScale;
    float AccumulatedHeight = 0.0f;
    float Amplitude = 1.0f;
    FVector2D DerivativeSum = FVector2D(0.0f, 0.0f);

    for (int32 Octave = 0; Octave < Biome.Octaves; Octave++)
    {
        FVector2D WarpedCoords = ScaledSampleCoords + DerivativeSum * Biome.WarpStrength;
        FVector3f NoiseResult = ValueNoiseWithDerivatives(WarpedCoords);

        float RidgeShape = 1.0f - FMath::Abs(NoiseResult.X);
        RidgeShape = FMath::Max(0.0f, RidgeShape - Biome.SharpnessOffset);
        RidgeShape = RidgeShape * RidgeShape;

        float SlopeDenom = 1.0f + FVector2D::DotProduct(DerivativeSum, DerivativeSum);
        AccumulatedHeight += Amplitude * RidgeShape / SlopeDenom;

        DerivativeSum += FVector2D(NoiseResult.Y, NoiseResult.Z);
        Amplitude *= Biome.AmplitudeDecay;

        float RotatedX = (0.8f * ScaledSampleCoords.X + 0.6f * ScaledSampleCoords.Y) * 2.1f;
        float RotatedY = (-0.6f * ScaledSampleCoords.X + 0.8f * ScaledSampleCoords.Y) * 2.1f;
        ScaledSampleCoords = FVector2D(RotatedX, RotatedY);
    }

    return AccumulatedHeight * Biome.GlobalHeightScale;
}

void AProceduralTerrain::CreateTerrainGeometry()
{
    CustomMesh->ClearAllMeshSections();

    int32 TotalVertices = GridSize * GridSize;
    int32 TotalTriangles = (GridSize - 1) * (GridSize - 1) * 6;

    FTransform ActorTransform = GetActorTransform();

    // --- STEP 1: PRE-ALLOCATE MEMORY ---
    TArray<FVector> Vertices;     Vertices.SetNumUninitialized(TotalVertices);
    TArray<FVector2D> UV0;        UV0.SetNumUninitialized(TotalVertices);
    TArray<FVector> Normals;      Normals.SetNumUninitialized(TotalVertices);
    TArray<FColor> VertexColors;  VertexColors.SetNumUninitialized(TotalVertices);
    TArray<int32> Triangles;      Triangles.SetNumUninitialized(TotalTriangles);
    TArray<FProcMeshTangent> Tangents;

    // --- STEP 2: PARALLEL VERTEX & HEIGHT GENERATION ---
    ParallelFor(TotalVertices, [this, &Vertices, &UV0, ActorTransform](int32 Index)
        {
            int32 y = Index / GridSize;
            int32 x = Index % GridSize;

            float VertexX = x * GridSpacing;
            float VertexY = y * GridSpacing;

            FVector2D SampleCoords;
            if (bUseWorldLocationOffset)
            {
                FVector WorldPos = ActorTransform.TransformPosition(FVector(VertexX, VertexY, 0.0f));
                SampleCoords = FVector2D(WorldPos.X, WorldPos.Y);
            }
            else
            {
                SampleCoords = FVector2D(VertexX + GlobalNoiseOffset.X, VertexY + GlobalNoiseOffset.Y);
            }

            // Sample global biome map noise (returns 0.0 to 1.0 based on Hash2D's Frac)
            float BiomeNoise = ValueNoiseWithDerivatives(SampleCoords * BiomeScale).X;
            float BiomeWeight = FMath::SmoothStep(0.35f, 0.65f, BiomeNoise);

            // Calculate heights per biome and blend
            float PlainsHeight = CalculateBiomeHeight(SampleCoords, PlainsBiome);
            float MountainHeight = CalculateBiomeHeight(SampleCoords, MountainBiome);
            float FinalHeight = FMath::Lerp(PlainsHeight, MountainHeight, BiomeWeight);

            Vertices[Index] = FVector(VertexX, VertexY, FinalHeight);
            UV0[Index] = FVector2D((float)x / (GridSize - 1), (float)y / (GridSize - 1));
        });

    // --- STEP 3: PARALLEL NORMALS & COLORS CALCULATION ---
    ParallelFor(TotalVertices, [this, &Vertices, &Normals, &VertexColors, ActorTransform](int32 Index)
        {
            int32 y = Index / GridSize;
            int32 x = Index % GridSize;

            int32 LeftIdx = (x > 0) ? (Index - 1) : Index;
            int32 RightIdx = (x < GridSize - 1) ? (Index + 1) : Index;
            int32 DownIdx = (y > 0) ? (Index - GridSize) : Index;
            int32 UpIdx = (y < GridSize - 1) ? (Index + GridSize) : Index;

            float StepX = (x > 0 && x < GridSize - 1) ? (2.0f * GridSpacing) : GridSpacing;
            float StepY = (y > 0 && y < GridSize - 1) ? (2.0f * GridSpacing) : GridSpacing;

            FVector TangentX = FVector(StepX, 0.0f, Vertices[RightIdx].Z - Vertices[LeftIdx].Z);
            FVector TangentY = FVector(0.0f, StepY, Vertices[UpIdx].Z - Vertices[DownIdx].Z);

            FVector CalculatedNormal = FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
            Normals[Index] = CalculatedNormal;

            // Recompute Biome Weight for this vertex's world position to blend colors accurately
            float VertexX = x * GridSpacing;
            float VertexY = y * GridSpacing;
            FVector2D SampleCoords;
            if (bUseWorldLocationOffset)
            {
                FVector WorldPos = ActorTransform.TransformPosition(FVector(VertexX, VertexY, 0.0f));
                SampleCoords = FVector2D(WorldPos.X, WorldPos.Y);
            }
            else
            {
                SampleCoords = FVector2D(VertexX + GlobalNoiseOffset.X, VertexY + GlobalNoiseOffset.Y);
            }

            float BiomeNoise = ValueNoiseWithDerivatives(SampleCoords * BiomeScale).X;
            float BiomeWeight = FMath::SmoothStep(0.35f, 0.65f, BiomeNoise);

            // Interpolate biome colors dynamically
            FLinearColor BiomeGrass = FMath::Lerp(PlainsBiome.GrassColor, MountainBiome.GrassColor, BiomeWeight);
            FLinearColor BiomeRock = FMath::Lerp(PlainsBiome.RockColor, MountainBiome.RockColor, BiomeWeight);

            // Calculate slope-based rock vs grass blending
            float SlopeIntensity = FVector::DotProduct(CalculatedNormal, FVector::UpVector);
            float SlopeFactor = FMath::Clamp((SlopeIntensity - 0.6f) / 0.2f, 0.0f, 1.0f);

            FLinearColor FinalColor = FMath::Lerp(BiomeRock, BiomeGrass, SlopeFactor);
            VertexColors[Index] = FinalColor.ToFColor(false);
        });

    // --- STEP 4: INDEX TRIANGLES SEQUENTIALLY ---
    int32 TriIdx = 0;
    for (int32 y = 0; y < GridSize - 1; y++)
    {
        for (int32 x = 0; x < GridSize - 1; x++)
        {
            int32 Row1 = x + (y * GridSize);
            int32 Row2 = x + ((y + 1) * GridSize);

            Triangles[TriIdx++] = Row1;
            Triangles[TriIdx++] = Row2;
            Triangles[TriIdx++] = Row1 + 1;

            Triangles[TriIdx++] = Row1 + 1;
            Triangles[TriIdx++] = Row2;
            Triangles[TriIdx++] = Row2 + 1;
        }
    }

    // --- STEP 5: BAKE MESH SECTION ---
    CustomMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);

    // --- STEP 6: APPLY MATERIAL ---
    if (TerrainMaterial != nullptr)
    {
        CustomMesh->SetMaterial(0, TerrainMaterial);
    }
}