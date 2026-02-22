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
	AAgentController();

	void SetInitialTarget(AActor* TargetActor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Tracking")
	float TargetUpdateInterval = 1.0f; 

	UPROPERTY(BlueprintReadOnly, Category = "AI|Tracking")
	AActor* CurrentTarget;

	FTimerHandle TrackingTimerHandle;

	UFUNCTION()
	void UpdateTargetAndMove();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team)
	FGenericTeamId TeamId;

	UFUNCTION(BlueprintCallable, Category = Team)
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable, Category = Team)
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};
