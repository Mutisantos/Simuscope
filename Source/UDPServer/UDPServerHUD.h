// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "UDPServerHUD.generated.h"

UCLASS()
class AUDPServerHUD : public AHUD
{
	GENERATED_BODY()

public:
	AUDPServerHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

