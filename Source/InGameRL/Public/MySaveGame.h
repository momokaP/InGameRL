// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class INGAMERL_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    int32 Stage;

    UPROPERTY(BlueprintReadWrite)
    int32 TrainingCountResource;

    UPROPERTY(BlueprintReadWrite)
    FString EncoderPath;
    UPROPERTY(BlueprintReadWrite)
    FString DecoderPath;
    UPROPERTY(BlueprintReadWrite)
    FString PolicyPath;
    UPROPERTY(BlueprintReadWrite)
    FString CriticPath;
};
