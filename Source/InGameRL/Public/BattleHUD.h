// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BattleHUD.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API ABattleHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void DrawHUD() override;
};
