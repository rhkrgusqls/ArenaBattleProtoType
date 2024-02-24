// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AABPlayerController.h"

void AAABPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

}
