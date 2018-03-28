// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoopShooterGameModeBase.h"
#include "SLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPSHOOTER_API ASLobbyGameMode : public ACoopShooterGameModeBase
{
	GENERATED_BODY()
	
public:

	void PostLogin(APlayerController* NewPlayer) override;

	void Logout(AController* Exiting) override;

private:
	void StartGame();

	uint32 NumberOfPlayers = 0;

	FTimerHandle GameStartTimer;
	
	
	
};
