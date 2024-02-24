// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ABAnimNotify_AttackHitCheck.h"
//#include "Character/AABCharacterPlayer.h"
//#include "Character/ABCharacterNonPlayer.h"
#include "Interface/ABAnimationAttackInterface.h"

void UABAnimNotify_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	//(AAABCharacterPlayer*)(MeshComp->GetOwner())->AttackHitCheck();
	//(AABCharacterNonPlayer*)(MeshComp->GetOwner())->AttackHitCheck();

	IABAnimationAttackInterface* AttackInterface = Cast<IABAnimationAttackInterface>(MeshComp->GetOwner());
	if (AttackInterface)
	{
		AttackInterface->AttackHitCheck(AttackType);
	}
}
