#include "Components/CoopHealthComponent.h"
#include "Net/UnrealNetwork.h"

UCoopHealthComponent::UCoopHealthComponent()
{
	SetIsReplicatedByDefault(true);
	bIsDowned = false;
}

void UCoopHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

void UCoopHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCoopHealthComponent, CurrentHealth);
	DOREPLIFETIME(UCoopHealthComponent, bIsDowned);
}

void UCoopHealthComponent::TakeDamage(float DamageAmount)
{
	if (!GetOwner()->HasAuthority() || bIsDowned) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		bIsDowned = true;
		OnRep_IsDowned();
	}
}

void UCoopHealthComponent::Revive()
{
	if (!GetOwner()->HasAuthority()) return;

	bIsDowned = false;
	CurrentHealth = MaxHealth * 0.5f;
	OnRep_IsDowned();
}

void UCoopHealthComponent::OnRep_Health()
{
	// Aquí podrías actualizar una barra de vida si quisieras
}

void UCoopHealthComponent::OnRep_IsDowned()
{
	OnDownedStateChanged.Broadcast(bIsDowned);
	
	// Debug visual rápido
	/* if (bIsDowned)
		UE_LOG(LogTemp, Warning, TEXT("ESTOY HERIDO! AYUDA!"));
	else
		UE_LOG(LogTemp, Warning, TEXT("ESTOY VIVO OTRA VEZ!"));
	*/
}