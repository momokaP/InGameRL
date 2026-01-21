// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RLCharacter.h"

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "LA_Interactor.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API ULA_Interactor : public ULearningAgentsInteractor
{
	GENERATED_BODY()
public:
	virtual void SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema) override;

	virtual void GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId) override;

	virtual void SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema) override;

	virtual void PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId) override;
private:
	FLearningAgentsActionSchemaElement MakeStructAction3Location3Rotation(
		ULearningAgentsActionSchema* InActionSchema
	);

	void ApplyDiscreteActionMove(
		const ULearningAgentsActionObject* InActionObject,
		const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
		const FName& KeyName,
		ARLCharacter* ActCharacter,
		const FVector& MoveAxis,
		float Amount,
		void (ARLCharacter::* MoveFunc)(FVector)
	);

	void ApplyDiscreteActionRotate(
		const ULearningAgentsActionObject* InActionObject,
		const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
		const FName& KeyName,
		ARLCharacter* ActCharacter,
		const FRotator& RotateAxis,
		float Amount,
		USceneComponent* (ARLCharacter::* GetPointFunc)() const
	);

	float LocationAmount = 15;
	float RotationAmount = 15;

	int MaxEnemyArrayNum = 64;
	int DiscreteActionSize = 3;
};
