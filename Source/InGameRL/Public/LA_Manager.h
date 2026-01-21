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
#include "LA_Manager.generated.h"

class ARLCharacter;
class ULearningAgentsInteractor;
// class ULearningAgentsPolicy;
// class ULearningAgentsCritic;
class ULearningAgentsTrainingEnvironment;
class ULearningAgentsNeuralNetwork;

UCLASS()
class INGAMERL_API ALA_Manager : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULearningAgentsManager* LearningAgentsManager;

public:	
	// Sets default values for this actor's properties
	ALA_Manager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	TArray<ARLCharacter*> ActorCharacters;

	bool RunInference = false;
	bool Reinitialize = true;

	// Interactor
	ULearningAgentsInteractor* Interactor;

	// Policy
	ULearningAgentsPolicy* Policy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Snapshot")
	FFilePath EncoderSnapshot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Snapshot")
	FFilePath PolicySnapshot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Snapshot")
	FFilePath DecoderSnapshot;

	// UPROPERTY(EditAnywhere, Category = "NeuralNetwork")
	// FString EncoderNNPath = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "NeuralNetwork")
	ULearningAgentsNeuralNetwork* EncoderNN;
	// = LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *EncoderNNPath);

	// UPROPERTY(EditAnywhere, Category = "NeuralNetwork")
	// FString PolicyNNPath = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "NeuralNetwork")
	ULearningAgentsNeuralNetwork* PolicyNN;
	// = LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *PolicyNNPath);

	// UPROPERTY(EditAnywhere, Category = "NeuralNetwork")
	// FString DecoderNNPath = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "NeuralNetwork")
	ULearningAgentsNeuralNetwork* DecoderNN;
	// = LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *DecoderNNPath);

	FLearningAgentsPolicySettings PolicySettings;

	// Critic
	ULearningAgentsCritic* Critic;
	// UPROPERTY(EditAnywhere, Category = "NeuralNetwork")
	// FString CriticNNPath = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "NeuralNetwork")
	ULearningAgentsNeuralNetwork* CriticNN;
	// = LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *CriticNNPath);
	FLearningAgentsCriticSettings CriticSettings;

	// TrainingEnvironment
	ULearningAgentsTrainingEnvironment* TrainingEnv;

	// Communicator
	FLearningAgentsCommunicator Communicator;
	FLearningAgentsTrainerProcessSettings TrainerProcessSettings;
	FLearningAgentsSharedMemoryCommunicatorSettings SharedMemorySettings;

	// PPO Trainer
	ULearningAgentsPPOTrainer* PPOTrainer;
	FLearningAgentsPPOTrainerSettings PPOTrainerSettings;
	FLearningAgentsPPOTrainingSettings PPOTrainingSettings;
	FLearningAgentsTrainingGameSettings TrainingGameSettings;

};
