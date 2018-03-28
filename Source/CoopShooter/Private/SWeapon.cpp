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
#include "Net/UnrealNetwork.h"

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

	BaseDamage = 21.0f;
	CriticalMultiplier = 2.0f;
	FireRate = 300;
	BulletSpread = 0.5f;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
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

	if (Role < ROLE_Authority) {
		ServerFire();
	}

	//Trace the world, from pawn eyes to crosshairs location
	AActor* MyOwner = GetOwner();
	if (!ensure(MyOwner != nullptr)) return;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	FVector EyeLocation;
	FRotator EyeRotation;

	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// Bullet spread
	FVector ShotDirection = EyeRotation.Vector();
	float HalfRad = FMath::DegreesToRadians(BulletSpread);
	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	// Particle target end point
	FVector TracerEndPoint = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	EPhysicalSurface SurfaceHit = SurfaceType_Default;
	FHitResult Hit;
	// if blocking hit process hit
	if (World->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_COLLISION_WEAPON, QueryParams)) {

		AActor* HitActor = Hit.GetActor();
		TraceEnd = Hit.ImpactPoint;
		SurfaceHit = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		float ActualDamage = BaseDamage;
		if (SurfaceHit == SURFACE_FLESHVULNERABLE) {
			ActualDamage *= CriticalMultiplier;
		}
		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

		PlayImpactEffects(SurfaceHit, TraceEnd);
	}

	if (DebugWeaponDrawing > 0) {
		DrawDebugLine(World, EyeLocation, TraceEnd, FColor::Green, false, 1.0f, 0, 1.0f);
	}

	PlayFireEffects(TraceEnd);

	if (Role == ROLE_Authority) {
		HitScanTrace.TraceTo = TraceEnd;
		HitScanTrace.SurfaceType = SurfaceHit;
		// Magic number just to force update on multiple sequence shots
		HitScanTrace.RandomSeed = FMath::Rand();
		//UE_LOG(LogTemp, Warning, TEXT("Hitscan seed: %i"), HitScanTrace.RandomSeed);
	}

	LastFiredTime = GetWorld()->TimeSeconds;
}

void ASWeapon::ServerFire_Implementation() {
	Fire();
}

bool ASWeapon::ServerFire_Validate() {
	return true;
}

void ASWeapon::StartFire() {

	float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire() {
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TraceEnd) {
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

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

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	UParticleSystem* SelectedEffect = nullptr;

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

	if (SelectedEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(World, SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}

}

void ASWeapon::OnRep_HitScanTrace() {

	//UE_LOG(LogTemp, Warning, TEXT("Replicated!"));
	//Play cosmetic effects for replicated stuff
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}

