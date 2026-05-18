#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralTerrain.generated.h"

USTRUCT(BlueprintType)
struct FBiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings", meta = (UIMin = "0.00005", UIMax = "0.001"))
    float CoordinateScale = 0.0002f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings", meta = (UIMin = "1", UIMax = "12", ClampMin = "1", ClampMax = "16"))
    int32 Octaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings", meta = (UIMin = "0.1", UIMax = "0.8"))
    float AmplitudeDecay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Settings", meta = (UIMin = "0.0", UIMax = "1.0"))
    float WarpStrength = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Settings", meta = (UIMin = "0.0", UIMax = "0.6"))
    float SharpnessOffset = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Settings", meta = (UIMin = "1000", UIMax = "15000"))
    float GlobalHeightScale = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Colors")
    FLinearColor GrassColor = FLinearColor(0.1f, 0.35f, 0.1f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Colors")
    FLinearColor RockColor = FLinearColor(0.15f, 0.15f, 0.15f);
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Coordinate Offset")
    bool bUseWorldLocationOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Coordinate Offset", meta = (EditCondition = "!bUseWorldLocationOffset"))
    FVector2D GlobalNoiseOffset = FVector2D(0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Biomes")
    FBiomeSettings PlainsBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Biomes")
    FBiomeSettings MountainBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain | Biomes", meta = (UIMin = "0.000001", UIMax = "0.0001"))
    float BiomeScale = 0.00002f;

    float Hash2D(FVector2D p);
    FVector3f ValueNoiseWithDerivatives(FVector2D p);
    float CalculateBiomeHeight(FVector2D BaseCoords, const FBiomeSettings& Biome);
    void CreateTerrainGeometry();
};