// Fill out your copyright notice in the Description page of Project Settings.


#include "TrainingListenerRunnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

TrainingListenerRunnable::TrainingListenerRunnable(int32 InPort)
    : Port(InPort),
    bStopRequested(false),
    Thread(nullptr),
    ListenSocket(nullptr),
    ClientSocket(nullptr)
{
}

TrainingListenerRunnable::~TrainingListenerRunnable()
{
    Stop();
    if (Thread)
    {
        Thread->WaitForCompletion();
        delete Thread;
        Thread = nullptr;
    }
}

bool TrainingListenerRunnable::Start()
{
    if (!Thread)
    {
        Thread = FRunnableThread::Create(this, TEXT("TrainingListenerThread"));
        return Thread != nullptr;
    }
    return false;
}

uint32 TrainingListenerRunnable::Run()
{
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TrainingListener"), false);
    ListenSocket->SetNonBlocking(true);

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    Addr->SetAnyAddress();
    Addr->SetPort(Port);

    if (!ListenSocket->Bind(*Addr))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind TCP socket on port %d"), Port);
        return 0;
    }

    ListenSocket->Listen(1);
    UE_LOG(LogTemp, Log, TEXT("TrainingListener listening on port %d"), Port);

    double LastPingTime = FPlatformTime::Seconds();
    const double PingTimeout = 10.0;
    PendingParts.Empty();

    while (!bStopRequested)
    {
        bool bPending = false;
        if (ListenSocket->HasPendingConnection(bPending) && bPending)
        {
            ClientSocket = ListenSocket->Accept(TEXT("ClientSocket"));
            if (ClientSocket)
            {
                ClientSocket->SetNonBlocking(true);
                UE_LOG(LogTemp, Log, TEXT("Client connected on port %d"), Port);
            }
        }

        if (ClientSocket)
        {
            uint8 Data[4096];
            int32 BytesRead = 0;
            if (ClientSocket->Recv(Data, sizeof(Data), BytesRead) && BytesRead > 0)
            {
                FString Msg = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(Data))).TrimStartAndEnd();
                UE_LOG(LogTemp, Log, TEXT("Received: %s"), *Msg);

                if (Msg.Contains(TEXT("EXIT")))
                {
                    const FString AckMsg = TEXT("ACK_EXIT");
                    SendStringToClient(AckMsg);

                    UE_LOG(LogTemp, Warning, TEXT("Received EXIT signal. Requesting shutdown."));
                    HandleExitRequest();
                    break;
                }
                else if (Msg.Contains(TEXT("PING")))
                {
                    LastPingTime = FPlatformTime::Seconds();
                    FString TempMsg = FString::Printf(TEXT("PONG"));
                    if (CurrentNetworkSet.IsComplete()) {
                        PendingParts.Add(CurrentNetworkSet.Encoder);
                        PendingParts.Add(CurrentNetworkSet.Decoder);
                        PendingParts.Add(CurrentNetworkSet.Policy);
                        PendingParts.Add(CurrentNetworkSet.Critic);
                        CurrentNetworkSet.Reset();    
                    }               
                    if (PendingParts.Num() > 0) {
                        FString Part = PendingParts[0];   // 앞에서 하나
                        PendingParts.RemoveAt(0);
                        TempMsg = FString::Printf(TEXT("PONG | %s"), *Part);
                    }
                    SendStringToClient(TempMsg);
                    UE_LOG(LogTemp, Log, TEXT("Responded to PING with PONG"));
                    
                }
                else if (Msg.Contains(TEXT("GET_ACTOR_INFO")))
                {
                    IncomingQueue.Enqueue(Msg);
                    UE_LOG(LogTemp, Log, TEXT("Enqueued GET_ACTOR_INFO command for actor processing"));
                }
                else if (Msg.StartsWith(TEXT("SNAPSHOT:")))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Received SNAPSHOT message: %s"), *Msg);
                }
                else
                {
                    const FString Echo = FString::Printf(TEXT("ECHO: %s"), *Msg);
                    SendStringToClient(Echo);
                }
            }

            double Now = FPlatformTime::Seconds();
            if (Now - LastPingTime > PingTimeout)
            {
                UE_LOG(LogTemp, Error, TEXT("Ping timeout: No PING from parent for %.1f seconds. Exiting."), PingTimeout);
                HandleExitRequest();
                break;
            }

            FString OutMsg;
            while (OutgoingQueue.Dequeue(OutMsg))
            {
                if (OutMsg.Contains("SNAPSHOT")) {
                    if (OutMsg.Contains("encoder"))
                    {
                        CurrentNetworkSet.Encoder = OutMsg;
                    }
                    else if (OutMsg.Contains("decoder"))
                    {
                        CurrentNetworkSet.Decoder = OutMsg;
                    }
                    else if (OutMsg.Contains("policy"))
                    {
                        CurrentNetworkSet.Policy = OutMsg;
                    }
                    else if (OutMsg.Contains("critic"))
                    {
                        CurrentNetworkSet.Critic = OutMsg;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Snapshot file does not match any known type: %s"), *OutMsg);
                    }
                }
                SendStringToClient(OutMsg);
            }
        }

        FPlatformProcess::Sleep(0.01f);
    }

    CleanupSockets();
    return 0;
}

void TrainingListenerRunnable::Stop()
{
    bStopRequested = true;
}

void TrainingListenerRunnable::SendStringToClient(const FString& Msg)
{
    if (!ClientSocket)
        return;

    FTCHARToUTF8 Converter(*Msg);
    int32 BytesSent = 0;
    ClientSocket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);
    UE_LOG(LogTemp, Log, TEXT("Sent: %s (%d bytes)"), *Msg, BytesSent);
}

void TrainingListenerRunnable::HandleExitRequest()
{
    static bool bShutdownRequested = false;
    if (bShutdownRequested)
        return;

    bShutdownRequested = true;

    AsyncTask(ENamedThreads::GameThread, []()
        {
#if WITH_EDITOR
            if (GIsEditor && GEditor)
            {
                UE_LOG(LogTemp, Warning, TEXT("Stopping PIE session safely"));
                GEditor->RequestEndPlayMap();
            }
            else
#endif
            {
                UE_LOG(LogTemp, Warning, TEXT("Requesting application exit safely"));
                FGenericPlatformMisc::RequestExit(false);
            }
        });
}

void TrainingListenerRunnable::CleanupSockets()
{
    if (ClientSocket)
    {
        ClientSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
        ClientSocket = nullptr;
    }
    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
}


