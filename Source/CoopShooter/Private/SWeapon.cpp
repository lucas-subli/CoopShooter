// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoopShooter.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

static int32 DebugWeaponDrawing;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw debug lines for weapons"), ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Setup mesh component
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.0f;
	CriticalMultiplier = 2.0f;
	FireRate = 300;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / FireRate;
	LastFiredTime = -TimeBetweenShots; // So you can begin the game firing =)
}

// Called to fire the weapon (hit scan)
void ASWeapon::Fire() {
	//Trace the world, from pawn eyes to crosshairs location
	AActor* MyOwner = GetOwner();
	if (!ensure(MyOwner != nullptr)) return;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	FVector EyeLocation;
	FRotator EyeRotation;

	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	// Particle target end point
	FVector TracerEndPoint = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	// if blocking hit process hit
	if (World->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_COLLISION_WEAPON, QueryParams)) {

		AActor* HitActor = Hit.GetActor();
		TraceEnd = Hit.ImpactPoint;

		float ActualDamage = BaseDamage;
		if (UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()) == SURFACE_FLESHVULNERABLE) {
			ActualDamage *= CriticalMultiplier;
		}
		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
	}

	if (DebugWeaponDrawing > 0) {
		DrawDebugLine(World, EyeLocation, TraceEnd, FColor::Green, false, 1.0f, 0, 1.0f);
	}

	PlayFireEffects(TraceEnd, &Hit);

	LastFiredTime = GetWorld()->TimeSeconds;
}

void ASWeapon::StartFire() {

	float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire() {
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TraceEnd, FHitResult* Hit) {
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	UParticleSystem* SelectedEffect = nullptr;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit->PhysMaterial.Get());

	switch (SurfaceType) {
	case SURFACE_FLESHDEFAULT:
		SelectedEffect = FleshImpactEffect;
		break;
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshVulnerableImpactEffect;
		if (CriticalSound) {
			UGameplayStatics::PlaySound2D(World, CriticalSound);
		}
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}


	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(World, TracerEffect, MuzzleLocation);

		if (TracerComp) {
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	if (SelectedEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(World, SelectedEffect, Hit->ImpactPoint, Hit->ImpactNormal.Rotation());
	}
	
	if (FireSound) {
		UGameplayStatics::PlaySound2D(World, FireSound);
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

