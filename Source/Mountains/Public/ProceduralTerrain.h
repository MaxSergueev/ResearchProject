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
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    UProceduralMeshComponent* CustomMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Grid Setup", meta = (UIMin = "50", UIMax = "1000", ClampMin = "2", ClampMax = "2000"))
    int32 GridSize = 400;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Grid Setup", meta = (UIMin = "10", UIMax = "200"))
    float GridSpacing = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Grid Setup", meta = (UIMin = "1000", UIMax = "15000"))
    float GlobalHeightScale = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Coordinate Offset")
    bool bUseWorldLocationOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Coordinate Offset", meta = (EditCondition = "!bUseWorldLocationOffset"))
    FVector2D GlobalNoiseOffset = FVector2D(0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Noise Settings", meta = (UIMin = "0.00005", UIMax = "0.001"))
    float CoordinateScale = 0.0002f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Noise Settings", meta = (UIMin = "1", UIMax = "12", ClampMin = "1", ClampMax = "16"))
    int32 Octaves = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Noise Settings", meta = (UIMin = "0.1", UIMax = "0.8"))
    float AmplitudeDecay = 0.48f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Erosion Settings", meta = (UIMin = "0.0", UIMax = "1.0"))
    float WarpStrength = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Erosion Settings", meta = (UIMin = "0.0", UIMax = "0.6"))
    float SharpnessOffset = 0.2f;

    float Hash2D(FVector2D p);
    FVector3f ValueNoiseWithDerivatives(FVector2D p);
    void CreateTerrainGeometry();
};