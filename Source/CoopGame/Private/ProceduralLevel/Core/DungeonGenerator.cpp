#include "ProceduralLevel/Core/DungeonGenerator.h"

#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Algo/RandomShuffle.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "ProceduralLevel/Core/DungeonDirector.h"
#include "DrawDebugHelpers.h"
#include "InteractionSystem/Core/Activator.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralLevel/Components/DungeonActivatorSpawner.h"


ADungeonGenerator::ADungeonGenerator()
{
    bReplicates = true;
}

void ADungeonGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    StartLocation = GetActorLocation();
    CurrentRetries = 0;

    if (HasAuthority())
    {
        StartGeneration();
    }
}

void ADungeonGenerator::StepGeneration()
{
    bool bShouldStop = PendingExits.Num() == 0;
    if (bExitSpawned && bStopGenerationOnExitSpawn) bShouldStop = true;
    
    const bool bForceExitPhase = (!bExitSpawned && CurrentRoomCount >= MaxRooms);
    if (CurrentRoomCount >= MaxRooms && !bForceExitPhase) bShouldStop = true;

    if (bShouldStop)
    {
        GetWorld()->GetTimerManager().ClearTimer(GenerationTimerHandle);

        bool bIsValidDungeon = true;

        if (!bExitSpawned)
        {
            bIsValidDungeon = false;
        }

        if (CurrentRoomCount < MinRoomsToValidate)
        {
             bIsValidDungeon = false;
        }

        
        if (!bIsValidDungeon)
        {
            if (CurrentRetries < MaxGenerationRetries)
            {
                CurrentRetries++;
                ResetDungeon();
                
                FTimerHandle RetryHandle;
                GetWorld()->GetTimerManager().SetTimer(RetryHandle, this, &ADungeonGenerator::StartGeneration, 0.1f, false);
                return;
            }
        }
        

        for (const FPendingExit& Exit : PendingExits)
        {
            SpawnEndWall(Exit.Transform);
        }
        PendingExits.Empty();

        if (ActivatorClass && ActivatorSpawnersLocations.Num() > 0)
        {
            int32 Counter = 0;
            
            while (Counter < MaxAmountOfActivators && ActivatorSpawnersLocations.Num() > 0)
            {
                if (const int32 RandomIndex = UKismetMathLibrary::RandomInteger(ActivatorSpawnersLocations.Num()); ActivatorSpawnersLocations.IsValidIndex(RandomIndex))
                {
                    AActor* Activator = GetWorld()->SpawnActor<AActor>(ActivatorClass, ActivatorSpawnersLocations[RandomIndex]);
                    ActivatorsGenerated.Add(Activator);
                    ActivatorSpawnersLocations.RemoveAt(RandomIndex);
                }
        
                Counter++;
            }
        }

        FBox DungeonBounds(ForceInit);
        for (const ADungeonRoomBase* Room : GeneratedRooms)
        {
            if (Room) DungeonBounds += Room->GetComponentsBoundingBox(true);
        }
        DungeonBounds = DungeonBounds.ExpandBy(500.0f);

        if (NavMeshBoundsVolume)
        {
            NavMeshBoundsVolume->SetActorLocation(DungeonBounds.GetCenter());
            const FVector NewScale = DungeonBounds.GetExtent() / 100.0f; 
            NavMeshBoundsVolume->SetActorScale3D(NewScale * 1.1f);
            
            if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
            {
                NavSys->OnNavigationBoundsUpdated(NavMeshBoundsVolume);
                NavSys->Build();
            }
        }

        if (DungeonDirector)
        {
            DungeonDirector->InitializeDungeon(GeneratedRooms);
        }
        
        Multicast_NotifyGenerationFinished();
        
        return;
    }

    const int32 RandomExitIndex = FMath::RandRange(0, PendingExits.Num() - 1);
    auto [Transform, ParentRoom] = PendingExits[RandomExitIndex];

    ERoomType ParentType = ERoomType::Room;

    if (IsValid(ParentRoom))
    {
        if (const ADungeonRoomBase* Room = Cast<ADungeonRoomBase>(ParentRoom))
        {
            ParentType = Room->RoomType;
        }
    }

    PendingExits.RemoveAtSwap(RandomExitIndex);

    const float DistToStart = FVector::Dist(Transform.GetLocation(), StartLocation);

    DrawDebugLine(GetWorld(), StartLocation, Transform.GetLocation(), FColor::Red, false, 2.0f, 0, 5.0f);

    const bool bExitConditionsMet = (!bExitSpawned && DistToStart >= MinExitDistance && CurrentRoomCount >= MinRoomsBeforeExit)
                              || bForceExitPhase;

    if (bExitConditionsMet)
    {
        if (TrySpawnExitRoom(Transform))
        {
            return;
        }
        DrawDebugBox(GetWorld(), Transform.GetLocation(), FVector(150), FColor::Red, false, 5.0f);
    }

    if (TrySpawnRoomAt(Transform, ParentRoom, ParentType))
    {
        return;
    }

    if (ParentType == ERoomType::Corridor)
    {
        if (TrySpawnCapRoom(Transform)) 
        {
            return;
        }
    }

    SpawnEndWall(Transform);
}

bool ADungeonGenerator::TrySpawnRoomAt(const FTransform& ExitTransform, AActor* ParentActor, const ERoomType ParentType)
{
    if (RoomClasses.Num() == 0) return false;

    const UClass* NeighborClass = ParentActor ? ParentActor->GetClass() : nullptr;

    TArray<TSubclassOf<ADungeonRoomBase>> Candidates = RoomClasses;

    Algo::RandomShuffle(Candidates);

    SortCandidatesByRhythm(Candidates, ParentType);

    if (Candidates.Num() > 1 && NeighborClass)
    {
        if (Candidates[0] == NeighborClass)
        {
            const auto Temp = Candidates[0];
            Candidates.RemoveAt(0);
            Candidates.Add(Temp);
        }
    }

    for (TSubclassOf SelectedClass : Candidates)
    {
        if (!SelectedClass) continue;

        const ADungeonRoomBase* CDO = SelectedClass->GetDefaultObject<ADungeonRoomBase>();
        if (!CDO) continue;

        const FTransform EntranceRelative = CDO->GetEntrance()->GetRelativeTransform();
        
        FTransform ConnectionTransform = ExitTransform;
        ConnectionTransform.SetRotation(ExitTransform.GetRotation() * FQuat(FRotator(0, 180, 0)));

        if (const FTransform SpawnTransform = EntranceRelative.Inverse() * ConnectionTransform; CanRoomFit(SelectedClass, SpawnTransform))
        {
            if (ADungeonRoomBase* NewRoom = GetWorld()->SpawnActor<ADungeonRoomBase>(SelectedClass, SpawnTransform))
            {
                GeneratedRooms.Add(NewRoom);
                CurrentRoomCount++;
                
                if (NewRoom->GetActivatorSpawner())
                {
                    FTransform NewTransform = NewRoom->GetActivatorSpawner()->GetComponentTransform();
                    FRotator NewRotation = NewRoom->GetActivatorSpawner()->GetRelativeRotation();

                    NewTransform.SetRotation(NewRotation.Quaternion());
                    ActivatorSpawnersLocations.Add(NewTransform);
                }
                
                if (ParentActor)
                {
                    if (ADungeonRoomBase* ParentRoomBase = Cast<ADungeonRoomBase>(ParentActor))
                    {
                        NewRoom->AddNeighbor(ParentRoomBase);
                        ParentRoomBase->AddNeighbor(NewRoom);
                    }
                }

                for (const UDungeonExitComponent* NewExit : NewRoom->GetExits())
                {
                    FPendingExit NextExit;
                    NextExit.Transform = NewExit->GetComponentTransform();
                    NextExit.ParentRoom = NewRoom;
                    PendingExits.Add(NextExit);
                }

                return true;
            }
        }
    }

    return false;
}

void ADungeonGenerator::SortCandidatesByRhythm(TArray<TSubclassOf<ADungeonRoomBase>>& Candidates, const ERoomType ParentType)
{
    ERoomType PreferredType = (ParentType == ERoomType::Corridor) ? ERoomType::Room : ERoomType::Corridor;

    Candidates.Sort([PreferredType](const TSubclassOf<ADungeonRoomBase>& A, const TSubclassOf<ADungeonRoomBase>& B)
    {
        if (!A || !B) return false;

        const ERoomType TypeA = A->GetDefaultObject<ADungeonRoomBase>()->RoomType;
        const ERoomType TypeB = B->GetDefaultObject<ADungeonRoomBase>()->RoomType;

        if (TypeA == PreferredType && TypeB != PreferredType) return true;
        
        if (TypeA != PreferredType && TypeB == PreferredType) return false;

        return false; 
    });
}

bool ADungeonGenerator::TrySpawnCapRoom(const FTransform& ExitTransform)
{
    if (CapRoomClasses.Num() == 0) return false;

    TArray<TSubclassOf<ADungeonRoomBase>> Caps = CapRoomClasses;
    Algo::RandomShuffle(Caps);

    for (TSubclassOf SelectedCap : Caps)
    {
        if (!SelectedCap) continue;
        const ADungeonRoomBase* CDO = SelectedCap->GetDefaultObject<ADungeonRoomBase>();
        
        FTransform EntranceRelative = CDO->GetEntrance()->GetRelativeTransform();
        FTransform ConnectionTransform = ExitTransform;
        ConnectionTransform.SetRotation(ExitTransform.GetRotation() * FQuat(FRotator(0, 180, 0)));
        
        FTransform SpawnTransform = EntranceRelative.Inverse() * ConnectionTransform;

        if (CanRoomFit(SelectedCap, SpawnTransform))
        {
            if (ADungeonRoomBase* NewRoom = GetWorld()->SpawnActor<ADungeonRoomBase>(SelectedCap, SpawnTransform))
            {
                GeneratedRooms.Add(NewRoom);
                return true;
            }
        }
    }
    return false;
}

bool ADungeonGenerator::CanRoomFit(const TSubclassOf<ADungeonRoomBase>& RoomClass, const FTransform& TargetTransform) const
{
    const ADungeonRoomBase* CDO = RoomClass->GetDefaultObject<ADungeonRoomBase>();
    if (!CDO || !CDO->GetRoomBounds()) return false;

    const FVector BoxExtent = CDO->GetRoomBounds()->GetScaledBoxExtent();
    const FVector BoxCenterLocal = CDO->GetRoomBounds()->GetRelativeLocation();

    const FVector BoxCenterWorld = TargetTransform.TransformPosition(BoxCenterLocal);
    const FQuat BoxRotationWorld = TargetTransform.GetRotation();

    const FVector CheckExtent = BoxExtent * 0.95f; 

    FCollisionQueryParams Params;
    Params.bTraceComplex = false;

    const bool bHit = GetWorld()->OverlapBlockingTestByChannel(
        BoxCenterWorld,
        BoxRotationWorld,
        ECC_WorldStatic,
        FCollisionShape::MakeBox(CheckExtent),
        Params
    );

    DrawDebugBox(GetWorld(), BoxCenterWorld, CheckExtent, BoxRotationWorld, 
        bHit ? FColor::Red : FColor::Green, false, 0.5f);

    return !bHit;
}

void ADungeonGenerator::SpawnEndWall(const FTransform& TargetTransform)
{
    if (EndWallClass)
    {
        FTransform WallTransform = TargetTransform;
        WallTransform.SetRotation(TargetTransform.GetRotation() * FQuat(FRotator(0, 180, 0)));

        const auto EndWall = GetWorld()->SpawnActor<ADungeonRoomBase>(EndWallClass, WallTransform);
        GeneratedRooms.Add(EndWall);
    }
}

bool ADungeonGenerator::TrySpawnExitRoom(const FTransform& ExitTransform)
{
    if (!ExitRoomClass || bExitSpawned) return false;

    const ADungeonRoomBase* CDO = ExitRoomClass->GetDefaultObject<ADungeonRoomBase>();
    if (!CDO) return false;

    const FTransform EntranceRelative = CDO->GetEntrance()->GetRelativeTransform();
    FTransform ConnectionTransform = ExitTransform;
    ConnectionTransform.SetRotation(ExitTransform.GetRotation() * FQuat(FRotator(0, 180, 0)));
    const FTransform SpawnTransform = EntranceRelative.Inverse() * ConnectionTransform;

    if (CanRoomFit(ExitRoomClass, SpawnTransform))
    {
        if (ADungeonRoomBase* NewRoom = GetWorld()->SpawnActor<ADungeonRoomBase>(ExitRoomClass, SpawnTransform))
        {
            GeneratedRooms.Add(NewRoom);
            CurrentRoomCount++;
            bExitSpawned = true;

            if (!bStopGenerationOnExitSpawn)
            {
                for (const UDungeonExitComponent* NewExit : NewRoom->GetExits())
                {
                    FPendingExit NextExit;
                    NextExit.Transform = NewExit->GetComponentTransform();
                    NextExit.ParentRoom = NewRoom;
                    PendingExits.Add(NextExit);
                }
            }
            
            return true;
        }
    }
    return false;
}

void ADungeonGenerator::ResetDungeon()
{
    GetWorld()->GetTimerManager().ClearTimer(GenerationTimerHandle);

    for (ADungeonRoomBase* Room : GeneratedRooms)
    {
        if (Room && !Room->IsActorBeingDestroyed())
        {
            Room->Destroy();
        }
    }

    for (AActor* Activator : ActivatorsGenerated)
    {
        if (Activator && !Activator->IsActorBeingDestroyed())
        {
            Activator->Destroy();
        }
    }

    ActivatorSpawnersLocations.Empty();
    GeneratedRooms.Empty();
    PendingExits.Empty();
}

void ADungeonGenerator::StartGeneration()
{
    CurrentRoomCount = 0;
    bExitSpawned = false;
    PendingExits.Empty();

    if (GeneratedRooms.Num() > 0) ResetDungeon();

    if (StartRoomClass)
    {
        if (ADungeonRoomBase* StartRoom = GetWorld()->SpawnActor<ADungeonRoomBase>(StartRoomClass, GetActorTransform()))
        {
            GeneratedRooms.Add(StartRoom);
            CurrentRoomCount++;
            
            for (const UDungeonExitComponent* Exit : StartRoom->GetExits())
            {
                FPendingExit NewExit;
                NewExit.Transform = Exit->GetComponentTransform();
                NewExit.ParentRoom = StartRoom;
                PendingExits.Add(NewExit);
            }

            GetWorld()->GetTimerManager().SetTimer(GenerationTimerHandle, this, &ADungeonGenerator::StepGeneration, GenerationStepDelay, true);
        }
    }
}

void ADungeonGenerator::Multicast_NotifyGenerationFinished_Implementation()
{
    if (OnGenerationFinished.IsBound())
    {
        OnGenerationFinished.Broadcast();
    }
}