// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class INGAMERL_API MyLogListener : public FOutputDevice
{
public:
	MyLogListener();
	~MyLogListener();

    virtual void Serialize(
        const TCHAR* V,
        ELogVerbosity::Type Verbosity,
        const FName& Category
    ) override;

    DECLARE_DELEGATE_OneParam(FOnSnapshotPathFound, const FString&);
    FOnSnapshotPathFound OnSnapshotPathFound;
};
