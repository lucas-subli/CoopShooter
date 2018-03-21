// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class ASWeapon;
class UCameraComponent;
class USHealthComponent;
class USpringArmComponent;

UCLASS()
class COOPSHOOTER_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

private:
	// Mover forward action
	void MoveForward(float Value);

	// Move right action
	void MoveRight(float Value);

	// Begin crouch action
	void BeginCrouch();

	// End Crouch action
	void EndCrouch();

	// Begin zoom action
	void BeginZoom();

	// End zoom action
	void EndZoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Fires the current weapon
	void StartFire();

	// Stop Firing the current weapon
	void StopFire();

	// Handle damage / Heal
	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float NewHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// The view camera variable for this character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	// The spring arm to hold the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	// The spring arm to hold the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	USHealthComponent* HealthComp;

	// Controls aims down the sight
	bool bWantsToZoom;

	// Are we dead ?
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bDied;

	// Controls how fast we can zoom in
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	// Control the value of the zoom
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	float ZoomedFOV;

	// Controls the default FOV
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	float DefaultFOV;

	// Control the value of the zoom
	float DefaultMaxWalkSpeed;

	// Controls the default FOV
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float AimingWalkSpeed;

	// Current weapon owned by the player
	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	// Default weapon to spawn with
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	// Weapon attachment point
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocketName;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Override to use our camera
	virtual FVector GetPawnViewLocation() const override;
	
	
};
