// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "GenericTeamAgentInterface.h"
#include "AgentController.generated.h"

UCLASS()
class COOPGAME_API AAgentController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAgentController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team)
	FGenericTeamId TeamId;

	UFUNCTION(BlueprintCallable, Category = Team)
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable, Category = Team)
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};
