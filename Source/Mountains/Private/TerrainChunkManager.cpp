#include "TerrainChunkManager.h"
#include "Kismet/GameplayStatics.h"

ATerrainChunkManager::ATerrainChunkManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
}

void ATerrainChunkManager::BeginPlay()
{
    Super::BeginPlay();
    // Set the viewer to the player pawn at game start
    Viewer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    UpdateChunks();
}

// Spawns a terrain chunk at the given grid coordinate if not already present
bool ATerrainChunkManager::SpawnChunk(FIntPoint TargetCoord)
{
    if (ActiveChunks.Contains(TargetCoord)) return false;

    FVector SpawnLocation = FVector(TargetCoord.X * ChunkWorldSize, TargetCoord.Y * ChunkWorldSize, 0.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProceduralTerrain* NewChunk = GetWorld()->SpawnActor<AProceduralTerrain>(TerrainChunkClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (NewChunk)
    {
        NewChunk->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        ActiveChunks.Add(TargetCoord, NewChunk);
        return true;
    }

    return false;
}

void ATerrainChunkManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Viewer)
    {
        FIntPoint ViewerCoord = GetChunkCoordinate(Viewer->GetActorLocation());

        // Only update if the viewer has moved to a new chunk
        if (ViewerCoord != CurrentViewerChunkCoord)
        {
            CurrentViewerChunkCoord = ViewerCoord;
            UpdateChunks();
        }
    }

    int32 ChunksSpawnedThisFrame = 0;

    // Spawn queued chunks, limited by MaxChunksSpawnedPerFrame
    while (SpawnQueue.Num() > 0 && ChunksSpawnedThisFrame < MaxChunksSpawnedPerFrame)
    {
        FIntPoint TargetCoord = SpawnQueue[0];
        SpawnQueue.RemoveAt(0);

        if (SpawnChunk(TargetCoord))
        {
            ChunksSpawnedThisFrame++;
        }
    }
}

// Updates which chunks should be present based on the viewer's position
void ATerrainChunkManager::UpdateChunks()
{
    if (!TerrainChunkClass) return;

    FVector ViewerLocation = Viewer ? Viewer->GetActorLocation() : GetActorLocation();
    FIntPoint ViewerCoord = GetChunkCoordinate(ViewerLocation);

    TSet<FIntPoint> ChunksToKeep;

    // Determine all chunk coordinates within draw distance
    for (int32 y = -ChunkDrawDistance; y <= ChunkDrawDistance; ++y)
    {
        for (int32 x = -ChunkDrawDistance; x <= ChunkDrawDistance; ++x)
        {
            FIntPoint TargetCoord = FIntPoint(ViewerCoord.X + x, ViewerCoord.Y + y);
            ChunksToKeep.Add(TargetCoord);

            // Queue chunk for spawning if not already present or queued
            if (!ActiveChunks.Contains(TargetCoord) && !SpawnQueue.Contains(TargetCoord))
            {
                SpawnQueue.Add(TargetCoord);
            }
        }
    }

    // Destroy and remove chunks that are now out of range
    TArray<FIntPoint> KeysToRemove;
    for (auto& Elem : ActiveChunks)
    {
        if (!ChunksToKeep.Contains(Elem.Key))
        {
            if (Elem.Value)
            {
                Elem.Value->Destroy();
            }
            KeysToRemove.Add(Elem.Key);
        }
    }

    for (const FIntPoint& Key : KeysToRemove)
    {
        ActiveChunks.Remove(Key);
    }

    // Remove queued chunks that are no longer needed
    for (int32 i = SpawnQueue.Num() - 1; i >= 0; i--)
    {
        if (!ChunksToKeep.Contains(SpawnQueue[i]))
        {
            SpawnQueue.RemoveAt(i);
        }
    }
}

// Converts a world location to a chunk grid coordinate
FIntPoint ATerrainChunkManager::GetChunkCoordinate(FVector Location) const
{
    int32 X = FMath::FloorToInt(Location.X / ChunkWorldSize);
    int32 Y = FMath::FloorToInt(Location.Y / ChunkWorldSize);
    return FIntPoint(X, Y);
}

// Generates a preview of the terrain grid in the editor
void ATerrainChunkManager::GeneratePreview()
{
    ClearChunks();
    Viewer = this; // Use manager's location as center for preview
    UpdateChunks();

    // Spawn all preview chunks immediately
    while (SpawnQueue.Num() > 0)
    {
        FIntPoint TargetCoord = SpawnQueue[0];
        SpawnQueue.RemoveAt(0);

        SpawnChunk(TargetCoord);
    }
}

// Destroys all active chunks and clears the spawn queue
void ATerrainChunkManager::ClearChunks()
{
    for (auto& Elem : ActiveChunks)
    {
        if (Elem.Value)
        {
            Elem.Value->Destroy();
        }
    }
    ActiveChunks.Empty();
    SpawnQueue.Empty();
}
