// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralLevel/Components/DungeonExitComponent.h"
#include "DungeonRoomBase.generated.h"

class ULevelStreamingDynamic;
class UDungeonActivatorSpawner;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnterRoom, ADungeonRoomBase*, Room);

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Room        UMETA(DisplayName = "Room"),
	Corridor    UMETA(DisplayName = "Corridor"),
	SmallCap    UMETA(DisplayName = "Small Cap/Reward")
};

UCLASS()
class COOPGAME_API ADungeonRoomBase : public AActor
{
	GENERATED_BODY()

public:    
	ADungeonRoomBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon Logic")
	UBoxComponent* RoomTrigger;

	UPROPERTY(VisibleInstanceOnly, Category = "Dungeon Logic")
	TArray<ADungeonRoomBase*> ConnectedNeighbors;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon Validation")
	UBoxComponent* RoomBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon Validation")
	USceneComponent* EntranceMarker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon Visuals")
	TSoftObjectPtr<UWorld> RoomLevel;

	UPROPERTY(Transient)
	ULevelStreamingDynamic* StreamingLevel;

public:
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	TArray<UDungeonExitComponent*> GetExits() const;

	UBoxComponent* GetRoomBounds() const { return RoomBounds; }
	USceneComponent* GetEntrance() const { return EntranceMarker; }
	TArray<ADungeonRoomBase*> GetConnectedNeighbors() const { return ConnectedNeighbors; }

	UDungeonActivatorSpawner* GetActivatorSpawner() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Config")
	ERoomType RoomType = ERoomType::Room;

	FOnPlayerEnterRoom OnPlayerEnter;

	void AddNeighbor(ADungeonRoomBase* Neighbor);

	TArray<FTransform> GetSpawnPoints() const;

	void SetRoomActive(bool bActive);

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
