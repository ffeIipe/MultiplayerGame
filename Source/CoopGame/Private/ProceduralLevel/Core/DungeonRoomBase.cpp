#include "ProceduralLevel/Core/DungeonRoomBase.h"

#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralLevel/Components/DungeonEnemySpawner.h"
#include "ProceduralLevel/Components/DungeonActivatorSpawner.h"


ADungeonRoomBase::ADungeonRoomBase()
{
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->Mobility = EComponentMobility::Static;

	RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
	RoomBounds->SetupAttachment(RootComponent);
	RoomBounds->SetCollisionProfileName(TEXT("NoCollision"));

	EntranceMarker = CreateDefaultSubobject<USceneComponent>(TEXT("EntranceMarker"));
	EntranceMarker->SetupAttachment(RootComponent);

	RoomTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomTrigger"));
	RoomTrigger->SetupAttachment(RootComponent);
	RoomTrigger->SetBoxExtent(FVector(400, 400, 100)); 
	RoomTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void ADungeonRoomBase::BeginPlay()
{
	Super::BeginPlay();
	RoomTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADungeonRoomBase::OnTriggerOverlap);

	if (!RoomLevel.IsNull())
	{
		bool bSuccess = false;
		
		StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			GetWorld(),
			RoomLevel,
			GetActorLocation(),
			GetActorRotation(),
			bSuccess
		);
	}
	
	SetRoomActive(true);
}

UDungeonActivatorSpawner* ADungeonRoomBase::GetActivatorSpawner() const
{
	TArray<FTransform> Points;
	TArray<UDungeonActivatorSpawner*> PossibleActivators;
	GetComponents<UDungeonActivatorSpawner>(PossibleActivators);

	if (PossibleActivators.Num() > 0)
	{
		const int32 RandIndex = UKismetMathLibrary::RandomInteger(PossibleActivators.Num());
		return PossibleActivators[RandIndex];
	}

	return nullptr;
}

void ADungeonRoomBase::AddNeighbor(ADungeonRoomBase* Neighbor)
{
	if (Neighbor && !ConnectedNeighbors.Contains(Neighbor))
	{
		ConnectedNeighbors.Add(Neighbor);
	}
}

TArray<FTransform> ADungeonRoomBase::GetSpawnPoints() const
{
	TArray<FTransform> Points;
	TArray<UDungeonEnemySpawner*> Spawners;
	GetComponents<UDungeonEnemySpawner>(Spawners);

	for (const UDungeonEnemySpawner* Spawner : Spawners)
	{
		FTransform SpawnTransform = Spawner->GetComponentTransform();
        
		SpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
        
		Points.Add(SpawnTransform);
	}
	return Points;
}

void ADungeonRoomBase::SetRoomActive(const bool bActive)
{
	SetActorHiddenInGame(!bActive);

	if (StreamingLevel)
	{
		StreamingLevel->SetShouldBeVisible(bActive);
	}
}

void ADungeonRoomBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (const ACharacter* OverlappedCharacter = Cast<ACharacter>(OtherActor))
	{
		if (OverlappedCharacter->IsPlayerControlled())
		{
			if (OnPlayerEnter.IsBound())
			{
				OnPlayerEnter.Broadcast(this);
			}
		}
	}
}

TArray<UDungeonExitComponent*> ADungeonRoomBase::GetExits() const
{
	TArray<UDungeonExitComponent*> Exits;
	GetComponents<UDungeonExitComponent>(Exits);
	return Exits;
}
