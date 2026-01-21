// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingWidget.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API UTrainingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MultiInstance")
	void LaunchNewInstance(
		const FString& MapName,
		float RewardWeight1,
		float RewardWeight2,
		float RewardWeight3,
		int32 TrainingCount,
		const FString& EncoderPath,
		const FString& DecoderPath,
		const FString& PolicyPath,
		const FString& CriticPath
	);

	UFUNCTION(BlueprintCallable)
	void TerminateInstance();

	UFUNCTION(BlueprintCallable)
	void RequestActorInfo();

private:
	void SendExitSignal();
	void CheckConnection();
	void OnExitSignalCompleted(bool bAckReceived);
	void StartPeriodicPing();
	void StopPeriodicPing();

	FProcHandle LaunchedProcessHandle;
	int32 DefaultPort = 35001;
	FTimerHandle PingTimerHandle;
	float PingInterval = 5.0f;
	bool bPingThreadRunning = false;
};
