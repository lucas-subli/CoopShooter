// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class UMaterialInstanceDynamic;
class UParticleSystem;
class USoundCue;
class USphereComponent;

UCLASS()
class COOPSHOOTER_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Gets the next path point to move to
	FVector GetNextPathPoint();

	// Self destructs when near an enemy
	void SelfDestruct();

	// Damage Self for suicide
	void DamageSelf();

	// Check if there are nearby bots for power up
	void OnCheckNearbyBots();

	// refreshes the path, avoid getting stuck 
	void RefreshPath();

	// Callback for when our health changes
	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float NewHealth, float HealthDelta, const class UDamageType* DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	// Visual representation
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	// Health !
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	// Collision Detection for EXPLOSION baby !
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	// Next point to go to
	FVector NextPathPoint;

	// Amount of force to use in the movement
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	// Should use velocity change on movement
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	// Proximity threshold to be considered on target
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ProximityTresh;

	// Have we already exploded ?
	bool bExploded;

	// Have we already programmed a self destruct 
	bool bStartedSelfDestruct;

	// Material to pulse
	UMaterialInstanceDynamic* MatInst;

	// Damage type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DmgType;

	// Damage value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DamageValue;

	// Explosion radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DamageRadius;

	// Explosion radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float SelfDamageInterval;

	// Distance to check for nearby bots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float NearbyBonusRange;

	// Effect to play when we explode
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	// Sound for EXPLOSIONS
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	USoundCue* ExplosionSound;

	// Sound for Self destruct warning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	USoundCue* SelfDestructWarningSound;

	// the power boost of the bot, affects damaged caused to enemies and color of the bot (range: 1 to MaxPower level)
	int32 PowerLevel;

	// the max power boost of the bot
	int32 MaxPowerLevel = 4;

	// Timer handle to keep refreshing the path
	FTimerHandle TimerHandle_RefreshPath;

	// Timer for self destruct
	FTimerHandle TimerHandle_SelfDamage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Notify overlap
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};