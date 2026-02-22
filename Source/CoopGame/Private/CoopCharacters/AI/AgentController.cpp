// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopCharacters/AI/AgentController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


AAgentController::AAgentController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAgentController::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TrackingTimerHandle, this, &AAgentController::UpdateTargetAndMove, TargetUpdateInterval, true);
}

void AAgentController::SetInitialTarget(AActor* TargetActor)
{
    CurrentTarget = TargetActor;
    if (CurrentTarget)
    {
    	if (GetBlackboardComponent())
    	{
    		GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), CurrentTarget);
    	}
    }
}

void AAgentController::UpdateTargetAndMove()
{
    const APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    TArray<AActor*> FoundPlayers;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundPlayers);

    AActor* ClosestPlayer = nullptr;
    float MinDistance = UE_BIG_NUMBER;
    const FVector ZombieLocation = ControlledPawn->GetActorLocation();

    for (AActor* PlayerActor : FoundPlayers)
    {
        const APawn* PlayerPawn = Cast<APawn>(PlayerActor);
        if (PlayerPawn && PlayerPawn->GetController() && PlayerPawn->GetController()->IsPlayerController())
        {
            const float DistToPlayer = FVector::Dist(ZombieLocation, PlayerActor->GetActorLocation());
            
            if (DistToPlayer < MinDistance)
            {
                MinDistance = DistToPlayer;
                ClosestPlayer = PlayerActor;
            }
        }
    }

    if (ClosestPlayer)
    {
        CurrentTarget = ClosestPlayer;
        
    	if (GetBlackboardComponent())
    	{
    		GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), ClosestPlayer);
    	}
    }
}

FGenericTeamId AAgentController::GetGenericTeamId() const
{
	return TeamId.GetId();
}

void AAgentController::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);
}

ETeamAttitude::Type AAgentController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<APawn>(&Other);

	if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(PawnToCheck->GetController()))
	{
		const FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

		if (OtherTeamId == FGenericTeamId(0)) return ETeamAttitude::Neutral;
		if (OtherTeamId == FGenericTeamId(1)) return ETeamAttitude::Friendly;
		if (OtherTeamId == FGenericTeamId(2)) return ETeamAttitude::Hostile;
	}
	return ETeamAttitude::Neutral;
}