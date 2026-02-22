#include "ProceduralLevel/Core/DungeonDirector.h"

#include "Algo/RandomShuffle.h"
#include "CoopCharacters/AI/AgentController.h"
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
    
    if (GeneratedRooms.Num() > 0 && bEnableRoomCulling)
    {
        OnRoomEntered(GeneratedRooms[0]);
    }
}

void ADungeonDirector::OnRoomEntered(ADungeonRoomBase* NewRoom)
{
    if (CurrentPlayerRoom == NewRoom) return;

    CurrentPlayerRoom = NewRoom;
    
    if (bEnableRoomCulling)
    {
        UpdateVisibility();
    }
    
    ManageEnemySpawning();
}

void ADungeonDirector::UpdateVisibility()
{
    if (!CurrentPlayerRoom) return;

    for (ADungeonRoomBase* Room : GeneratedRooms)
    {
        if (Room) Room->SetRoomActive(false);
    }

    SetRoomVisibilityRecursive(CurrentPlayerRoom, VisibilityDepth);
}

void ADungeonDirector::SetRoomVisibilityRecursive(ADungeonRoomBase* Room, int32 Depth)
{
    if (!Room || Depth < 0) return;

    Room->SetRoomActive(true);

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
    if (!CurrentPlayerRoom) return;

    FVector PlayerGeneralLocation = CurrentPlayerRoom->GetActorLocation();

    constexpr float MaxZombieDistance = 4000.0f; 

    for (int32 i = ActiveZombies.Num() - 1; i >= 0; --i)
    {
        ACharacter* Zombie = ActiveZombies[i];

        if (!Zombie || Zombie->IsActorBeingDestroyed()/* || Zombie->IsDead()*/)
        {
            ActiveZombies.RemoveAt(i);
            continue;
        }

        float DistToPlayer = FVector::Dist(Zombie->GetActorLocation(), PlayerGeneralLocation);
        if (DistToPlayer > MaxZombieDistance)
        {
            Zombie->Destroy();
            ActiveZombies.RemoveAt(i);
        }
    }
}

void ADungeonDirector::ManageEnemySpawning()
{
    if (!CurrentPlayerRoom || ZombiesClasses.Num() <= 0) return;

    CleanupDeadZombies();

    const int32 CurrentCount = ActiveZombies.Num();
    int32 SlotsAvailable = MaxActiveZombies - CurrentCount;

    if (SlotsAvailable <= 0) return;

    TArray<FTransform> AvailableSpawns;
    TArray<ADungeonRoomBase*> TargetRooms = CurrentPlayerRoom->GetConnectedNeighbors();
    
    // Opcional: También podrías agregar los spawns del CurrentPlayerRoom aquí si quieres
    
    for (const ADungeonRoomBase* Room : TargetRooms)
    {
        if (Room)
        {
            AvailableSpawns.Append(Room->GetSpawnPoints());
        }
    }

    if (AvailableSpawns.Num() == 0) return;

    Algo::RandomShuffle(AvailableSpawns);

    int32 SpawnsToPerform = FMath::Min(SlotsAvailable, AvailableSpawns.Num());

    for (int32 i = 0; i < SpawnsToPerform; i++)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

        int32 RandomIndex = FMath::RandRange(0, ZombiesClasses.Num() - 1);
        auto RandomClass = ZombiesClasses[RandomIndex];

        if (ACharacter* NewZombie = GetWorld()->SpawnActor<ACharacter>(RandomClass, AvailableSpawns[i], SpawnParams))
        {
            ActiveZombies.Add(NewZombie);

            const APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC && PC->GetPawn())
            {
                if (AAgentController* ZombieAI = Cast<AAgentController>(NewZombie->GetController()))
                {
                    ZombieAI->SetInitialTarget(PC->GetPawn());
                }
            }
        }
    }
}