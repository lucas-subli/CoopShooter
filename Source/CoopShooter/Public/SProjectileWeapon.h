// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPSHOOTER_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()
	
protected:

	virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
};
