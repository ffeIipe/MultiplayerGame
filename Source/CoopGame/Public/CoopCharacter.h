// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CoopCharacter.generated.h"

class UCameraComponent;
class UCoopHealthComponent;
class USpringArmComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACoopCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCoopHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	
public:
	ACoopCharacter();

protected:
	void Interact();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	void Attack();
	
	UFUNCTION(Server, Reliable)
	void Server_Attack();

public:
	FORCEINLINE UCoopHealthComponent* GetHealthComponent() const { return HealthComp; }

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() const { return SpringArm; }

	FORCEINLINE UCameraComponent* GetCameraComponent() const { return Camera; }
};

