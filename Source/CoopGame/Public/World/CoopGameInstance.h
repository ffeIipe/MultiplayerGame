#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CoopGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UCoopGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadSettings();
};
