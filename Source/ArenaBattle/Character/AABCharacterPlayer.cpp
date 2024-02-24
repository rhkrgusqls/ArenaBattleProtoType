// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AABCharacterPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/ABCharacterControllDataAsset.h"
#include "Character/ABComboAttackDataAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"

AAABCharacterPlayer::AAABCharacterPlayer()
{
	//Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;
	//Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	//Input

	//static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/ThirdPerson/Input/IMC_Default.IMC_Default'"));
	//if (InputMappingContextRef.Object)
	//{
	//	DefaultMappingContext = InputMappingContextRef.Object;
	//}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove"));
	if (InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove"));
	if (InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook"));
	if (InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump"));
	if (InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionChangeControlRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl"));
	if (InputActionChangeControlRef.Object)
	{
		ChangeControlAction = InputActionChangeControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack"));
	if (InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputWeaponChangeRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeWeapon.IA_ChangeWeapon'"));
	if (InputWeaponChangeRef.Object)
	{
		weaponChangeAction = InputWeaponChangeRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// Take Item Section
	TakeItemActions.Add(FTakeItemDelegateWrapper(FOnTakeItemDelegate::CreateUObject(this, &AAABCharacterPlayer::EquipWeapon)));
	TakeItemActions.Add(FTakeItemDelegateWrapper(FOnTakeItemDelegate::CreateUObject(this, &AAABCharacterPlayer::DrinkPotion)));
	TakeItemActions.Add(FTakeItemDelegateWrapper(FOnTakeItemDelegate::CreateUObject(this, &AAABCharacterPlayer::ReadScroll)));
}

void AAABCharacterPlayer::AttackHitCheck(AttackType AttackType)
{
	// 공격 충돌 판정을 한다

	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(Attack), false, this);
	FHitResult OutHitResult;
	const float AttackRange = Stat->GetTotalStat().AttackRange;
	const float CapsuleRadius = 50.0f;
	const float Damage = Stat->GetTotalStat().Attack;

	const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius(); //판정 범위 시작
	const FVector End = Start + GetActorForwardVector() * AttackRange; //판정 범위 끝
	TArray<FOverlapResult> OutOverlapResults;

	bool IsHit;

	switch (AttackType)
	{
	case AttackType::SingleAttack:
		IsHit = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(CapsuleRadius), CollisionParams);
		break;
	case AttackType::MultiAttack:
		IsHit = GetWorld()->OverlapMultiByChannel(OutOverlapResults, GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRange), CollisionParams);
		break;
	}
	//결과값 받아올 구조체, 시작점, 끝점, 트레이스채널, 콜리전 형태, 콜리전 파라미터?

	//bool IsHit = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(CapsuleRadius), CollisionParams);

	//const float LastAttackRange = 300.0f;
	const float SphereRadius = 100.0f;
	//bool IsLastAttackHit = GetWorld()->OverlapAnyTestByChannel(Start, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), CollisionParams);
	/*
	if (CurrentComboCount == ComboAttackData->MaxComboCount)
	{
		IsHit = GetWorld()->OverlapMultiByChannel(LastHitted, Start, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), CollisionParams);
	}
	else
	{
		IsHit = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(CapsuleRadius), CollisionParams);
	}
	*/

	if(IsHit)
	{
		FDamageEvent DamageEvent;
		if (OutHitResult.GetActor())
		{
			OutHitResult.GetActor()->TakeDamage(Damage, DamageEvent, GetController(), this);
		}
		if (OutOverlapResults.Num()>0)
		{
			
		}
	}

#if ENABLE_DRAW_DEBUG

	FVector CapsulePosition = Start + (End - Start) / 2.0f;
	float HalfHeight = AttackRange / 2.0f;


	FColor Color = IsHit ? FColor::Green : FColor::Red;

	switch (AttackType)
	{
	case AttackType::SingleAttack:
		DrawDebugCapsule(GetWorld(), CapsulePosition, HalfHeight, CapsuleRadius,
			FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), Color, false, 3.0f);
		break;
	case AttackType::MultiAttack:
		DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRange, 8, Color, false, 3.0f);
		break;
	}
	
#endif
}



void AAABCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	//APlayerController* PlayerController =CastChecked<APlayerController>(GetController());
	//UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	//if (nullptr != Subsystem)
	//{
	//	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	//	//Subsystem->RemoveMappingContext(DefaultMappingContext);
	//}

	SetCharacterControl(ECharacterControlType::Quater);
}

void AAABCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//향상된 입력 시스템 사용
	//입력매핑컨텍스트에서 액션과 함수
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked <UEnhancedInputComponent> (PlayerInputComponent);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::ChangeControl);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::Attack);
	EnhancedInputComponent->BindAction(weaponChangeAction, ETriggerEvent::Triggered, this, &AAABCharacterPlayer::ChangeWeapon);

}

void AAABCharacterPlayer::SetCharacterControlData(const UABCharacterControllDataAsset* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);//상위클래스 설정 사용

	//Input
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (nullptr != Subsystem)
	{
		Subsystem->ClearAllMappings();
		if (CharacterControlData->InputMappingContext)
		{
			Subsystem->AddMappingContext(CharacterControlData->InputMappingContext, 0);
		}
		//Subsystem->RemoveMappingContext(DefaultMappingContext);
	}

	//Camera
	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bDoCollisionTest  = CharacterControlData->bDoCollisionTest;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
}

void AAABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	float MovementVectorsizeSquared = MovementVector.SquaredLength();
	if (MovementVectorsizeSquared > 1.0f)
	{
		//크기를 1로 고정
		MovementVector.Normalize();
		MovementVectorsizeSquared = 1.0f;
	}
	else
	{

	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorsizeSquared);
}

void AAABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

}

void AAABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AAABCharacterPlayer::ChangeControl()
{
	//현재 캐릭터 컨트롤 타입이 쿼터이면 숄더로 숄더면 쿼터로 전환
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		CurrentCharacterControlType = ECharacterControlType::Shoulder;
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		CurrentCharacterControlType = ECharacterControlType::Quater;
	}

	SetCharacterControl(CurrentCharacterControlType);

}

void AAABCharacterPlayer::SetCharacterControl(ECharacterControlType ControlType)
{
	UABCharacterControllDataAsset* NewCharacterControlData = CharacterControlManager[ControlType];
	SetCharacterControlData(NewCharacterControlData);

}

void AAABCharacterPlayer::Attack()
{
	ProcessComboCommand();
}

void AAABCharacterPlayer::ChangeWeapon()
{
	if (IsAttack == true)
	{
		return;
	}
	if (WeaponList.Num() < 1)
		return;
	curWeapon++;
	curWeapon = WeaponList.Num() > curWeapon ? curWeapon : 0;

	Weapon->SetSkeletalMesh(WeaponList[curWeapon]);

}