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

void AProceduralTerrain::CreateTerrainGeometry()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    // Generate Vertices, UVs, and Colors
    for (int32 y = 0; y < GridSize; y++)
    {
        for (int32 x = 0; x < GridSize; x++)
        {
            float VertexX = x * GridSpacing;
            float VertexY = y * GridSpacing;
            float VertexZ = 0.0f;

            Vertices.Add(FVector(VertexX, VertexY, VertexZ));
            UV0.Add(FVector2D((float)x / GridSize, (float)y / GridSize));
            VertexColors.Add(FColor::White);
        }
    }

    // Generate Index Triangles
    for (int32 y = 0; y < GridSize - 1; y++)
    {
        for (int32 x = 0; x < GridSize - 1; x++)
        {
            int32 Row1 = x + (y * GridSize);
            int32 Row2 = x + ((y + 1) * GridSize);

            // Triangle 1
            Triangles.Add(Row1);
            Triangles.Add(Row2);
            Triangles.Add(Row1 + 1);

            // Triangle 2
            Triangles.Add(Row1 + 1);
            Triangles.Add(Row2);
            Triangles.Add(Row2 + 1);
        }
    }

    // Bake Mesh Section
    CustomMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
}