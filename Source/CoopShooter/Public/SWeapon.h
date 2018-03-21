// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "SWeapon.generated.h"


class UCameraShake;
class UDamageType;
class UParticleSystem;
class USkeletalMeshComponent;

// Contains information of a single hit scan weapon line trace for net replication
USTRUCT()
struct FHitScanTrace {
	GENERATED_BODY()
public:
	// Magic number just to force update on multiple sequence shots
	UPROPERTY()
	int32 RandomSeed;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

};

UCLASS()
class COOPSHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

private:

	// Derived from fire rate
	float TimeBetweenShots;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Fire the weapon
	virtual void Fire();

	// Fire the weapon in the server
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	// Play fire effects
	void PlayFireEffects(FVector TraceEnd);

	// Play impact effects
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	// Replicate hit scan traces
	UFUNCTION()
	void OnRep_HitScanTrace();

	// Mesh for the weapon (Setup in blueprint)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	// Damage type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	TSubclassOf<UDamageType> DamageType;

	// Muzzle socket name for particle spawn
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	FName MuzzleSocketName;

	// Target name for tracer particle spawn
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	FName TracerTargetName;

	// Base Damage to apply
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float BaseDamage;

	// Critical hit damage multiplier
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float CriticalMultiplier;

	// Rate of fire for the weapon in shots per minute
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float FireRate;

	// Particle System for muzzle flash
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* MuzzleEffect;

	// Particle System for Hit impact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* DefaultImpactEffect;

	// Particle System for Hit impact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* FleshImpactEffect;

	// Particle System for Hit impact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* FleshVulnerableImpactEffect;

	// Particle System for bullet trajectory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* TracerEffect;

	// Camera Shake
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	TSubclassOf<UCameraShake> FireCamShake;

	// Sound for firing weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* FireSound;

	// Sound for critical hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* CriticalSound;

	// Time between shots in auto fire
	FTimerHandle TimerHandle_TimeBetweenShots;

	// Time between shots in auto fire
	float LastFiredTime;

	// Hit Scan trace to replicate over the network
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// Auto fire the weapon
	virtual void StartFire();

	// Stop auto firing the weapon the weapon
	virtual void StopFire();
};