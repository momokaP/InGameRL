// Fill out your copyright notice in the Description page of Project Settings.


#include "RLCharAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "RLCharacter.h"

void ARLCharAIController::MoveToTargetLocation(const FVector& TargetLocation)
{
    // 언리얼의 기본 내비게이션을 활용한 이동 명령
    MoveToLocation(TargetLocation);
    IsMoving = true;
    UE_LOG(LogTemp, Warning, TEXT("[AI] Moving to: %s"), *TargetLocation.ToString());
}

void ARLCharAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    IsMoving = false;
    UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted"));
}