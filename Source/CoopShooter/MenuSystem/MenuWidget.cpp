// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuWidget.h"

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	if (!ensure(GetOwningPlayer() != nullptr)) return;
	GetOwningPlayer()->SetInputMode(InputModeData);
	GetOwningPlayer()->bShowMouseCursor = true;
}

void UMenuWidget::OnLevelRemovedFromWorld(ULevel * InLevel, UWorld * InWorld) {
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);

	FInputModeGameOnly InputModeData;

	if (!ensure(GetOwningPlayer() != nullptr)) return;
	GetOwningPlayer()->SetInputMode(InputModeData);
	GetOwningPlayer()->bShowMouseCursor = false;
}

void UMenuWidget::Teardown() {
	this->RemoveFromViewport();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = false;
}

void UMenuWidget::SetMenuInterface(IMenuInterface* MenuInterface) {
	this->MenuInterface = MenuInterface;
}


