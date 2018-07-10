// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SEAGULLZGameMode.h"
#include "SEAGULLZHUD.h"
#include "SEAGULLZCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASEAGULLZGameMode::ASEAGULLZGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASEAGULLZHUD::StaticClass();
}
