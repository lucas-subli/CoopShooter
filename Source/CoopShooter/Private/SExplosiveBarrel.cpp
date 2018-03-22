// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Setup health comp
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
	HealthComp->SetMaxHealth(40.0f);

	//Setup mesh comp
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	//Setup radial force
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = DamageRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Only when i tell it to
	RadialForceComp->bIgnoreOwningActor = true;

	ExplosionImpulse = 400;
	DamageRadius = 250.0f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float NewHealth, float HealthDelta, const class UDamageType* DamageType, 
	class AController* InstigatedBy, AActor* DamageCauser) {
	// already exploded nothing to do here
	if (bExploded) return;

	if (NewHealth <= 0) {
		//Explode
		bExploded = true;
		// This only runs (Due to health comp) on server so we have to call manually
		OnRep_Exploded();

		//Launch myself
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);


		FVector ExplosionOrigin = GetActorLocation();
		TArray<AActor*> IgnoreActors;

		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), 100.0f, 20.0f, ExplosionOrigin, DamageRadius / 2, DamageRadius, 10.0f, DmgType, IgnoreActors, this);
		RadialForceComp->FireImpulse();
	}
}

void ASExplosiveBarrel::OnRep_Exploded() {
	// Play effects
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}
