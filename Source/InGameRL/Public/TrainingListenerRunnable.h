// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "HAL/ThreadSafeBool.h"

struct FNetworkPaths_R
{
    FString Encoder;

    FString Decoder;

    FString Policy;

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

/**
 * TCP Listener Thread for Training Communication
 */
class INGAMERL_API TrainingListenerRunnable : public FRunnable
{
public:
    TQueue<FString, EQueueMode::Mpsc> IncomingQueue; // Runnable → Actor
    TQueue<FString, EQueueMode::Mpsc> OutgoingQueue; // Actor → Runnable

    TrainingListenerRunnable(int32 InPort);
    virtual ~TrainingListenerRunnable();

    bool Start();

    virtual uint32 Run() override;
    virtual void Stop() override;

    void SendStringToClient(const FString& Msg);

private:
    void HandleExitRequest();
    void CleanupSockets();

private:
    int32 Port;
    FThreadSafeBool bStopRequested;
    FRunnableThread* Thread;
    class FSocket* ListenSocket;
    class FSocket* ClientSocket;

    UPROPERTY()
    FNetworkPaths_R CurrentNetworkSet;

    TArray<FString> PendingParts;
};