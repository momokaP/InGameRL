// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainingEnvironment.h"
#include "LA_Environment.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API ULA_Environment : public ULearningAgentsTrainingEnvironment
{
	GENERATED_BODY()
public:
	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;

	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId) override;

	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

};
