// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopGame/Public/World/Lobby/CoopLobbyGameMode.h"


void ACoopLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, "Player connected. Total: " + GetNumPlayers());
}

void ACoopLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, "Player connected. Total: " + GetNumPlayers());
}

void ACoopLobbyGameMode::StartGame()
{
	if (GetNumPlayers() < MinPlayersToStart)
	{
		return; 
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel("L_MainLevel");
	}
}