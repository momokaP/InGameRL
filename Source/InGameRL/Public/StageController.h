// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RLCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StageController.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API AStageController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	void SetStage();
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* BattleMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LeftClickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RightClickAction;

	UPROPERTY(EditAnywhere, Category = "Stage")
	UMaterialInterface* StageMaterial_clear;

	UPROPERTY(EditAnywhere, Category = "Stage")
	UMaterialInterface* StageMaterial_unclear;

	UPROPERTY(EditAnywhere, Category = "Stage")
	UMaterialInterface* StageMaterial_selected;

protected:
	UFUNCTION()
	void OnLeftClickTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnRightClickTriggered(const FInputActionValue& Value);

private:
	ARLCharacter* SelectedUnit;
};
