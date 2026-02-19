#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DungeonActivatorSpawner.generated.h"

UCLASS(ClassGroup=(Dungeon), meta=(BlueprintSpawnableComponent))
class COOPGAME_API UDungeonActivatorSpawner : public USceneComponent
{
	GENERATED_BODY()

public:
	UDungeonActivatorSpawner();

#if WITH_EDITOR
	virtual void OnRegister() override;
#endif
};