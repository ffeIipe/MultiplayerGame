#include "ProceduralLevel/Core/DungeonDirector.h"

#include "Algo/RandomShuffle.h"
#include "Components/CoopHealthComponent.h"
#include "CoopCharacters/AI/AgentController.h"
#include "GameFramework/Character.h"
#include "GenericPool/Core/ActorPoolSubsystem.h"
#include "Engine/GameInstance.h"

ADungeonDirector::ADungeonDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADungeonDirector::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ADungeonDirector::ManageEnemySpawning, SpawnCheckInterval, true);
    }
}

void ADungeonDirector::InitializeDungeon(TArray<ADungeonRoomBase*> AllRooms)
{
    if (!HasAuthority()) return;
    
    GeneratedRooms = AllRooms;

    if (const UWorld* World = GetWorld())
    {
        if (UActorPoolSubsystem* PoolSubsystem = World->GetSubsystem<UActorPoolSubsystem>())
        {
            if (ZombiesClasses.Num() > 0)
            {
                for (TSubclassOf ZombieClass : ZombiesClasses)
                {
                    if (ZombieClass)
                    {
                        PoolSubsystem->InitializePool(ZombieClass, ZombiesInitialAmount);
                    }
                }
            }
        }
    }

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
    if (!HasAuthority()) return;
    
    if (!IsValid(CurrentPlayerRoom)) return;

    FVector PlayerGeneralLocation = CurrentPlayerRoom->GetActorLocation();

    UActorPoolSubsystem* PoolSubsystem = nullptr;
    if (const UWorld* World = GetWorld())
    {
        PoolSubsystem = World->GetSubsystem<UActorPoolSubsystem>();
    }

    for (int32 i = ActiveZombies.Num() - 1; i >= 0; --i)
    {
        ACharacter* Zombie = ActiveZombies[i];

        if (!IsValid(Zombie))
        {
            ActiveZombies.RemoveAt(i);
            continue;
        }

        bool bIsAlive = false;

        if (const UCoopHealthComponent* HealthComponent = Zombie->GetComponentByClass<UCoopHealthComponent>())
        {
            bIsAlive = HealthComponent->IsAlive();
        }

        if (!bIsAlive)
        {
            if (PoolSubsystem) PoolSubsystem->ReturnToPool(Zombie);
            if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Emerald, ActiveZombies[i]->GetName() + "Is not alive...");

            ActiveZombies.RemoveAt(i);
            
            continue;
        }

        float DistToPlayer = FVector::Dist(Zombie->GetActorLocation(), PlayerGeneralLocation);
        if (DistToPlayer > MaxZombieDistance)
        {
            if (PoolSubsystem) PoolSubsystem->ReturnToPool(Zombie);
            ActiveZombies.RemoveAt(i);
        }
    }
}

void ADungeonDirector::ManageEnemySpawning()
{
    if (!HasAuthority()) return;
    
    if (!IsValid(CurrentPlayerRoom) || ZombiesClasses.Num() <= 0) return;

    CleanupDeadZombies();

    const int32 CurrentCount = ActiveZombies.Num();
    int32 SlotsAvailable = MaxActiveZombies - CurrentCount;

    if (SlotsAvailable <= 0) return;

    TArray<FTransform> AvailableSpawns;
    TArray<ADungeonRoomBase*> TargetRooms = CurrentPlayerRoom->GetConnectedNeighbors();
    
    for (const ADungeonRoomBase* Room : TargetRooms)
    {
        if (IsValid(Room)) 
        {
            AvailableSpawns.Append(Room->GetSpawnPoints());
        }
    }

    if (AvailableSpawns.Num() == 0) return;

    Algo::RandomShuffle(AvailableSpawns);

    int32 SpawnsToPerform = FMath::Min(SlotsAvailable, AvailableSpawns.Num());

    UActorPoolSubsystem* PoolSubsystem = nullptr;
    
    if (const UWorld* World = GetWorld())
    {
        PoolSubsystem = World->GetSubsystem<UActorPoolSubsystem>();
    }

    for (int32 i = 0; i < SpawnsToPerform; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, ZombiesClasses.Num() - 1);
        auto RandomClass = ZombiesClasses[RandomIndex];

        if (!RandomClass) continue;

        ACharacter* NewZombie = nullptr;

        if (PoolSubsystem)
        {
            NewZombie = Cast<ACharacter>(PoolSubsystem->GetActorFromPool(RandomClass, AvailableSpawns[i]));
        }

        ActiveZombies.Add(NewZombie);
    }
}