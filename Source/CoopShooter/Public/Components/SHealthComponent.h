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

	// Team number to identify allies and enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	uint8 TeamNum;


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

	// Tells us if we are already dead (Omae Wa Mou Shindeiru)
	bool bIsDead;

public:	
	// Called every frame
	/*virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;*/

	// Heal the component
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float HealAmount);

	// Set max Health points
	float SetMaxHealth(float NewMax);

	// Set current Health points
	float SetCurrentHealth(float NewHealth);

	// Get current Health points
	float GetCurrentHealth() const;

	// Get current Health points
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Team")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);

	// On health Changed
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
	
};
