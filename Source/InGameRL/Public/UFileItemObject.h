// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UFileItemObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class INGAMERL_API UUFileItemObject : public UObject
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite) FString Path;
    UPROPERTY(BlueprintReadWrite) FString Name;
    UPROPERTY(BlueprintReadWrite) bool bIsDirectory;
    UPROPERTY(BlueprintReadWrite) FDateTime ModifiedTime;
    UPROPERTY(BlueprintReadWrite) TArray<FString> GroupFiles;
    UPROPERTY(BlueprintReadWrite) bool bIsGroupFile = false;
};
