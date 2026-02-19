#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionSystem/Interfaces/SignalReceiver.h"
#include "LogicGate.generated.h"

UCLASS()
class COOPGAME_API ALogicGate : public AActor, public ISignalReceiver
{
	GENERATED_BODY()
	
public:	
	ALogicGate();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugConnections = true;

	void ResetSources();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 RequiredActivations = 2;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Properties")
	TArray<AActor*> LinkedTargets;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bIsByTimerDeactivation = true;

	UPROPERTY(EditAnywhere, Category = "Properties", meta = (EditCondition = "bIsByTimerDeactivation", EditConditionHides))
	float DeactivationTime = 3.0f;

	FTimerHandle DeactivationTimerHandle;
	
	UPROPERTY(SaveGame)
	TSet<AActor*> ActiveSources;

	UPROPERTY(SaveGame)
	bool bOutputState = false;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif

public:
	virtual void ReceiveSignal_Implementation(bool bActive, AActor* Activator) override;

private:
	void UpdateLogicState();
	void BroadcastToTargets(bool bActive);
};