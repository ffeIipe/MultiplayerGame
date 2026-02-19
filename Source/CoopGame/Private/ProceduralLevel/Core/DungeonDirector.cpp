#include "ProceduralLevel/Core/DungeonDirector.h"
#include "GameFramework/Character.h"

ADungeonDirector::ADungeonDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADungeonDirector::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ADungeonDirector::ManageEnemySpawning, SpawnCheckInterval, true);
}

void ADungeonDirector::InitializeDungeon(TArray<ADungeonRoomBase*> AllRooms)
{
    GeneratedRooms = AllRooms;

    for (ADungeonRoomBase* Room : GeneratedRooms)
    {
        if (Room)
        {
            Room->OnPlayerEnter.AddDynamic(this, &ADungeonDirector::OnRoomEntered);
            
            if (bEnableRoomCulling)
            {
                Room->SetRoomActive(false);
            }
            else
            {
                Room->SetRoomActive(true);
            }
        }
    }
    
    // Activar el primer cuarto manualmente si existe y hay culling
    if (GeneratedRooms.Num() > 0 && bEnableRoomCulling)
    {
        // Forzamos la entrada al primer cuarto para encenderlo a él y sus vecinos
        OnRoomEntered(GeneratedRooms[0]);
    }
}

void ADungeonDirector::OnRoomEntered(ADungeonRoomBase* NewRoom)
{
    if (CurrentPlayerRoom == NewRoom) return;

    CurrentPlayerRoom = NewRoom;
    
    // Solo actualizamos visibilidad si el sistema está activado
    if (bEnableRoomCulling)
    {
        UpdateVisibility();
    }
    
    ManageEnemySpawning();
}

void ADungeonDirector::UpdateVisibility()
{
    if (!CurrentPlayerRoom) return;

    // 1. Apagar TODOS los cuartos primero (Reset)
    for (ADungeonRoomBase* Room : GeneratedRooms)
    {
        if (Room) Room->SetRoomActive(false);
    }

    // 2. Encender recursivamente usando la variable 'VisibilityDepth'
    SetRoomVisibilityRecursive(CurrentPlayerRoom, VisibilityDepth);
}

void ADungeonDirector::SetRoomVisibilityRecursive(ADungeonRoomBase* Room, int32 Depth)
{
    if (!Room || Depth < 0) return;

    // Encender este cuarto
    Room->SetRoomActive(true);

    // Si aun nos queda profundidad, encender vecinos
    if (Depth > 0)
    {
        for (ADungeonRoomBase* Neighbor : Room->GetConnectedNeighbors())
        {
            SetRoomVisibilityRecursive(Neighbor, Depth - 1);
        }
    }
}

void ADungeonDirector::CleanupDeadZombies()
{
    for (int32 i = ActiveZombies.Num() - 1; i >= 0; --i)
    {
        if (!ActiveZombies[i] || ActiveZombies[i]->IsActorBeingDestroyed())
        {
            ActiveZombies.RemoveAt(i);
        }
    }
}

void ADungeonDirector::ManageEnemySpawning()
{
    if (!CurrentPlayerRoom || !ZombieClass) return;

    CleanupDeadZombies();

    const int32 CurrentCount = ActiveZombies.Num();
    int32 SlotsAvailable = MaxActiveZombies - CurrentCount;

    if (SlotsAvailable <= 0) return;

    for (TArray<ADungeonRoomBase*> TargetRooms = CurrentPlayerRoom->GetConnectedNeighbors(); ADungeonRoomBase* Room : TargetRooms)
    {
        if (SlotsAvailable <= 0) break;
        if (!Room) continue;

        TArray<FTransform> SpawnPoints = Room->GetSpawnPoints();
        
        for (const FTransform& SP : SpawnPoints)
        {
            if (SlotsAvailable <= 0) break;

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

            if (ACharacter* NewZombie = GetWorld()->SpawnActor<ACharacter>(ZombieClass, SP, SpawnParams))
            {
                ActiveZombies.Add(NewZombie);
                SlotsAvailable--;
            }
        }
    }
}