// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "SWeapon.generated.h"


class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class COOPSHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Play fire effects
	void PlayFireEffects(FVector TraceEnd);

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

	// Particle System for muzzle flash
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	UParticleSystem* MuzzleEffect;

	// Particle System for Hit impact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	UParticleSystem* ImpactEffect;

	// Particle System for bullet trajectory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	UParticleSystem* TracerEffect;

	// Camera Shake
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	TSubclassOf<UCameraShake> FireCamShake;

	// Sound for firing weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	USoundBase* FireSound;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	
	// Fire the weapon
	UFUNCTION(BlueprintCallable, Category = "Fire")
	virtual void Fire();
};
