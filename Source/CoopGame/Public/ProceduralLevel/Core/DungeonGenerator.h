// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonRoomBase.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"

class AActivator;
class UDungeonActivatorSpawner;
class ANavMeshBoundsVolume;
class ADungeonDirector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonGenerationFinished);

USTRUCT(BlueprintType)
struct FPendingExit
{
    GENERATED_BODY()

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    AActor* ParentRoom;
};

UCLASS()
class COOPGAME_API ADungeonGenerator : public AActor
{
    GENERATED_BODY()
    
public:    
    ADungeonGenerator();

    UPROPERTY(BlueprintAssignable, Category = "Dungeon Events")
    FOnDungeonGenerationFinished OnGenerationFinished;
    
protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    TArray<TSubclassOf<ADungeonRoomBase>> RoomClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    TSubclassOf<AActor> EndWallClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    TSubclassOf<ADungeonRoomBase> StartRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    int32 MaxRooms = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    float GenerationStepDelay = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Config")
    TArray<TSubclassOf<ADungeonRoomBase>> CapRoomClasses;

    UPROPERTY(EditAnywhere, Category = "Dungeon Logic")
    ADungeonDirector* DungeonDirector;

    UPROPERTY(EditAnywhere, Category = "Dungeon Logic")
    ANavMeshBoundsVolume* NavMeshBoundsVolume;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon Logic")
    TArray<ADungeonRoomBase*> GeneratedRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Validation")
    int32 MinRoomsToValidate = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Validation")
    int32 MaxGenerationRetries = 10;

    int32 CurrentRetries = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Exit")
    TSubclassOf<ADungeonRoomBase> ExitRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Exit")
    float MinExitDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Exit")
    int32 MinRoomsBeforeExit = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Exit")
    bool bStopGenerationOnExitSpawn = false;

    bool bExitSpawned = false;

    UPROPERTY(EditAnywhere, Category = "Puzzle")
    TSubclassOf<AActor> ActivatorClass;

    UPROPERTY(EditAnywhere, Category = "Puzzle")
    int32 MaxAmountOfActivators = 3;

    UPROPERTY(EditAnywhere, Category = "Puzzle")
    TArray<AActor*> ActivatorsGenerated;
    
    TArray<FTransform> ActivatorSpawnersLocations;
    
    FVector StartLocation;

    int32 CurrentRoomCount = 0;
    
    TArray<FPendingExit> PendingExits;

    FTimerHandle GenerationTimerHandle;

    TSubclassOf<ADungeonRoomBase> PreviousDungeonClass = nullptr;

    void StepGeneration();
    
    bool TrySpawnRoomAt(const FTransform& ExitTransform, AActor* ParentActor, ERoomType ParentType);

    bool CanRoomFit(const TSubclassOf<ADungeonRoomBase>& RoomClass, const FTransform& TargetTransform) const;

    void SpawnEndWall(const FTransform& TargetTransform);

    void SortCandidatesByRhythm(TArray<TSubclassOf<ADungeonRoomBase>>& Candidates, ERoomType ParentType);

    bool TrySpawnCapRoom(const FTransform& ExitTransform);

    bool TrySpawnExitRoom(const FTransform& ExitTransform);

    void ResetDungeon();

    void StartGeneration();
    
#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "Dungeon Debug")
    void DebugResetDungeon();
#endif
};