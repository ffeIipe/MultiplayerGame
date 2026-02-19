// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopCharacters/AI/AgentController.h"


// Sets default values
AAgentController::AAgentController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAgentController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAgentController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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