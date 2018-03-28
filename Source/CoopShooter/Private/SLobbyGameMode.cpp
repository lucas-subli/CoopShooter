// Fill out your copyright notice in the Description page of Project Settings.

#include "SLobbyGameMode.h"

#include "TimerManager.h"
#include "SGameInstance.h"
#include "Engine/World.h"



void ASLobbyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	++NumberOfPlayers;

	if (NumberOfPlayers >= 2) {
		GetWorldTimerManager().SetTimer(GameStartTimer, this, &ASLobbyGameMode::StartGame, 10);
	}
}

void ASLobbyGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	--NumberOfPlayers;
}

void ASLobbyGameMode::StartGame() {
	auto GameInstance = Cast<USGameInstance>(GetGameInstance());

	if (GameInstance == nullptr) return;
	GameInstance->StartSession();

	UWorld * World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;
	World->ServerTravel("/Game/Maps/Map1?listen");
}




