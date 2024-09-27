// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiTPSGameMode.h"
#include "MultiTPSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiTPSGameMode::AMultiTPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> 
		PlayerPawnBPClass(TEXT("/Game/_MultiTPS/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
