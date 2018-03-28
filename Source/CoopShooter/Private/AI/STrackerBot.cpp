// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw Debug Lines for TrackerBot"),
	ECVF_Cheat);

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
	HealthComp->SetMaxHealth(30.0f);

	// Initialize the Sphere comp
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	// Setup default values
	bStartedSelfDestruct = false;
	bUseVelocityChange = true;
	MovementForce = 1000.0f;
	ProximityTresh = 100.0f;
	DamageRadius = 250.0f;
	DamageValue = 40.0f;
	SelfDamageInterval = 0.3f;
	NearbyBonusRange = 600.0f;
	MaxPowerLevel = 4;
	SphereComp->SetSphereRadius(DamageRadius * 0.8);

	// Network replication
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority) {
		//Initial move to
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby bots
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}

	// Setup Delegates
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);
}

// FInds the next point
FVector ASTrackerBot::GetNextPathPoint() {
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this)) {
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetCurrentHealth() > 0.0f) {
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance) {
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget) {
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1) {
			// Return next point in the path
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct() {

	if (bExploded) return;

	bExploded = true;

	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}

	// Give time for the client to display effects
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float ActualDamage = DamageValue + (DamageValue * 0.5 * PowerLevel);

		// Apply damage
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), DamageRadius, DmgType, IgnoredActors, this, GetInstigatorController(), true);

		SetLifeSpan(2.0f);
	}
}

void ASTrackerBot::DamageSelf() {
	UGameplayStatics::ApplyDamage(this, 10.0f, GetInstigatorController(), this, DmgType);
}

void ASTrackerBot::OnCheckNearbyBots() {

	// Create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(NearbyBonusRange);

	// Only find Pawns (players and AI bots)
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bots mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	if (DebugTrackerBotDrawing) {
		DrawDebugSphere(GetWorld(), GetActorLocation(), NearbyBonusRange, 12, FColor::White, false, 1.0f);
	}

	int32 NrOfBots = 0;
	// loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps) {
		// Check if we overlapped with another tracker bot (ignoring players and other bot types)
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		// Ignore this tracker bot instance
		if (Bot && Bot != this) {
			NrOfBots++;
		}
	}

	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Update the material color
	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst) {
		// Convert to a float between 0 and 1 just like an 'Alpha'
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		// Note: (float)MaxPowerLevel converts the int32 to a float, so we don't end with integer division

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	// Draw on the bot location
	if (DebugTrackerBotDrawing) {
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}
}

void ASTrackerBot::RefreshPath() {
	NextPathPoint = GetNextPathPoint();
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

	if (NewHealth <= 0) {
		SelfDestruct();
		if (DebugTrackerBotDrawing) {
			DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12.0f, FColor::Red, false, 10.0f, 0, 1.0f);
		}
	}
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded) {
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= ProximityTresh) {
			NextPathPoint = GetNextPathPoint();
		} else {
			// keep moving towards target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing) {
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}
		if (DebugTrackerBotDrawing) {
			DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12.0f, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor) {

	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruct && !bExploded) {
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this)) {
			bStartedSelfDestruct = true;
			if (ROLE_Authority == ROLE_Authority) {
				// We overlapped with a player
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			if (SelfDestructWarningSound) {
				UGameplayStatics::SpawnSoundAttached(SelfDestructWarningSound, RootComponent);
			}
		}
	}
}

