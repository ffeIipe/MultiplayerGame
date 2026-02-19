#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionSystem/Interfaces/Interactable.h"
#include "Activator.generated.h"

UCLASS()
class COOPGAME_API AActivator : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AActivator();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Properties")
	TArray<AActor*> LinkedTargets;

	UPROPERTY(EditAnywhere, SaveGame, Category = "Properties")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, Category = "Properties|Debug")
	bool bDebugConnections = true;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
	
public:
	virtual bool ExecuteInteraction_Implementation(AActor* Interactor) override;
	
private:
	void SetLeverState(bool bNewState);
};