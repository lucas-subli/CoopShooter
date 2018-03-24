// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPSHOOTER_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	/* Time between power up ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total times we apply the power up effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	// Handler for the power up ticks
	FTimerHandle TimerHandle_PowerupTick;

	// Total number of ticks applied
	int32 TicksProcessed;

	// Keeps state of the power-up
	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
	bool bIsPowerupActive;

	// Tick power up
	UFUNCTION()
	void OnTickPowerup();

	// Replicating
	UFUNCTION()
	void OnRep_PowerupActive();

	// When something happens with the power up
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupStateChanged(bool bNewIsActive);

public:

	// Activate the power
	void ActivatePowerup(AActor* ActiveFor);

	// On receiving power up buff
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* ActiveFor);

	// On each tick of the power up
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	// On power up timer expired
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();
};
