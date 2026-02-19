#include "InteractionSystem/Core/Activator.h"
#include "InteractionSystem/Interfaces/SignalReceiver.h"
#include "Kismet/KismetSystemLibrary.h"

AActivator::AActivator()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void AActivator::BeginPlay()
{
	Super::BeginPlay();

	SetLeverState(bIsActive);
}

void AActivator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugConnections)
	{
		const FColor LineColor = bIsActive ? FColor::Green : FColor::Red;

		for (const AActor* Target : LinkedTargets)
		{
			if (Target)
			{
				UKismetSystemLibrary::DrawDebugArrow(GetWorld(), GetActorLocation(), Target->GetActorLocation(), 
					30.0f, LineColor, -1.0f, 1.5f);
			}
		}
	}
}

#if WITH_EDITOR
bool AActivator::ShouldTickIfViewportsOnly() const
{
	return bDebugConnections;
}
#endif

bool AActivator::ExecuteInteraction_Implementation(AActor* Interactor)
{
	SetLeverState(!bIsActive);
	return bIsActive;
}

void AActivator::SetLeverState(const bool bNewState)
{
	bIsActive = bNewState;

	for (AActor* Target : LinkedTargets)
	{
		if (!Target) continue;
		
		if (Target->GetClass()->ImplementsInterface(USignalReceiver::StaticClass()))
		{
			ISignalReceiver::Execute_ReceiveSignal(Target, bIsActive, this);
		}
	}
}