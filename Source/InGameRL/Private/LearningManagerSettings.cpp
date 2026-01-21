// Fill out your copyright notice in the Description page of Project Settings.

#include "LearningManagerSettings.h"
#include "TrainingGameInstance.h"

// Sets default values
ALearningManagerSettings::ALearningManagerSettings()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALearningManagerSettings::BeginPlay()
{
	Super::BeginPlay();

	PPOTrainingSettings1.bSaveSnapshots = true;
	PPOTrainingSettings2.bSaveSnapshots = true;

	PPOTrainingSettings1.bUseTensorboard = true;
	PPOTrainingSettings2.bUseTensorboard = true;

	PPOTrainingSettings1.RandomSeed = 1100;
	PPOTrainingSettings2.RandomSeed = 2200;

	PPOTrainingSettings1.NumberOfIterations = 101;
	PPOTrainingSettings2.NumberOfIterations = 101;

	if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
	{
		PPOTrainingSettings1.NumberOfIterations = GI->GetTrainingCount();
		PPOTrainingSettings2.NumberOfIterations = GI->GetTrainingCount();

		EncoderPath = GI->GetEncoderPath();
		DecoderPath = GI->GetDecoderPath();
		PolicyPath = GI->GetPolicyPath();
		CriticPath = GI->GetCriticPath();
	}

	UE_LOG(LogTemp, Warning, TEXT("NumberOfIterations %d, %d"), 
		PPOTrainingSettings1.NumberOfIterations, PPOTrainingSettings2.NumberOfIterations);
}

// Called every frame
void ALearningManagerSettings::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

