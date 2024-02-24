// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AABPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AAABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	void BeginPlay() override;
};
