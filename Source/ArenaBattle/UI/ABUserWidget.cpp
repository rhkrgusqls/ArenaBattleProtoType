// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ABUserWidget.h"
#include "Interface/ABCharacterWidgetInterface.h"
#include "Components/ProgressBar.h"

void UABUserWidget::UpdateHp(float NewHp)
{
	if (HpPrgressBar)
	{
		HpPrgressBar->SetPercent(NewHp / MaxHp);
	}
}

void UABUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HpPrgressBar = Cast<UProgressBar> (GetWidgetFromName(TEXT("PB_HpBar")));

	IABCharacterWidgetInterface* CharacterWidget = Cast<IABCharacterWidgetInterface> (OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
}
