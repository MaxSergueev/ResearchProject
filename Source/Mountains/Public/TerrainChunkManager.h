#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrain.h"
#include "TerrainChunkManager.generated.h"

UCLASS()
class MOUNTAINS_API ATerrainChunkManager : public AActor
{
    GENERATED_BODY()

public:
    ATerrainChunkManager();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    // Terrain chunk class to spawn
    UPROPERTY(EditAnywhere, Category = "Terrain Management")
    TSubclassOf<AProceduralTerrain> TerrainChunkClass;

    // Number of chunks to spawn around the viewer (2 = 5x5 grid)
    UPROPERTY(EditAnywhere, Category = "Terrain Management")
    int32 ChunkDrawDistance = 2;

    // World size of a single chunk
    UPROPERTY(EditAnywhere, Category = "Terrain Management")
    float ChunkWorldSize = 19950.0f;

    // Actor to track for chunk placement
    UPROPERTY(VisibleAnywhere, Category = "Terrain Management")
    AActor* Viewer;

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void UpdateChunks();

    UFUNCTION(CallInEditor, Category = "Terrain Management|Editor Tools")
    void GeneratePreview();

    UFUNCTION(CallInEditor, Category = "Terrain Management|Editor Tools")
    void ClearChunks();

    // Max chunks spawned per frame
    UPROPERTY(EditAnywhere, Category = "Terrain Management", meta = (ClampMin = "1"))
    int32 MaxChunksSpawnedPerFrame = 1;

private:
    // Active chunks by grid coordinate
    UPROPERTY()
    TMap<FIntPoint, AProceduralTerrain*> ActiveChunks;

    // Coordinates queued for spawning
    TArray<FIntPoint> SpawnQueue;

    FIntPoint CurrentViewerChunkCoord;
    FIntPoint GetChunkCoordinate(FVector Location) const;

    bool SpawnChunk(FIntPoint TargetCoord);
};

