#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralTerrain.generated.h"

UCLASS()
class MOUNTAINS_API AProceduralTerrain : public AActor
{
    GENERATED_BODY()

public:
    AProceduralTerrain();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
        UProceduralMeshComponent* CustomMesh;

    // Configuration Variables
    int32 GridSize = 500;     // Number of vertices on X and Y axes
    float GridSpacing = 100.f; // Distance between vertices

    // Returns height (x) and derivatives (y=dx, z=dy) - IQ blog ref
    FVector3f ValueNoiseWithDerivatives(FVector2D p);

    void CreateTerrainGeometry();
};