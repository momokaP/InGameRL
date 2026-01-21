// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RLCharAIController.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API ARLCharAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    UFUNCTION()
    void MoveToTargetLocation(const FVector& TargetLocation);

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	bool GetIsMoving() const { return IsMoving; }
private:
	bool IsMoving = false;
};
