// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "SGameMode.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;

	TeamNum = 255;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentHealth(MaxHealth);

	AActor* MyOwner = GetOwner();
	// Only hook on server
	if (MyOwner && GetOwnerRole() == ROLE_Authority) {
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {

	if (Damage <= 0.0f || bIsDead) return;

	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser)) {
		return;
	}

	CurrentHealth = SetCurrentHealth(CurrentHealth - Damage);

	bIsDead = CurrentHealth <= KINDA_SMALL_NUMBER;

	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead) {
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM) {
			if (InstigatedBy != nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("DIED: %s KILLED: %S"), *DamagedActor->GetName(), *InstigatedBy->GetName());
			} else {
				UE_LOG(LogTemp, Warning, TEXT("DIED: %s KILLED: %S"), *DamagedActor->GetName(), *DamageCauser->GetName());
			}
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float USHealthComponent::SetCurrentHealth(float NewHealth) {
	
 	UE_LOG(LogTemp, Warning, TEXT("HealthChanged from: %s to %s"), *FString::SanitizeFloat(CurrentHealth), *FString::SanitizeFloat(FMath::Clamp(NewHealth, 0.0f, MaxHealth)));
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	return CurrentHealth;
}

void USHealthComponent::Heal(float HealAmount) {

	if (HealAmount <= 0 || CurrentHealth <= 0) return;
	OnHealthChanged.Broadcast(this, CurrentHealth, -HealAmount, nullptr, nullptr, nullptr);
	SetCurrentHealth(CurrentHealth + HealAmount);
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


float USHealthComponent::GetCurrentHealth() const {
	return CurrentHealth;
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB) {
	//Friendly by default
	if (ActorA == nullptr || ActorB == nullptr) return true;

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	//Friendly by default
	if (HealthCompA == nullptr || HealthCompB == nullptr) return true;

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, CurrentHealth);
}