// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPSHOOTER_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// SPhere comp for collision
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	// Decal for making it pretty
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	// Which power up should be spawned
	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerUpClass;

	// The instance to remember
	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	// Handle periodic respawn
	FTimerHandle TimerHandle_RespawnTimer;

	// Actually respawn
	void Respawn();

public:

	// Notifier for overlap when player passes on top of it
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
};
