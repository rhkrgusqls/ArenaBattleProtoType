// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/ABCommon.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ABAnimNotify_AttackHitCheck.generated.h"


/**
 * 
 */

UCLASS()
class ARENABATTLE_API UABAnimNotify_AttackHitCheck : public UAnimNotify
{
	GENERATED_BODY()
	
protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AttackType AttackType;
};
