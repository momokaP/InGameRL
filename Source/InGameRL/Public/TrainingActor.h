// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TrainingListenerRunnable.h"
#include "RLCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainingActor.generated.h"

UCLASS()
class INGAMERL_API ATrainingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrainingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetActorRewardWeight();
	void SetListenerRunnable(TrainingListenerRunnable* InRunnable);
	FString GetActorInfo() const;

private:
	/** Runnable과 큐를 통한 메시지 전달 */
	TrainingListenerRunnable* ListenerRunnable;
	float RewardWeight1;
	float RewardWeight2;
	float RewardWeight3;
	int32 TrainingCount;

	bool bInitDone = false;
};
