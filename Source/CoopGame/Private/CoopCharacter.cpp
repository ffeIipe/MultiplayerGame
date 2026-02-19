// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoopCharacter.h"

#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CoopHealthComponent.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACoopGameCharacter

ACoopCharacter::ACoopCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	HealthComp = CreateDefaultSubobject<UCoopHealthComponent>(TEXT("HealthComp"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}


void ACoopCharacter::Server_Interact_Implementation()
{
}

void ACoopCharacter::Server_Attack_Implementation()
{
	if (HealthComp)
	{
		HealthComp->TakeDamage(20.0f);
        
		UE_LOG(LogTemp, Warning, TEXT("El Servidor aplicó daño a %s"), *GetName());
	}
}

void ACoopCharacter::Interact()
{
	if (HealthComp && HealthComp->IsDowned()) return;

	Server_Interact();
}

void ACoopCharacter::Attack()
{
	Server_Attack();
}
