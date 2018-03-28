// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"

#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class COOPSHOOTER_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	void OnLevelRemovedFromWorld(ULevel * InLevel, UWorld * InWorld) override;

	void Teardown();

	void SetMenuInterface(IMenuInterface* MenuInterface);

protected:
	IMenuInterface * MenuInterface;
	
	
};
