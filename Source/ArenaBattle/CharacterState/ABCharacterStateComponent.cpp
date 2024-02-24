// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterState/ABCharacterStateComponent.h"
#include "GameData/ABGameSingleton.h"

// Sets default values for this component's properties
UABCharacterStateComponent::UABCharacterStateComponent()
{
	CurrentLevel = 0;
}


// Called when the game starts
void UABCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	SetLevelStat(CurrentLevel);
	SetHP(BaseStat.MaxHP);
	

	
}

void UABCharacterStateComponent::SetHP(float NewHP)
{
	CurrentHP = FMath::Clamp(NewHP, 0.0f, BaseStat.MaxHP);

	OnHpChanged.Broadcast(CurrentHP);
}


// Called every frame
void UABCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UABCharacterStateComponent::ApplyDamage(float InDamage)
{
	float ActualDamage = FMath::Clamp(InDamage, 0.0f, InDamage);
	
	SetHP(CurrentHP - ActualDamage);
	if (CurrentHP <= 0.0f)
	{
		//등록된 델리게이트를 호출하여 죽었다는 사실 알림
		OnHpZero.Broadcast();

	}

	return 0.0f;
}

void UABCharacterStateComponent::SetLevelStat(uint8 InNewLevel)
{
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UABGameSingleton::Get().GetMaxLevel());
	BaseStat = UABGameSingleton::Get().GetCharacterStatTable(CurrentLevel);
}

