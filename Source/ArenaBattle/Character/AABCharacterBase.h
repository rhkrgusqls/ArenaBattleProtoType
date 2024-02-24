// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/ABCharacterWidgetInterface.h"
#include "Interface/ABCharacterItemInterface.h"
#include "CharacterState/ABCharacterStateComponent.h"
#include "AABCharacterBase.generated.h"

//DECLARE_DELEGATE(FOnNPCDead);

UENUM()//이걸 붙여야 언리얼 시스템 내에 들어감
enum class ECharacterControlType : uint8 //enum문 생성
{
	Shoulder,
	Quater
};

DECLARE_DELEGATE_OneParam(FOnTakeItemDelegate, class UABItemData* /*InItemData*/);


USTRUCT(BlueprintType)
struct FTakeItemDelegateWrapper
{
	GENERATED_BODY()
	FTakeItemDelegateWrapper() {}
	FTakeItemDelegateWrapper(const FOnTakeItemDelegate& InItemDelegate) : ItemDelegate(InItemDelegate) {}

	FOnTakeItemDelegate ItemDelegate;
};

UCLASS()//이게 있어야 언리얼 시스템내에 들어감
class ARENABATTLE_API AAABCharacterBase : public ACharacter, public IABCharacterWidgetInterface, public IABCharacterItemInterface
{
	GENERATED_BODY()

	//FOnNPCDead OnNPCDead;

public:
	// Sets default values for this character's properties
	AAABCharacterBase();
	virtual void SetupCharacterWidget(class UABUserWidget* InUserWidget) override;
	virtual void TakeItem(UABItemData* InItemData) override;

	int32 GetLevel();
	void SetLevel(int32 InLevel);

protected:
	virtual void SetCharacterControlData(const class UABCharacterControllDataAsset* CharacterControlData);
	void ProcessComboCommand();

	void ComboActionBegin();
	void ComboActionEnd(UAnimMontage* TargetMontage, bool bInterrupted);

	void SetComboStartCheckTimer();
	void SetComboCheckTimer();
	void ComboCheck();

	virtual void SetDead();


	virtual void EquipWeapon(class UABItemData* InItemData);
	virtual void DrinkPotion(class UABItemData* InItemData);
	virtual void ReadScroll(class UABItemData* InItemData);


protected:
	TMap< ECharacterControlType, class UABCharacterControllDataAsset*> CharacterControlManager; // 생성자가 호출될떄 같이 메모리 할당

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> ComboActionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> DeadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttackData)
	TObjectPtr<class UABComboAttackDataAsset> ComboAttackData;

	uint8 CurrentComboCount;

	FTimerHandle ComboTimerHandle;
	FTimerHandle ComboStartTimerHandle;

	bool HasNextComboCommand = false;
	bool IsAttack = false;

	FTimerHandle DeadTimerHandle;

	// Stat Component
	UPROPERTY (VisibleAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UABCharacterStateComponent> Stat;

	// UI Widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widget, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UABWidgetComponent> HpBar;

	//// Gate
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gate)
	//TObjectPtr<class AABGate> Gate;

	// Take Item Section
	UPROPERTY()
	TArray<FTakeItemDelegateWrapper> TakeItemActions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment, meta = (AllowPrivateAccess="true"))
	TObjectPtr<class USkeletalMeshComponent> Weapon;

	//ChangeWeapon
	TArray<TObjectPtr<class USkeletalMesh>> WeaponList;

	uint8 curWeapon = 0;
};
