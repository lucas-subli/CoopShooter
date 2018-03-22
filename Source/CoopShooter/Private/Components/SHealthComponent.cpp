// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetMaxHealth(100.0f);
	SetCurrentHealth(100.0f);

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();
	// Only hook on server
	if (MyOwner && GetOwnerRole() == ROLE_Authority) {
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {
	if (Damage <= 0.0f) return;

	CurrentHealth = SetCurrentHealth(CurrentHealth - Damage);

	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
}

float USHealthComponent::SetCurrentHealth(float NewHealth) {
	
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	return CurrentHealth;
}

float USHealthComponent::SetMaxHealth(float NewMax) {
	MaxHealth = FMath::Max(NewMax, 1.0f);
	if (CurrentHealth > MaxHealth) {
		CurrentHealth = MaxHealth;
	}
	return MaxHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth) {
	float Damage = CurrentHealth - OldHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, nullptr, nullptr, nullptr);

}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, CurrentHealth);
}