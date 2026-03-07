#include "GenericPool/Core/ActorPoolSubsystem.h"


void UActorPoolSubsystem::ReturnToPool(AActor* ActorToReturn)
{
	if (!IsValid(ActorToReturn)) return;

	if (ActorToReturn->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnPoolDeactivate(ActorToReturn);
	}

	InactivePools.FindOrAdd(ActorToReturn->GetClass()).Actors.Push(ActorToReturn);
}

AActor* UActorPoolSubsystem::GetActorFromPool(const TSubclassOf<AActor>& ClassToSpawn, const FTransform& SpawnTransform)
{
	if (!ClassToSpawn) return nullptr;

	AActor* ResultActor = nullptr;
	
	if (FPooledActorArray* PoolStruct = InactivePools.Find(ClassToSpawn))
	{
		while (PoolStruct->Actors.Num() > 0)
		{
			if (AActor* Candidate = PoolStruct->Actors.Pop(); IsValid(Candidate))
			{
				ResultActor = Candidate;
				break;
			}
		}
	}

	if (!ResultActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ResultActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTransform, SpawnParams);
	}

	if (ResultActor)
	{
		ResultActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		
		ResultActor->SetActorHiddenInGame(false);
		ResultActor->SetActorEnableCollision(true);

		if (ResultActor->Implements<UPoolableActor>())
		{
			IPoolableActor::Execute_OnPoolActivate(ResultActor);
		}
	}

	return ResultActor;
}

void UActorPoolSubsystem::InitializePool(const TSubclassOf<AActor> ClassToSpawn, const int32 Quantity)
{
	if (!ClassToSpawn || Quantity <= 0) return;

	FPooledActorArray& PoolStruct = InactivePools.FindOrAdd(ClassToSpawn);
	PoolStruct.Actors.Reserve(PoolStruct.Actors.Num() + Quantity);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 0; i < Quantity; ++i)
	{
		if (AActor* NewActor = World->SpawnActor<AActor>(ClassToSpawn, FTransform::Identity, SpawnParams); IsValid(NewActor))
		{
			ReturnToPool(NewActor);
		}
	}
}

AActor* UActorPoolSubsystem::RetrieveActor(const UClass* ClassType)
{
	if (FPooledActorArray* PoolStruct = InactivePools.Find(ClassType))
	{
		while (PoolStruct->Actors.Num() > 0)
		{
			if (AActor* Candidate = PoolStruct->Actors.Pop(); IsValid(Candidate))
			{
				return Candidate;
			}
		}
	}
    
	return nullptr;
}