// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// On health changed event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, NewHealth, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPSHOOTER_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// on damage taken
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Replicate health
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	// Max Health points possible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	// Health points owned at the moment
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

public:	
	// Called every frame
	/*virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;*/

	// Set current Health points
	float SetCurrentHealth(float NewHealth);

	// Set max Health points
	float SetMaxHealth(float NewMax);

	// On health Changed
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
	
};
