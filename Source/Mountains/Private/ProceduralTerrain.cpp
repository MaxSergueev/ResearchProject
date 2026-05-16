#include "ProceduralTerrain.h"

AProceduralTerrain::AProceduralTerrain()
{
    PrimaryActorTick.bCanEverTick = false;
    CustomMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CustomMesh"));
    RootComponent = CustomMesh;
}

void AProceduralTerrain::BeginPlay()
{
    Super::BeginPlay();
    CreateTerrainGeometry();
}

float AProceduralTerrain::Hash2D(FVector2D p) {
    return FMath::Frac(FMath::Sin(FVector2D::DotProduct(p, FVector2D(127.1f, 311.7f))) * 43758.5453123f);
}

void AProceduralTerrain::CreateTerrainGeometry()
{
    CustomMesh->ClearAllMeshSections();

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    // Generate vertices and UVs
    for (int32 y = 0; y < GridSize; y++)
    {
        for (int32 x = 0; x < GridSize; x++)
        {
            float VertexX = x * GridSpacing;
            float VertexY = y * GridSpacing;
            FVector2D ScaledSampleCoords = FVector2D(VertexX, VertexY) * 0.0002f;

            float AccumulatedHeight = 0.0f;
            float Amplitude = 1.0f;
            FVector2D DerivativeSum = FVector2D(0.0f, 0.0f);

            for (int32 Octave = 0; Octave < 10; Octave++)
            {
                FVector3f NoiseResult = ValueNoiseWithDerivatives(ScaledSampleCoords);

                DerivativeSum += FVector2D(NoiseResult.Y, NoiseResult.Z);
                AccumulatedHeight += Amplitude * NoiseResult.X / (1.0f + FVector2D::DotProduct(DerivativeSum, DerivativeSum));

                Amplitude *= 0.5f;

                float RotatedX = (0.8f * ScaledSampleCoords.X + 0.6f * ScaledSampleCoords.Y) * 2.0f;
                float RotatedY = (-0.6f * ScaledSampleCoords.X + 0.8f * ScaledSampleCoords.Y) * 2.0f;
                ScaledSampleCoords = FVector2D(RotatedX, RotatedY);
            }

            float TotalHeight = AccumulatedHeight * 4500.0f;

            Vertices.Add(FVector(VertexX, VertexY, TotalHeight));
            UV0.Add(FVector2D((float)x / (GridSize - 1), (float)y / (GridSize - 1)));

            Normals.Add(FVector::UpVector);
            VertexColors.Add(FColor::Black);
        }
    }

    // Calculate normals using the Numerical/Finite Difference method
    for (int32 y = 0; y < GridSize; y++)
    {
        for (int32 x = 0; x < GridSize; x++)
        {
            int32 Index = x + (y * GridSize);

            int32 LeftIdx = (x > 0) ? (Index - 1) : Index;
            int32 RightIdx = (x < GridSize - 1) ? (Index + 1) : Index;
            int32 DownIdx = (y > 0) ? (Index - GridSize) : Index;
            int32 UpIdx = (y < GridSize - 1) ? (Index + GridSize) : Index;

            float StepX = (x > 0 && x < GridSize - 1) ? (2.0f * GridSpacing) : GridSpacing;
            float StepY = (y > 0 && y < GridSize - 1) ? (2.0f * GridSpacing) : GridSpacing;

            // Numerical/Finite Difference: estimate surface tangents from neighbor heights
            FVector TangentX = FVector(StepX, 0.0f, Vertices[RightIdx].Z - Vertices[LeftIdx].Z);
            FVector TangentY = FVector(0.0f, StepY, Vertices[UpIdx].Z - Vertices[DownIdx].Z);

            FVector CalculatedNormal = FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
            Normals[Index] = CalculatedNormal;

            float SlopeIntensity = FVector::DotProduct(CalculatedNormal, FVector::UpVector);
            float BlendFactor = FMath::Clamp((SlopeIntensity - 0.6f) / 0.2f, 0.0f, 1.0f);

            FLinearColor RockColor = FLinearColor(0.15f, 0.15f, 0.15f);
            FLinearColor GrassColor = FLinearColor(0.1f, 0.35f, 0.1f);
            FLinearColor FinalColor = FMath::Lerp(RockColor, GrassColor, BlendFactor);

            VertexColors[Index] = FinalColor.ToFColor(true);
        }
    }

    // Generate triangles
    for (int32 y = 0; y < GridSize - 1; y++)
    {
        for (int32 x = 0; x < GridSize - 1; x++)
        {
            int32 Row1 = x + (y * GridSize);
            int32 Row2 = x + ((y + 1) * GridSize);

            Triangles.Add(Row1);
            Triangles.Add(Row2);
            Triangles.Add(Row1 + 1);

            Triangles.Add(Row1 + 1);
            Triangles.Add(Row2);
            Triangles.Add(Row2 + 1);
        }
    }

    CustomMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
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
