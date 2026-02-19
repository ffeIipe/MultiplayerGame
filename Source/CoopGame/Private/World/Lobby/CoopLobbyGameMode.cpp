// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopGame/Public/World/Lobby/CoopLobbyGameMode.h"


void ACoopLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GetNumPlayers();

	UE_LOG(LogTemp, Warning, TEXT("Jugador conectado. Total: %d"), NumberOfPlayers);
}

void ACoopLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	const int32 NumberOfPlayers = GetNumPlayers();
	UE_LOG(LogTemp, Warning, TEXT("Jugador desconectado. Restantes: %d"), NumberOfPlayers);
}

void ACoopLobbyGameMode::StartGame()
{
	if (GetNumPlayers() < MinPlayersToStart)
	{
		UE_LOG(LogTemp, Error, TEXT("No se puede iniciar. Faltan jugadores."));
		return; 
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel("L_MainLevel");
	}
}