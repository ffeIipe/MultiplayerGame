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
	OnRep_Health();

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

	ResetStats();
	
	OnRep_IsDowned();
}

void UCoopHealthComponent::ResetStats()
{
	if (!GetOwner()->HasAuthority()) 
	{
		return;
	}

	CurrentHealth = MaxHealth;
	bIsDowned = false;

	OnRep_Health();
	OnRep_IsDowned();
}
void UCoopHealthComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(CurrentHealth);
}

void UCoopHealthComponent::OnRep_IsDowned()
{
	OnDownedStateChanged.Broadcast(bIsDowned);
}