// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UDPServer.h"
#include "UDPServerGameMode.h"
#include "UDPServerHUD.h"
#include "UDPServerCharacter.h"

AUDPServerGameMode::AUDPServerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUDPServerHUD::StaticClass();
}
