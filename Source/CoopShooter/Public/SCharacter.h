// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The view camera variable for this character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	// The spring arm to hold the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Override to use our camera
	virtual FVector GetPawnViewLocation() const override;
	
	
};
