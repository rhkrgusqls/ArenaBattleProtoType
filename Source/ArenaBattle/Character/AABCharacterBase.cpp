// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AABCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/ABCharacterControllDataAsset.h"
#include "Character/ABComboAttackDataAsset.h"
#include "UI/ABWidgetComponent.h"
#include "UI/ABUserWidget.h"
#include "Item/ABItemData.h"
#include "Item/ABWeaponItemData.h"
#include "Item/ABPotionItemData.h"
#include "Item/ABScrollItemData.h"

// Sets default values
AAABCharacterBase::AAABCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Stat Component
	Stat = CreateDefaultSubobject<UABCharacterStateComponent>(TEXT("Stat"));

	// UI Widget
	HpBar = CreateDefaultSubobject<UABWidgetComponent>(TEXT("HpBar"));
	HpBar->SetupAttachment(GetMesh());
	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

	// Weapon Mesh Component
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	static ConstructorHelpers::FClassFinder<UUserWidget> HpBarWidgetRef(TEXT("/Game/ArenaBattle/UI/WBP_HpBar.WBP_HpBar_C"));
	if (HpBarWidgetRef.Class)
	{
		HpBar->SetWidgetClass(HpBarWidgetRef.Class);
		HpBar->SetWidgetSpace(EWidgetSpace::Screen);
		HpBar->SetDrawSize(FVector2D(150.0f, 20.0f));
		HpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//캡슐컴포넌트
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	//프로젝트 콜리젼 설정 프리셋 이름 정하는 코드
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCapsule"));

	//스켈레톤 매쉬
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	//캐릭터 무브먼트 컴포넌트
	//이동하는 방향으로 바라보는 방향 변경
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//Skeletal Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/sk_CharM_Base.sk_CharM_Base"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	//Animation
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/ArenaBattle/Animation/ABP_CharacterAnimation.ABP_CharacterAnimation_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}
	//데이터 에셋, CharacterControl DataAsset
	static ConstructorHelpers::FObjectFinder<UABCharacterControllDataAsset> ShoulderDataAssetRef(TEXT("/Game/ArenaBattle/CharacterControlData/DA_CCShoulder.DA_CCShoulder"));
	if (ShoulderDataAssetRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Shoulder, ShoulderDataAssetRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UABCharacterControllDataAsset> QuaterDataAssetRef(TEXT("/Game/ArenaBattle/CharacterControlData/DA_CCQuater.DA_CCQuater"));
	if (QuaterDataAssetRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Quater, QuaterDataAssetRef.Object);
	}

	/*static ConstructorHelpers::FObjectFinder<AABGate> GateRef(TEXT("/Game/BluePrint/BP_Gate.BP_Gate"));
	if (GateRef.Object)
	{
		Gate = GateRef.Object;
	}*/
	//OnNPCDead.BindUObject(Gate, &AABGate::GateOpen);

	Stat->OnHpZero.AddUObject(this, &AAABCharacterBase::SetDead);

	
	
}

void AAABCharacterBase::SetupCharacterWidget(UABUserWidget* InUserWidget)
{
	if (InUserWidget)
	{
		InUserWidget->SetMaxHP(Stat->GetTotalStat().MaxHP);
		InUserWidget->UpdateHp(Stat->GetCurrentHP());
		Stat->OnHpChanged.AddUObject(InUserWidget, &UABUserWidget::UpdateHp);
	}
}

void AAABCharacterBase::SetCharacterControlData(const UABCharacterControllDataAsset* CharacterControlData)
{
	//pawn
	bUseControllerRotationPitch =  CharacterControlData->bUseControlRotationPitch;
	bUseControllerRotationYaw = CharacterControlData->bUseControlRotationYaw;
	bUseControllerRotationRoll = CharacterControlData->bUseControlRotationRoll;

		//Movement
	GetCharacterMovement()->bOrientRotationToMovement =  CharacterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;
}

void AAABCharacterBase::ProcessComboCommand()
{
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//AnimInstance->Montage_Play(ComboActionMontage);

	if (CurrentComboCount == 0)
	{
		ComboActionBegin();
		return;
	}

	if (ComboTimerHandle.IsValid() && !ComboStartTimerHandle.IsValid())
	{
		HasNextComboCommand = true;
	}
	else
	{
		HasNextComboCommand = false;

		GetWorld()->GetTimerManager().ClearTimer(ComboStartTimerHandle);
		ComboStartTimerHandle.Invalidate();
		GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
		ComboTimerHandle.Invalidate();
	}
}

void AAABCharacterBase::ComboActionBegin()
{
	IsAttack = true;
	CurrentComboCount = 1;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	const float AttackSpeedRate = Stat->GetTotalStat().AttackSpeed;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ComboActionMontage, AttackSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AAABCharacterBase::ComboActionEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboActionMontage);
	
	SetComboStartCheckTimer();
}

void AAABCharacterBase::ComboActionEnd(UAnimMontage* TargetMontage, bool bInterrupted)
{
	IsAttack = false;
	CurrentComboCount = 0;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AAABCharacterBase::SetComboStartCheckTimer()
{
	GetWorld()->GetTimerManager().SetTimer(ComboStartTimerHandle, this, &AAABCharacterBase::SetComboCheckTimer, 0.3f, false);
}

void AAABCharacterBase::SetComboCheckTimer()
{
	if (CurrentComboCount <= 0)	return;

	float ComboEffectiveTime = ComboAttackData->EffectiveFrameCount[CurrentComboCount - 1] / ComboAttackData->FrameRate;  // (17 / 30) (0 / 30)
	if (ComboEffectiveTime > 0.0f)
	{
		ComboStartTimerHandle.Invalidate();
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &AAABCharacterBase::ComboCheck, 0.3f, false);
	}
}

void AAABCharacterBase::ComboCheck()
{
	if (HasNextComboCommand)
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, ComboAttackData->MaxComboCount);

		FName NextSectionName = *FString::Printf(TEXT("%s%d"), *ComboAttackData->MontageSectionPrefix, CurrentComboCount);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_JumpToSection(NextSectionName, ComboActionMontage);

		SetComboStartCheckTimer();
		HasNextComboCommand = false;
	}
}

void AAABCharacterBase::SetDead()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorEnableCollision(false);

	//Dead Animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(DeadMontage, 1.0f);

	GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			Destroy();
		}), 3.0f, false);

	//OnNPCDead.ExecuteIfBound();
}

void AAABCharacterBase::TakeItem(UABItemData* InItemData)
{
	// 획득한 아이템 추후 구현

	UE_LOG(LogTemp, Log, TEXT("%d"), InItemData->Type);		// 로그 찍을때 스트링타입으로 받으려 했는데 형변환이 안됨...

	/*switch (InItemData->Type)
	{
	case EItemType::Scroll:
		break;
	case EItemType::Weapon:
		break;
	case EItemType::Potion:
		break;
	}*/

	if (InItemData)
	{
		TakeItemActions[(uint8)InItemData->Type].ItemDelegate.ExecuteIfBound(InItemData);
	}


}
int32 AAABCharacterBase::GetLevel()
{
	return Stat->GetCurrentLevel();
}
void AAABCharacterBase::SetLevel(int32 InLevel)
{
	Stat->SetLevelStat(InLevel);
}

void AAABCharacterBase::EquipWeapon(UABItemData* InItemData)
{
	UE_LOG(LogTemp, Log, TEXT("EquipWeapon"));

	UABWeaponItemData* WeaponItemData = Cast<UABWeaponItemData>(InItemData);
	if (WeaponItemData)
	{
		if (WeaponList.Contains(WeaponItemData->WeaponMesh))
		{

			return;
		}

		if (WeaponItemData->WeaponMesh.IsPending())
		{
			WeaponItemData->WeaponMesh.LoadSynchronous();
		}
		WeaponList.Add(WeaponItemData->WeaponMesh.Get());

		Weapon->SetSkeletalMesh(WeaponItemData->WeaponMesh.Get());
		Stat->SetModifierStat(WeaponItemData->ModifierStat);
	}
}

void AAABCharacterBase::DrinkPotion(UABItemData* InItemData)
{
	UE_LOG(LogTemp, Log, TEXT("DrinkPotion"));
}

void AAABCharacterBase::ReadScroll(UABItemData* InItemData)
{
	UE_LOG(LogTemp, Log, TEXT("ReadScroll"));
}



