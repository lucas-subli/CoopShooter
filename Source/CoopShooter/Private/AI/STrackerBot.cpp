// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the mesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	// Initialize the Health
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);


	bUseVelocityChange = true;
	MovementForce = 1000;
	ProximityTresh = 100;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	//Initial move to
	NextPathPoint = GetNextPathPoint();
}

// FInds the next point
FVector ASTrackerBot::GetNextPathPoint() {
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (!ensure(PlayerPawn != nullptr)) return GetActorLocation();

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (!ensure(NavPath != nullptr)) return GetActorLocation();

	if (NavPath->PathPoints.Num() > 1) {
		// Point 0 is current location
		return NavPath->PathPoints[1];
	}

	// Already at the point!
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float NewHealth, float HealthDelta, const class UDamageType* DamageType, 
	class AController* InstigatedBy, AActor* DamageCauser) {

	// TODO: Pulse the material on hit
	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}


	// already exploded nothing to do here
	if (bExploded) return;

	if (NewHealth <= 0) {
		//Explode
		bExploded = true;
		// This only runs (Due to health comp) on server so we have to call manually
		//OnRep_Exploded();

		/*FVector ExplosionOrigin = GetActorLocation();
		TArray<AActor*> IgnoreActors;

		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), 100.0f, 20.0f, ExplosionOrigin, DamageRadius / 2, DamageRadius, 10.0f, DmgType, IgnoreActors, this);
		RadialForceComp->FireImpulse();*/
	}

	UE_LOG(LogTemp, Warning, TEXT("Health %s"), *FString::SanitizeFloat(NewHealth));
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

	if (DistanceToTarget <= ProximityTresh) {
		NextPathPoint = GetNextPathPoint();
	} else {
		// keep moving towards target
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();
		ForceDirection *= MovementForce;

		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
	}

	DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12.0f, FColor::Yellow, false, 0.0f, 1.0f);
}

