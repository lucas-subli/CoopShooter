// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class UMaterialInstanceDynamic;

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

	// Material to pulse
	UMaterialInstanceDynamic* MatInst;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};