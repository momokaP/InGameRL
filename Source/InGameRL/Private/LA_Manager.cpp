// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_Manager.h"

#include "Kismet/GameplayStatics.h"

#include "LearningAgentsInteractor.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsNeuralNetwork.h"
#include "LearningNeuralNetwork.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsTrainingEnvironment.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"

#include "RLCharacter.h"
#include "LA_Interactor.h"
#include "LA_Environment.h"

// Sets default values
ALA_Manager::ALA_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	LearningAgentsManager = CreateDefaultSubobject<ULearningAgentsManager>(TEXT("LearningAgentsManager"));
}

// Called when the game starts or when spawned
void ALA_Manager::BeginPlay()
{
	Super::BeginPlay();
	
	if (PolicyNN) {
		UE_LOG(LogTemp, Log, TEXT("PolicyNN is valid: %s"), *PolicyNN->GetName());
	}
	else { UE_LOG(LogTemp, Warning, TEXT("PolicyNN is null")); }
	if (CriticNN) {
		UE_LOG(LogTemp, Log, TEXT("CriticNN is valid: %s"), *CriticNN->GetName());
	}
	else { UE_LOG(LogTemp, Warning, TEXT("CriticNN is null")); }
	if (EncoderNN) {
		UE_LOG(LogTemp, Log, TEXT("EncoderNN is valid: %s"), *EncoderNN->GetName());
	}
	else { UE_LOG(LogTemp, Warning, TEXT("EncoderNN is null")); }
	if (DecoderNN) {
		UE_LOG(LogTemp, Log, TEXT("DecoderNN is valid: %s"), *DecoderNN->GetName());
	}
	else { UE_LOG(LogTemp, Warning, TEXT("DecoderNN is null")); }


	// Set ActorCharacters
	ActorCharacters.Empty();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), ARLCharacter::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		ARLCharacter* Character = Cast<ARLCharacter>(Actor);
		if (Character)
		{
			Character->AddTickPrerequisiteActor(this);
			ActorCharacters.Add(Character);
		}
	}

	// Make Interactor
	Interactor = ULearningAgentsInteractor::MakeInteractor(
		LearningAgentsManager, ULA_Interactor::StaticClass());
	if (!Interactor)
	{
		UE_LOG(LogTemp, Error, TEXT("Interactor is nullptr."));
		return;
	}

	// Make Policy
	Policy = ULearningAgentsPolicy::MakePolicy(
		LearningAgentsManager,
		Interactor,
		ULearningAgentsPolicy::StaticClass(),
		TEXT("Policy"),
		EncoderNN,
		PolicyNN,
		DecoderNN,
		Reinitialize,
		Reinitialize,
		Reinitialize,
		PolicySettings
	);
	if (RunInference)
	{
		Policy->GetEncoderNetworkAsset()->LoadNetworkFromSnapshot(EncoderSnapshot);
		Policy->GetPolicyNetworkAsset()->LoadNetworkFromSnapshot(PolicySnapshot);
		Policy->GetDecoderNetworkAsset()->LoadNetworkFromSnapshot(DecoderSnapshot);
	}
	if (!Policy)
	{
		UE_LOG(LogTemp, Error, TEXT("Policy is nullptr."));
		return;
	}
	if (!Policy->GetPolicyNetworkAsset()->NeuralNetworkData)
	{
		UE_LOG(LogTemp, Error, TEXT("NeuralNetworkData가 비정상입니다."));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("  InputSize = %d"), Policy->GetPolicyNetworkAsset()->NeuralNetworkData->GetInputSize());
		UE_LOG(LogTemp, Log, TEXT("  OutputSize = %d"), Policy->GetPolicyNetworkAsset()->NeuralNetworkData->GetOutputSize());
		UE_LOG(LogTemp, Log, TEXT("  SnapshotByteNum = %d"), Policy->GetPolicyNetworkAsset()->NeuralNetworkData->GetSnapshotByteNum());
	}


	// Make Critic
	Critic = ULearningAgentsCritic::MakeCritic(
		LearningAgentsManager,
		Interactor,
		Policy,
		ULearningAgentsCritic::StaticClass(),
		TEXT("Critic"),
		CriticNN,
		Reinitialize,
		CriticSettings
	);
	if (!Critic)
	{
		UE_LOG(LogTemp, Error, TEXT("Critic is nullptr."));
		return;
	}

	// Make TrainingEnvironment
	TrainingEnv = ULearningAgentsTrainingEnvironment::MakeTrainingEnvironment(
		LearningAgentsManager, ULA_Environment::StaticClass());
	if (!TrainingEnv)
	{
		UE_LOG(LogTemp, Error, TEXT("TrainingEnv is nullptr."));
		return;
	}

	// Make Communicator

	FLearningAgentsSharedMemoryTrainerProcess TrainerProcess =
		ULearningAgentsCommunicatorLibrary::SpawnSharedMemoryTrainingProcess(
			TrainerProcessSettings, SharedMemorySettings
		);
	Communicator =
		ULearningAgentsCommunicatorLibrary::MakeSharedMemoryCommunicator(
			TrainerProcess, TrainerProcessSettings, SharedMemorySettings
		);

	// Make PPO Trainer
	PPOTrainer = ULearningAgentsPPOTrainer::MakePPOTrainer(
		LearningAgentsManager,
		Interactor,
		TrainingEnv,
		Policy,
		Critic,
		Communicator,
		ULearningAgentsPPOTrainer::StaticClass(),
		TEXT("PPOTrainer"),
		PPOTrainerSettings
	);
	if (!PPOTrainer)
	{
		UE_LOG(LogTemp, Error, TEXT("PPOTrainer is nullptr."));
		return;
	}
}

// Called every frame
void ALA_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Super::Tick(DeltaTime);

	if (RunInference)
	{
		Policy->RunInference();
	}
	else
	{
		PPOTrainer->RunTraining(
			PPOTrainingSettings, TrainingGameSettings, true, true);
	}
}

