// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CoopLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ACoopLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable, Category = "GameLoop")
	void StartGame();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameLoop")
	int32 MinPlayersToStart = 2;
};
