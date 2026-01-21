// Fill out your copyright notice in the Description page of Project Settings.


#include "TrainingWidget.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "TrainingGameInstance.h"

void UTrainingWidget::LaunchNewInstance(
    const FString& MapName,
    float RewardWeight1,
    float RewardWeight2,
    float RewardWeight3,
    int32 TrainingCount,
    const FString& EncoderPath,
    const FString& DecoderPath,
    const FString& PolicyPath,
    const FString& CriticPath
) {
    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    {
        GI->LaunchNewInstance(
            MapName,
            RewardWeight1,
            RewardWeight2,
            RewardWeight3,
            TrainingCount,
            EncoderPath,
            DecoderPath,
            PolicyPath,
            CriticPath
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("LaunchNewInstance: GameInstance is not UTrainingGameInstance."));
    }

    //if (LaunchedProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(LaunchedProcessHandle))
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Headless instance is already running. Skipping new launch."));
    //    return;
    //}

    ////FString ExePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/MyGame.exe"));
    //FString ExePath = TEXT("C:/Unreal_Projects/InGameRL/Saved/StagedBuilds/Windows/InGameRL/Binaries/Win64/InGameRL.exe");

    //FString Params = FString::Printf(
    //    //TEXT("%s -nullrhi -nosound -Unattended -Port=%d -RewardWeight1=%f -RewardWeight2=%f -RewardWeight3=%f -TrainingCount=%d"),
    //    TEXT("%s -RenderOffScreen -nosound -Unattended -Port=%d -RewardWeight1=%f -RewardWeight2=%f -RewardWeight3=%f -TrainingCount=%d -Train=%d"),
    //    *MapName,
    //    DefaultPort,
    //    RewardWeight1,
    //    RewardWeight2,
    //    RewardWeight3,
    //    TrainingCount,
    //    1
    //);

    //LaunchedProcessHandle = FPlatformProcess::CreateProc(
    //    *ExePath,
    //    *Params,
    //    true,   // bLaunchDetached
    //    false,  // bLaunchHidden
    //    false,  // bLaunchReallyHidden
    //    nullptr,
    //    0,
    //    nullptr,
    //    nullptr
    //);

    //if (LaunchedProcessHandle.IsValid())
    //{
    //    UE_LOG(LogTemp, Log, TEXT("Launched headless instance with map: %s"), *MapName);

    //    Async(EAsyncExecution::Thread, [this]()
    //        {
    //            FPlatformProcess::Sleep(5.0f); // 리스너 대기 (2초)
    //            AsyncTask(ENamedThreads::GameThread, [this]()
    //                {
    //                    UE_LOG(LogTemp, Log, TEXT("Starting periodic ping after delay..."));
    //                    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    //                    {
    //                        GI->SetTrainingState(ETrainingState::Running);
    //                        GI->ShowTrainingStatusUI();

    //                        if (!GI->IsPingThreadRunning())
    //                        {
    //                            GI->StartPingThread();
    //                        }
    //                    }
    //                });
    //        });

    //}
    //else
    //{
    //    UE_LOG(LogTemp, Error, TEXT("Failed to launch instance for map: %s"), *MapName);
    //}
}

void UTrainingWidget::TerminateInstance()
{
    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    {
        GI->TerminateInstance();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TerminateInstance: GameInstance is not UTrainingGameInstance."));
    }

    //if (!IsValid(this))
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("UTrainingWidget is not valid. Skipping TerminateInstance."));
    //    return;
    //}

    //if (!LaunchedProcessHandle.IsValid())
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("No process handle to terminate."));
    //    return;
    //}

    //UE_LOG(LogTemp, Log, TEXT("Requesting graceful shutdown (non-blocking)..."));

    //// EXIT 신호 전송 (비동기)
    //SendExitSignal();
}

void UTrainingWidget::RequestActorInfo()
{
    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    {
        GI->RequestActorInfo();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RequestActorInfo: GameInstance is not UTrainingGameInstance."));
    }

    //Async(EAsyncExecution::Thread, [this]()
    //    {
    //        const FString ServerAddress = TEXT("127.0.0.1");
    //        const int32 Port = DefaultPort;

    //        ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    //        if (!SocketSubsystem) return;

    //        TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    //        bool bIsValid = false;
    //        Addr->SetIp(*ServerAddress, bIsValid);
    //        Addr->SetPort(Port);
    //        if (!bIsValid) return;

    //        FSocket* ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("ActorInfoSocket"), false);
    //        if (!ClientSocket) return;

    //        if (!ClientSocket->Connect(*Addr))
    //        {
    //            ClientSocket->Close();
    //            SocketSubsystem->DestroySocket(ClientSocket);
    //            return;
    //        }

    //        // ▶ GET_ACTOR_INFO 메시지 전송
    //        FString Message = TEXT("GET_ACTOR_INFO");
    //        FTCHARToUTF8 Converter(*Message);
    //        int32 BytesSent = 0;
    //        ClientSocket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

    //        // ▶ 응답 수신
    //        uint8 RecvData[1024] = { 0 };
    //        int32 BytesRead = 0;
    //        double StartTime = FPlatformTime::Seconds();
    //        FString Result;

    //        while (FPlatformTime::Seconds() - StartTime < 3.0)
    //        {
    //            if (ClientSocket->Recv(RecvData, sizeof(RecvData), BytesRead, ESocketReceiveFlags::None) && BytesRead > 0)
    //            {
    //                Result = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData))).TrimStartAndEnd();
    //                break;
    //            }
    //            FPlatformProcess::Sleep(0.05f);
    //        }

    //        ClientSocket->Close();
    //        SocketSubsystem->DestroySocket(ClientSocket);


    //        if (!Result.IsEmpty())
    //        {
    //            UE_LOG(LogTemp, Log, TEXT("Actor Info Received:\n%s"), *Result);
    //        }
    //        else
    //        {
    //            UE_LOG(LogTemp, Warning, TEXT("No actor info received from listener."));
    //        }

    //    });
}

void UTrainingWidget::StartPeriodicPing()
{
    if (bPingThreadRunning)
        return;

    bPingThreadRunning = true;

    Async(EAsyncExecution::Thread, [this]()
        {
            UE_LOG(LogTemp, Log, TEXT("[PingThread] Started periodic PING thread."));

            while (bPingThreadRunning)
            {
                if (IsEngineExitRequested())
                {
                    UE_LOG(LogTemp, Log, TEXT("[PingThread] Game is exiting, stopping thread."));
                    break;
                }

                CheckConnection();
                FPlatformProcess::Sleep(PingInterval); // ex. 5초
            }

            UE_LOG(LogTemp, Log, TEXT("[PingThread] Stopped."));
        });
}

void UTrainingWidget::StopPeriodicPing()
{
    bPingThreadRunning = false;
}

void UTrainingWidget::CheckConnection()
{
    const FString ServerAddress = TEXT("127.0.0.1");
    const int32 Port = DefaultPort;

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SocketSubsystem not found."));
        return;
    }

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp(*ServerAddress, bIsValid);
    Addr->SetPort(Port);

    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address: %s"), *ServerAddress);
        return;
    }

    FSocket* ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("HealthCheckSocket"), false);
    if (!ClientSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket for health check."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Connecting to listener for health check (port %d)..."), Port);

    if (!ClientSocket->Connect(*Addr))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to listener."));
        ClientSocket->Close();
        SocketSubsystem->DestroySocket(ClientSocket);
        return;
    }

    // --- PING 메시지 전송 ---
    FString PingMsg = TEXT("PING");
    FTCHARToUTF8 PingConverter(*PingMsg);
    int32 BytesSent = 0;
    bool bSent = ClientSocket->Send((uint8*)PingConverter.Get(), PingConverter.Length(), BytesSent);

    if (!bSent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send PING."));
        ClientSocket->Close();
        SocketSubsystem->DestroySocket(ClientSocket);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Sent PING (%d bytes). Waiting for PONG..."), BytesSent);

    // --- 응답 수신 대기 ---
    uint8 RecvData[256] = { 0 };
    int32 BytesRead = 0;
    double StartTime = FPlatformTime::Seconds();
    bool bGotPong = false;

    while (FPlatformTime::Seconds() - StartTime < 3.0) // 최대 3초 대기
    {
        if (ClientSocket->Recv(RecvData, sizeof(RecvData), BytesRead, ESocketReceiveFlags::None) && BytesRead > 0)
        {
            FString Reply = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData))).TrimStartAndEnd();
            UE_LOG(LogTemp, Log, TEXT("Received reply: %s"), *Reply);

            if (Reply.Contains(TEXT("PONG")))
            {
                UE_LOG(LogTemp, Display, TEXT("Listener connection OK (PONG received)."));
                bGotPong = true;
                break;
            }
        }
        FPlatformProcess::Sleep(0.05f);
    }

    if (!bGotPong)
    {
        UE_LOG(LogTemp, Error, TEXT("No PONG received. Listener may not be running or port is wrong."));
    }

    // --- 정리 ---
    ClientSocket->Close();
    SocketSubsystem->DestroySocket(ClientSocket);
}

void UTrainingWidget::SendExitSignal()
{
    if (!IsValid(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("UTrainingWidget is not valid. Skipping SendExitSignal."));
        return;
    }

    // --- 별도 스레드에서 실행 (게임 스레드 블로킹 방지) ---
    Async(EAsyncExecution::Thread, [this]()
        {
            const FString ServerAddress = TEXT("127.0.0.1");
            const int32 Port = DefaultPort;

            ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
            if (!SocketSubsystem)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get SocketSubsystem."));
                return;
            }

            TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
            bool bIsValid = false;
            Addr->SetIp(*ServerAddress, bIsValid);
            Addr->SetPort(Port);

            if (!bIsValid)
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid IP address."));
                return;
            }

            FSocket* ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TrainingExitSocket"), false);
            if (!ClientSocket)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create socket."));
                return;
            }

            if (!ClientSocket->Connect(*Addr))
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to connect to headless listener socket."));
                ClientSocket->Close();
                SocketSubsystem->DestroySocket(ClientSocket);
                return;
            }

            // --- EXIT 신호 전송 ---
            FString Message = TEXT("EXIT");
            FTCHARToUTF8 Converter(*Message);
            int32 BytesSent = 0;
            bool bSent = ClientSocket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

            if (bSent)
            {
                UE_LOG(LogTemp, Log, TEXT("[Async] Sent EXIT signal to headless instance."));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[Async] Failed to send EXIT signal."));
            }

            // --- ACK 응답 대기 ---
            uint8 RecvData[256] = { 0 };
            int32 BytesRead = 0;
            bool bAckReceived = false;
            double StartTime = FPlatformTime::Seconds();

            while (FPlatformTime::Seconds() - StartTime < 3.0) // 최대 3초 대기
            {
                if (ClientSocket->Recv(RecvData, sizeof(RecvData), BytesRead, ESocketReceiveFlags::None) && BytesRead > 0)
                {
                    FString Reply = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData))).TrimStartAndEnd();
                    UE_LOG(LogTemp, Log, TEXT("[Async] Received reply: %s"), *Reply);
                    if (Reply.Contains(TEXT("ACK_EXIT")))
                    {
                        UE_LOG(LogTemp, Display, TEXT("Received ACK_EXIT from headless instance."));
                        bAckReceived = true;
                        break;
                    }
                }
                FPlatformProcess::Sleep(0.05f);
            }

            if (!bAckReceived)
            {
                UE_LOG(LogTemp, Warning, TEXT("No ACK_EXIT received within 3 seconds."));
            }

            // --- 소켓 정리 ---
            ClientSocket->Close();
            SocketSubsystem->DestroySocket(ClientSocket);

            // --- 메인 스레드로 종료 요청 전달 ---
            AsyncTask(ENamedThreads::GameThread, [this, bAckReceived]()
                {
                    OnExitSignalCompleted(bAckReceived);
                });
        });
}

void UTrainingWidget::OnExitSignalCompleted(bool bAckReceived)
{
    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    {
        GI->StopPingThread();

        if (bAckReceived)
        {
            GI->SetTrainingState(ETrainingState::Disconnected);
            UE_LOG(LogTemp, Log, TEXT("Headless instance acknowledged shutdown (ACK_EXIT)."));
        }
        else
        {
            GI->SetTrainingState(ETrainingState::Disconnected);
            UE_LOG(LogTemp, Warning, TEXT("No ACK received. Assuming crash or disconnection."));
        }

        GI->ShowTrainingStatusUI();
    }

    if (!LaunchedProcessHandle.IsValid())
        return;

    if (bAckReceived)
    {
        UE_LOG(LogTemp, Log, TEXT("Headless instance acknowledged shutdown (ACK_EXIT). Closing handle."));
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
        LaunchedProcessHandle.Reset();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Forcefully terminating process (no ACK received)."));
        if (FPlatformProcess::IsProcRunning(LaunchedProcessHandle))
        {
            FPlatformProcess::TerminateProc(LaunchedProcessHandle);
        }
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
        LaunchedProcessHandle.Reset();
    }
}