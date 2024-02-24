// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ABItemData.h"

FPrimaryAssetId UABItemData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("ABItemData", GetFName());
}
