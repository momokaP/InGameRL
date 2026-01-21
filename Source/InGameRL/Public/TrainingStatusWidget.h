// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingStatusWidget.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API UTrainingStatusWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Training")
	void UpdateTrainingState(ETrainingState NewState);
};
