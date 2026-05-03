// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectNovaGameMode.h"
#include "ProjectNovaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectNovaGameMode::AProjectNovaGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
