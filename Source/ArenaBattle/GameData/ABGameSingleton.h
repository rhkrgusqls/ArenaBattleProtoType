// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ABCharacterStat.h"
#include "ABGameSingleton.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABGameSingleton : public UObject
{
	GENERATED_BODY()

public:
	UABGameSingleton();
	static UABGameSingleton& Get();

	FORCEINLINE uint32 GetMaxLevel() { return MaxLevel; }
	FORCEINLINE TArray<FABCharacterStat> GetCharacterStatTable() const { return CharacterStatTable; }
	FORCEINLINE FABCharacterStat GetCharacterStatTable(int32 InLevel) const { 
		return CharacterStatTable.IsValidIndex(InLevel - 1) ? CharacterStatTable[InLevel 
			- 1] : FABCharacterStat(); }

private:
	TArray<FABCharacterStat> CharacterStatTable;
	uint32 MaxLevel;
	
};
