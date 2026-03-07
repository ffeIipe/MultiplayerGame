#include "GenericPool/Core/PoolManager.h"

APoolManager::APoolManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APoolManager::InitializePool(const TSubclassOf<AActor> ActorClass, const int32 PoolSize)
{
    if (!ActorClass)
    {
        return;
    }

    if (Pool.Num() > 0)
    {
        Pool.Empty();
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < PoolSize; i++)
    {
        if (AActor* NewActor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
        {
            NewActor->SetActorHiddenInGame(true);
            NewActor->SetActorEnableCollision(false);
            NewActor->SetActorTickEnabled(false);
            
            Pool.Add(NewActor);
        }
    }
}

AActor* APoolManager::GetActorFromPool(FTransform SpawnTransform)
{
    for (AActor* Actor : Pool)
    {
        if (Actor && Actor->IsHidden())
        {
            Actor->SetActorTransform(SpawnTransform);
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
            
            
            return Actor;
        }
    }

    return nullptr;
}

void APoolManager::ReturnActorToPool(AActor* ActorToReturn)
{
    if (ActorToReturn)
    {
        ActorToReturn->SetActorHiddenInGame(true);
        ActorToReturn->SetActorEnableCollision(false);
        ActorToReturn->SetActorTickEnabled(false);
    }
}