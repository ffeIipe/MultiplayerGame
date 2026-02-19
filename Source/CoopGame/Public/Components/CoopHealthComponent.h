// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoopHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDownedStateChanged, bool, bNewIsDowned);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOPGAME_API UCoopHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	UCoopHealthComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(ReplicatedUsing = OnRep_IsDowned, BlueprintReadOnly, Category = "Health")
	bool bIsDowned;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDowned();

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Revive();

	UFUNCTION(BlueprintPure)
	bool IsDowned() const { return bIsDowned; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDownedStateChanged OnDownedStateChanged;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
