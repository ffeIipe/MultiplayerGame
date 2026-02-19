#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonRoomBase.h"
#include "DungeonDirector.generated.h"


UCLASS()
class COOPGAME_API ADungeonDirector : public AActor
{
	GENERATED_BODY()
    
public:    
	ADungeonDirector();

	void InitializeDungeon(TArray<ADungeonRoomBase*> AllRooms);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	bool bEnableRoomCulling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (EditCondition = "bEnableRoomCulling"))
	int32 VisibilityDepth = 3;

	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<ACharacter> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "AI")
	int32 MaxActiveZombies = 10;

	UPROPERTY(EditAnywhere, Category = "AI")
	float SpawnCheckInterval = 2.0f;
	
	UPROPERTY()
	TArray<ADungeonRoomBase*> GeneratedRooms;

	UPROPERTY()
	ADungeonRoomBase* CurrentPlayerRoom;

	UPROPERTY()
	TArray<ACharacter*> ActiveZombies;

	FTimerHandle SpawnTimerHandle;

	UFUNCTION()
	void OnRoomEntered(ADungeonRoomBase* NewRoom);

	void UpdateVisibility();
	void SetRoomVisibilityRecursive(ADungeonRoomBase* Room, int32 Depth);

	void ManageEnemySpawning();
	void CleanupDeadZombies();
};