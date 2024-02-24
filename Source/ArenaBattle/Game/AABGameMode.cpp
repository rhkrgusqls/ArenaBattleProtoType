// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AABGameMode.h"

AAABGameMode::AAABGameMode()
{
	//Deafault Pawn Class
	static ConstructorHelpers::FClassFinder<APawn> ABCharacterPlayerClassRef(TEXT("/Game/BluePrint/BP_Player.BP_Player_C"));

	if (ABCharacterPlayerClassRef.Class)
	{
		DefaultPawnClass = ABCharacterPlayerClassRef.Class;
	}
	//player controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.AABPlayerController"));

	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}
