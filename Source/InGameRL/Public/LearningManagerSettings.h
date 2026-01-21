// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LearningManagerSettings.generated.h"

UCLASS()
class INGAMERL_API ALearningManagerSettings : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALearningManagerSettings();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FLearningAgentsPolicySettings GetPolicySettings() const { return PolicySettings; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsCriticSettings GetCriticSettings() const { return CriticSettings; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsTrainerProcessSettings GetTrainerProcessSettings() const { return TrainerProcessSettings; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsSharedMemoryCommunicatorSettings GetSharedMemorySettings() const { return SharedMemorySettings; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsPPOTrainerSettings GetPPOTrainerSettings() const { return PPOTrainerSettings; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsPPOTrainingSettings GetPPOTrainingSettings1() const { return PPOTrainingSettings1; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsPPOTrainingSettings GetPPOTrainingSettings2() const { return PPOTrainingSettings2; }
	UFUNCTION(BlueprintCallable)
	FLearningAgentsTrainingGameSettings GetTrainingGameSettings() const { return TrainingGameSettings; }
	
	UFUNCTION(BlueprintCallable)
	FString GetEncoderPath() const { return EncoderPath; }
	UFUNCTION(BlueprintCallable)
	FString GetDecoderPath() const { return DecoderPath; }
	UFUNCTION(BlueprintCallable)
	FString GetPolicyPath() const { return PolicyPath; }
	UFUNCTION(BlueprintCallable)
	FString GetCriticPath() const { return CriticPath; }

public:
	FLearningAgentsPolicySettings PolicySettings;
	FLearningAgentsCriticSettings CriticSettings;
	FLearningAgentsTrainerProcessSettings TrainerProcessSettings;
	FLearningAgentsSharedMemoryCommunicatorSettings SharedMemorySettings;
	FLearningAgentsPPOTrainerSettings PPOTrainerSettings;
	FLearningAgentsPPOTrainingSettings PPOTrainingSettings1;
	FLearningAgentsPPOTrainingSettings PPOTrainingSettings2;
	FLearningAgentsTrainingGameSettings TrainingGameSettings;

	FString EncoderPath;
	FString DecoderPath;
	FString PolicyPath;
	FString CriticPath;
};
