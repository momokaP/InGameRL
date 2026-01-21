// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TrainingListenerRunnable.h"
#include "TrainingGameInstance.generated.h"

UENUM(BlueprintType)
enum class ETrainingState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Running     UMETA(DisplayName = "Running"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Disconnected UMETA(DisplayName = "Disconnected")
};

USTRUCT(BlueprintType)
struct FNetworkPaths
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Encoder;

    UPROPERTY(BlueprintReadWrite)
    FString Decoder;

    UPROPERTY(BlueprintReadWrite)
    FString Policy;

    UPROPERTY(BlueprintReadWrite)
    FString Critic;

    void Reset()
    {
        Encoder.Empty();
        Decoder.Empty();
        Policy.Empty();
        Critic.Empty();
    }

    bool IsComplete() const
    {
        return !Encoder.IsEmpty() && !Decoder.IsEmpty() && !Policy.IsEmpty() && !Critic.IsEmpty();
    }
};

class UTrainingStatusWidget;

/**
 * 
 */
UCLASS()
class INGAMERL_API UTrainingGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    void TestExitCommand();
    void ReadProcessParam();

    UFUNCTION(BlueprintCallable)
    void SaveGameData(const FString& SlotName, int32 StageValue);
    UFUNCTION(BlueprintCallable)
    void LoadGameData(const FString& SlotName);
    UFUNCTION(BlueprintCallable)
    TArray<FString> GetAllSaveSlots() const;

    TrainingListenerRunnable* GetListenerRunnable() const { return ListenerRunnable.Get(); }
    float GetRewardWeight1() const { return RewardWeight1; }
    float GetRewardWeight2() const { return RewardWeight2; }
    float GetRewardWeight3() const { return RewardWeight3; }
    int32 GetTrainingCount() const { return TrainingCount; }
    FString GetEncoderPath() const { return EncoderPath; }
    FString GetDecoderPath() const { return DecoderPath; }
    FString GetPolicyPath() const { return PolicyPath; }
    FString GetCriticPath() const { return CriticPath; }

    void HandleNoPong();
    void StartPingThread();
    void StopPingThread();
    bool IsPingThreadRunning() const { return bPingThreadRunning; }
    UFUNCTION(BlueprintCallable)
    void SetTrainingDone(bool Done) { bTrainingDone = Done; }
    UFUNCTION(BlueprintCallable)
    bool IsTrainingDone() const { return bTrainingDone; }

    UFUNCTION(BlueprintCallable)
    void SetCurrentStage(int32 Stage) { CurrentStage = Stage; }
    UFUNCTION(BlueprintCallable)
    int32 GetCurrentStage() const { return CurrentStage; }

    UFUNCTION(BlueprintCallable)
    void SetCurrentSelectedStage(int32 Stage) { CurrentSelectedStage = Stage; }
    UFUNCTION(BlueprintCallable)
    int32 GetCurrentSelectedStage() const { return CurrentSelectedStage; }

    UFUNCTION(BlueprintCallable)
    int32 GetTrainingCountResource() const { return TrainingCountResource; }
    UFUNCTION(BlueprintCallable)
    void AddTrainingCountResource(int Add) { TrainingCountResource += Add; }


    // Current training state (readable from blueprints)
    UPROPERTY(BlueprintReadOnly, Category = "Training")
    ETrainingState TrainingState = ETrainingState::Idle;

    // Show the persistent training status UI (creates if missing)
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ShowTrainingStatusUI();

    // Set training state (updates UI safely)
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SetTrainingState(ETrainingState NewState);

    // Optionally hide/remove the widget
    UFUNCTION(BlueprintCallable, Category = "Training")
    void HideTrainingStatusUI();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UTrainingStatusWidget> TrainingStatusWidgetClass;

    UPROPERTY(Transient)
    UTrainingStatusWidget* TrainingStatusWidget = nullptr;

    UFUNCTION(BlueprintCallable)
    void LaunchNewInstance(
        const FString& MapName,
        float RewardWeight11,
        float RewardWeight22,
        float RewardWeight33,
        int32 TrainingCount1,
        const FString& EncoderPath,
        const FString& DecoderPath,
        const FString& PolicyPath,
        const FString& CriticPath
    );

    UFUNCTION(BlueprintCallable)
    void TerminateInstance();

    UFUNCTION(BlueprintCallable)
    void RequestActorInfo();

    UFUNCTION(BlueprintCallable)
    void OverwriteSaveGame();

    UFUNCTION()
    void HandleSnapshotSaved(const FString& FilePath);

    UPROPERTY(BlueprintReadWrite)
    FString CurrentSaveSlot = TEXT("Slot");

    // FileBrowserpath
    UFUNCTION(BlueprintCallable)
    const FString& GetFileBrowserpath() const { return FileBrowserpath; }
    UFUNCTION(BlueprintCallable)
    void SetFileBrowserpath(const FString& NewValue) { FileBrowserpath = NewValue; }

    // DefaultEncoderpath
    UFUNCTION(BlueprintCallable)
    const FString& GetDefaultEncoderpath() const { return DefaultEncoderpath; }
    UFUNCTION(BlueprintCallable)
    void SetDefaultEncoderpath(const FString& NewValue) { DefaultEncoderpath = NewValue; }

    // DefaultPolicypath
    UFUNCTION(BlueprintCallable)
    const FString& GetDefaultPolicypath() const { return DefaultPolicypath; }
    UFUNCTION(BlueprintCallable)
    void SetDefaultPolicypath(const FString& NewValue) { DefaultPolicypath = NewValue; }

    // DefaultDecoderpath
    UFUNCTION(BlueprintCallable)
    const FString& GetDefaultDecoderpath() const { return DefaultDecoderpath; }
    UFUNCTION(BlueprintCallable)
    void SetDefaultDecoderpath(const FString& NewValue) { DefaultDecoderpath = NewValue; }

    // Encoderpath1
    UFUNCTION(BlueprintCallable)
    const FString& GetEncoderpath1() const { return Encoderpath1; }
    UFUNCTION(BlueprintCallable)
    void SetEncoderpath1(const FString& NewValue) { Encoderpath1 = NewValue; }

    // Policypath1
    UFUNCTION(BlueprintCallable)
    const FString& GetPolicypath1() const { return Policypath1; }
    UFUNCTION(BlueprintCallable)
    void SetPolicypath1(const FString& NewValue) { Policypath1 = NewValue; }

    // Decoderpath1
    UFUNCTION(BlueprintCallable)
    const FString& GetDecoderpath1() const { return Decoderpath1; }
    UFUNCTION(BlueprintCallable)
    void SetDecoderpath1(const FString& NewValue) { Decoderpath1 = NewValue; }

    // Encoderpath2
    UFUNCTION(BlueprintCallable)
    const FString& GetEncoderpath2() const { return Encoderpath2; }
    UFUNCTION(BlueprintCallable)
    void SetEncoderpath2(const FString& NewValue) { Encoderpath2 = NewValue; }

    // Policypath2
    UFUNCTION(BlueprintCallable)
    const FString& GetPolicypath2() const { return Policypath2; }
    UFUNCTION(BlueprintCallable)
    void SetPolicypath2(const FString& NewValue) { Policypath2 = NewValue; }

    // Decoderpath2
    UFUNCTION(BlueprintCallable)
    const FString& GetDecoderpath2() const { return Decoderpath2; }
    UFUNCTION(BlueprintCallable)
    void SetDecoderpath2(const FString& NewValue) { Decoderpath2 = NewValue; }

    // Encoderpath3
    UFUNCTION(BlueprintCallable)
    const FString& GetEncoderpath3() const { return Encoderpath3; }
    UFUNCTION(BlueprintCallable)
    void SetEncoderpath3(const FString& NewValue) { Encoderpath3 = NewValue; }

    // Policypath3
    UFUNCTION(BlueprintCallable)
    const FString& GetPolicypath3() const { return Policypath3; }
    UFUNCTION(BlueprintCallable)
    void SetPolicypath3(const FString& NewValue) { Policypath3 = NewValue; }

    // Decoderpath3
    UFUNCTION(BlueprintCallable)
    const FString& GetDecoderpath3() const { return Decoderpath3; }
    UFUNCTION(BlueprintCallable)
    void SetDecoderpath3(const FString& NewValue) { Decoderpath3 = NewValue; }

    // EngineRelativepath
    UFUNCTION(BlueprintCallable)
    const FString& GetEngineRelativepath() const { return EngineRelativepath; }
    UFUNCTION(BlueprintCallable)
    void SetEngineRelativepath(const FString& NewValue) { EngineRelativepath = NewValue; }

    // IntermediateRelativepath
    UFUNCTION(BlueprintCallable)
    const FString& GetIntermediateRelativepath() const { return IntermediateRelativepath; }
    UFUNCTION(BlueprintCallable)
    void SetIntermediateRelativepath(const FString& NewValue) { IntermediateRelativepath = NewValue; }

    UFUNCTION(BlueprintCallable)
    const FString& GetProcesspath() const { return Processpath; }
    UFUNCTION(BlueprintCallable)
    void SetProcesspath(const FString& NewValue) { Processpath = NewValue; }

    UFUNCTION(BlueprintCallable)
    const FLearningAgentsTrainerProcessSettings& GetTrainerProcessSettings() const
    {
        return TrainerProcessSettings;
    }

    void LoadPathsFromJSON();

private:
    void SendExitSignal();
    void OnExitSignalCompleted(bool bAckReceived);
    void PingThreadLoop();
    void CheckConnection();

    FProcHandle LaunchedProcessHandle;
    int32 DefaultPort = 35001;
    float PingInterval = 5.0f;
    
    TUniquePtr<TrainingListenerRunnable> ListenerRunnable;
    int32 Port;
    float RewardWeight1;
    float RewardWeight2;
    float RewardWeight3;
    int32 TrainingCount;
    int32 Train;

    FThreadSafeBool bPingThreadRunning = false;
    
    bool bTrainingDone = false;

    int32 CurrentStage = 1;
    int32 CurrentSelectedStage = 1;

    int32 TrainingCountResource = 1000;

    bool bIsInPingLoop = false;

    FString EncoderPath;
    FString DecoderPath;
    FString PolicyPath;
    FString CriticPath;

    UPROPERTY(BlueprintReadWrite, Category = "Training", meta = (AllowPrivateAccess = "true"))
    TArray<FNetworkPaths> SavedNetworks;

    UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    FNetworkPaths EnemyNetworkSet1;

    UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    FNetworkPaths EnemyNetworkSet2;

    UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    FNetworkPaths EnemyNetworkSet3;

    UPROPERTY()
    FNetworkPaths CurrentNetworkSet;

    FString FileBrowserpath;
    FString DefaultEncoderpath;
    FString DefaultPolicypath;
    FString DefaultDecoderpath;

    FString Encoderpath1;
    FString Policypath1;
    FString Decoderpath1;

    FString Encoderpath2;
    FString Policypath2;
    FString Decoderpath2;

    FString Encoderpath3;
    FString Policypath3;
    FString Decoderpath3;

    FString EngineRelativepath;
    FString IntermediateRelativepath;

    FString Processpath;

    FLearningAgentsTrainerProcessSettings TrainerProcessSettings;
};
