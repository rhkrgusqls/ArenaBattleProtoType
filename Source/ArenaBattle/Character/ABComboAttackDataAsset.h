// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ABComboAttackDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABComboAttackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UABComboAttackDataAsset();

public:
	UPROPERTY(EditAnywhere, Category = Section)
	FString MontageSectionPrefix;

	UPROPERTY(EditAnywhere, Category = Section)
	uint8 MaxComboCount;

	UPROPERTY(EditAnywhere, Category = Section)
	float FrameRate;

	UPROPERTY(EditAnywhere, Category = Section)
	TArray<float> EffectiveFrameCount;
};
