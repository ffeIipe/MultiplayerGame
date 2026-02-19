#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DungeonEnemySpawner.generated.h"

UCLASS(ClassGroup=(Dungeon), meta=(BlueprintSpawnableComponent))
class COOPGAME_API UDungeonEnemySpawner : public USceneComponent
{
	GENERATED_BODY()

public:
	UDungeonEnemySpawner();

#if WITH_EDITOR
	virtual void OnRegister() override;
#endif
};