// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"




void ASProjectileWeapon::Fire() {

	//Throw a projectile
	AActor* MyOwner = GetOwner();
	if (!ensure(MyOwner != nullptr)) return;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	FVector EyeLocation;
	FRotator EyeRotation;

	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	/*FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);*/

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ProjectileClass) {
		World->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}
}
