// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RLCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BattleController.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API ABattleController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	void NotifyCharacterDeath(ARLCharacter* DeadCharacter);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* BattleMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LeftClickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RightClickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveForwardAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveRightAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveUpAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RotateYawAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RotatePitchAction;

protected:
	UFUNCTION()
	void OnLeftClickTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnRightClickTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnLeftClickStarted(const FInputActionValue& Value);
	UFUNCTION()
	void OnLeftClickReleased(const FInputActionValue& Value);

	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveUp(const FInputActionValue& Value);
	void RotateYaw(const FInputActionValue& Value);
	void RotatePitch(const FInputActionValue& Value);

	void SelectUnit(ARLCharacter* Unit);
	void DeselectAll();
	bool CanPlaceAtLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable)
	bool IsBattleStarted() const { return bBattleStarted; }
	UFUNCTION(BlueprintCallable)
	void SetBattleStarted(bool Started) { bBattleStarted = Started; }
	UFUNCTION(BlueprintCallable)
	ARLCharacter* GetSelectedUnit() const { return SelectedUnit; }
	UFUNCTION(BlueprintCallable)
	bool GetIsBattleEnd() const { return IsBattleEnd; }
	UFUNCTION(BlueprintCallable)
	bool GetIsWin() const { return IsWin; }

	void SelectUnitsInDragBox();

	void DrawSelectionBoxOnGround();

	UFUNCTION(BlueprintCallable)
	void RemoveDeploymentPlanes();

	void EndBattle(bool bPlayerWin);

private:
	ARLCharacter* SelectedUnit;
	bool bBattleStarted = false;

	UPROPERTY()
	TArray<ARLCharacter*> SelectedUnits;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MoveSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float RotateSpeed = 60.0f; // yaw/pitch 회전 속도

	UPROPERTY(EditAnywhere, Category = "Camera")
	float VerticalSpeed = 600.0f; // z축 이동 속도

	FVector2D MoveInput = FVector2D::ZeroVector;
	float YawInput = 0.0f;
	float PitchInput = 0.0f;
	float VerticalInput = 0.0f;

public:
	bool bIsDragging = false;
	FVector2D DragStartScreenPos;
	FVector2D DragEndScreenPos;

	bool IsBattleEnd = false;
	bool IsWin = false;
};
