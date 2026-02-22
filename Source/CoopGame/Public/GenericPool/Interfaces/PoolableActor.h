#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

UINTERFACE()
class UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

class COOPGAME_API IPoolableActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPoolActivate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPoolDeactivate();
};
