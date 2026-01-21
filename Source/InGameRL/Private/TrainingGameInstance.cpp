// Fill out your copyright notice in the Description page of Project Settings.

#include "TrainingGameInstance.h"
#include "MyLogListener.h"

#include "GameFramework/GameUserSettings.h"
#include "Scalability.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TrainingStatusWidget.h"
#include "Misc/OutputDevice.h"

static MyLogListener* GMyLogListener = nullptr; // ????

void UTrainingGameInstance::HandleSnapshotSaved(const FString& FilePath)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleSnapshotSaved New Snapshot Captured: %s"), *FilePath);

    if (ListenerRunnable)
    {
        FString Msg = FString::Printf(TEXT("SNAPSHOT:%s"), *FilePath);
        ListenerRunnable->OutgoingQueue.Enqueue(Msg);
    }

    //FString Lower = FilePath.ToLower();

    //// 파일 구분
    //if (Lower.Contains("encoder"))
    //{
    //    CurrentNetworkSet.Encoder = FilePath;
    //}
    //else if (Lower.Contains("decoder"))
    //{
    //    CurrentNetworkSet.Decoder = FilePath;
    //}
    //else if (Lower.Contains("policy"))
    //{
    //    CurrentNetworkSet.Policy = FilePath;
    //}
    //else if (Lower.Contains("critic"))
    //{
    //    CurrentNetworkSet.Critic = FilePath;
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Snapshot file does not match any known type: %s"), *FilePath);
    //    return;
    //}

    //// 네 가지 파일이 모두 모였는지 체크
    //if (CurrentNetworkSet.IsComplete())
    //{
    //    // 추가
    //    SavedNetworks.Add(CurrentNetworkSet);
    //    UE_LOG(LogTemp, Warning, TEXT("Network Set Completed. Total Sets = %d"), SavedNetworks.Num());

    //    // 저장세트가 4개 초과하면 오래된 것 삭제
    //    const int32 MaxSets = 4;
    //    while (SavedNetworks.Num() > MaxSets)
    //    {
    //        SavedNetworks.RemoveAt(0);
    //        UE_LOG(LogTemp, Warning, TEXT("Removed oldest network set. Now count = %d"), SavedNetworks.Num());
    //    }

    //    // 현재 세트 초기화
    //    CurrentNetworkSet = FNetworkPaths();
    //}
}

void UTrainingGameInstance::Init()
{
    Super::Init();

    TrainingState = ETrainingState::Idle;

    ReadProcessParam();

    LoadPathsFromJSON();

    ListenerRunnable = MakeUnique<TrainingListenerRunnable>(Port);
    if (ListenerRunnable->Start())
    {
        UE_LOG(LogTemp, Log, TEXT("Training listener thread started on port %d"), Port);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start training listener thread."));
        ListenerRunnable.Reset();
    }

    if (Train >= 1) {
        UE_LOG(LogTemp, Error, TEXT("Low QualityLevel for Training"));
        Scalability::FQualityLevels QualityLevels;
        QualityLevels.SetFromSingleQualityLevel(0);
        Scalability::SetQualityLevels(QualityLevels);

        UGameUserSettings* Settings = GEngine->GetGameUserSettings();
        Settings->SetScreenResolution(FIntPoint(1280, 720));
        Settings->SetResolutionScaleNormalized(0.75f);
        Settings->ApplySettings(false);

        Settings->SaveSettings();
    }

    // 로그 리스너 생성 및 등록
    GMyLogListener = new MyLogListener();
    GLog->AddOutputDevice(GMyLogListener);

    // Delegate 연결
    GMyLogListener->OnSnapshotPathFound.BindUObject(
        this,
        &UTrainingGameInstance::HandleSnapshotSaved
    );
}

void UTrainingGameInstance::ReadProcessParam()
{
    auto GetFloatParam = [](const TCHAR* Key, float DefaultValue)
        {
            FString ValueStr;
            if (FParse::Value(FCommandLine::Get(), Key, ValueStr))
            {
                return FCString::Atof(*ValueStr);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No %s specified. Using default %.2f"), Key, DefaultValue);
                return DefaultValue;
            }
        };

    auto GetIntParam = [](const TCHAR* Key, int32 DefaultValue)
        {
            FString ValueStr;
            if (FParse::Value(FCommandLine::Get(), Key, ValueStr))
            {
                return FCString::Atoi(*ValueStr);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No %s specified. Using default %d"), Key, DefaultValue);
                return DefaultValue;
            }
        };

    auto GetStringParam = [](const TCHAR* Key, const FString& DefaultValue)
        {
            FString ValueStr;
            if (FParse::Value(FCommandLine::Get(), Key, ValueStr))
            {
                return ValueStr;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No %s specified. Using default %s"), Key, *DefaultValue);
                return DefaultValue;
            }
        };

    Port = GetIntParam(TEXT("Port="), 35000);
    RewardWeight1 = GetFloatParam(TEXT("RewardWeight1="), 0.9f);
    RewardWeight2 = GetFloatParam(TEXT("RewardWeight2="), 0.9f);
    RewardWeight3 = GetFloatParam(TEXT("RewardWeight3="), 0.9f);
    TrainingCount = GetIntParam(TEXT("TrainingCount="), 99999);
    Train = GetIntParam(TEXT("Train="), 0);    

    EncoderPath = GetStringParam(TEXT("EncoderPath="), TEXT(""));
    DecoderPath = GetStringParam(TEXT("DecoderPath="), TEXT(""));
    PolicyPath = GetStringParam(TEXT("PolicyPath="), TEXT(""));
    CriticPath = GetStringParam(TEXT("CriticPath="), TEXT(""));

    UE_LOG(LogTemp, Log, TEXT("Parsed Params: Port=%d, RewardWeights=(%.2f, %.2f, %.2f), TrainingCount=%d, Train=%d"),
        Port, RewardWeight1, RewardWeight2, RewardWeight3, TrainingCount, Train);

    UE_LOG(LogTemp, Log, TEXT("Model Paths:"));
    UE_LOG(LogTemp, Log, TEXT("  Encoder: %s"), *EncoderPath);
    UE_LOG(LogTemp, Log, TEXT("  Decoder: %s"), *DecoderPath);
    UE_LOG(LogTemp, Log, TEXT("  Policy:  %s"), *PolicyPath);
    UE_LOG(LogTemp, Log, TEXT("  Critic:  %s"), *CriticPath);

    //// 커맨드라인에서 Port 값 읽기
    //FString PortStr;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("Port="), PortStr))
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("No Port specified. Using default 35000"));
    //    Port = 35000;
    //}
    //else
    //{
    //    Port = FCString::Atoi(*PortStr);
    //}

    //// 커맨드라인에서 RewardWeight 값 읽기
    //FString RewardStr1;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("RewardWeight1="), RewardStr1))
    //{
    //    RewardWeight1 = 0.9f; // 기본값
    //    UE_LOG(LogTemp, Warning, TEXT("No RewardWeight specified. Using default 1.0"));
    //}
    //else
    //{
    //    RewardWeight1 = FCString::Atof(*RewardStr1);
    //}

    //FString RewardStr2;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("RewardWeight2="), RewardStr2))
    //{
    //    RewardWeight2 = 0.9f; // 기본값
    //    UE_LOG(LogTemp, Warning, TEXT("No RewardWeight specified. Using default 1.0"));
    //}
    //else
    //{
    //    RewardWeight2 = FCString::Atof(*RewardStr2);
    //}

    //FString RewardStr3;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("RewardWeight3="), RewardStr3))
    //{
    //    RewardWeight3 = 0.9f; // 기본값
    //    UE_LOG(LogTemp, Warning, TEXT("No RewardWeight specified. Using default 1.0"));
    //}
    //else
    //{
    //    RewardWeight3 = FCString::Atof(*RewardStr3);
    //}

    //// 커맨드라인에서 TrainingCount 값 읽기
    //FString CountStr;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("TrainingCount="), CountStr))
    //{
    //    TrainingCount = 99999; // 기본값은 99
    //    UE_LOG(LogTemp, Warning, TEXT("No TrainingCount specified. Using default 1000"));
    //}
    //else
    //{
    //    TrainingCount = FCString::Atoi(*CountStr);
    //}

    //// 커맨드라인에서 Train 값 읽기
    //FString TrainStr;
    //if (!FParse::Value(FCommandLine::Get(), TEXT("Train="), TrainStr))
    //{
    //    Train = 0; // 기본값
    //    UE_LOG(LogTemp, Warning, TEXT("No Train specified. Using default 0"));
    //}
    //else
    //{
    //    Train = FCString::Atoi(*TrainStr);
    //}

    //UE_LOG(LogTemp, Log, TEXT("Parsed Params: Port=%d, RewardWeight=%.2f, TrainingCount=%d"),
    //    Port, RewardWeight1, TrainingCount);
}

void UTrainingGameInstance::Shutdown()
{
    Super::Shutdown();

    StopPingThread();

    if (LaunchedProcessHandle.IsValid())
    {
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
    }

    if (ListenerRunnable)
    {
        UE_LOG(LogTemp, Log, TEXT("Stopping training listener thread..."));
        ListenerRunnable->Stop();
        ListenerRunnable.Reset();
    }

    if (GMyLogListener)
    {
        GLog->RemoveOutputDevice(GMyLogListener);
        delete GMyLogListener;
        GMyLogListener = nullptr;

        UE_LOG(LogTemp, Warning, TEXT("[PythonLogListener] Shutdown"));
    }
}

void UTrainingGameInstance::TestExitCommand()
{
    UE_LOG(LogTemp, Warning, TEXT("Manual exit requested from GameInstance."));
#if WITH_EDITOR
    if (GIsEditor && GEditor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stopping PIE session safely"));
        GEditor->RequestEndPlayMap(); // PIE 안전 종료
        return;
    }
#endif
    // Standalone / Headless
    FGenericPlatformMisc::RequestExit(false);
}

void UTrainingGameInstance::StartPingThread()
{
    if (bPingThreadRunning)
        return;

    bPingThreadRunning = true;

    Async(EAsyncExecution::Thread, [this]()
        {
            UE_LOG(LogTemp, Log, TEXT("[PingThread] Started."));
            PingThreadLoop();
            UE_LOG(LogTemp, Log, TEXT("[PingThread] Stopped."));
        });
}

void UTrainingGameInstance::StopPingThread()
{
    bPingThreadRunning = false;
}

void UTrainingGameInstance::PingThreadLoop()
{
    bIsInPingLoop = true;
    while (bPingThreadRunning)
    {
        if (IsEngineExitRequested())
        {
            UE_LOG(LogTemp, Log, TEXT("[PingThread] Engine exit requested, stopping."));
            break;
        }

        CheckConnection();

        FPlatformProcess::Sleep(PingInterval);
    }
}

void UTrainingGameInstance::CheckConnection()
{
    const FString ServerAddress = TEXT("127.0.0.1");

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem)
    {
        HandleNoPong();
        return;
    }

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp(*ServerAddress, bIsValid);
    Addr->SetPort(DefaultPort);
    if (!bIsValid)
    {
        HandleNoPong();
        return;
    }

    FSocket* ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("HealthCheckSocket"), false);
    if (!ClientSocket)
    {
        HandleNoPong();
        return;
    }

    if (ClientSocket->Connect(*Addr))
    {
        FString PingMsg = TEXT("PING");
        FTCHARToUTF8 Converter(*PingMsg);
        int32 BytesSent = 0;
        ClientSocket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

        uint8 RecvData[256] = { 0 };
        int32 BytesRead = 0;
        double StartTime = FPlatformTime::Seconds();
        bool bGotPong = false;

        while (FPlatformTime::Seconds() - StartTime < 3.0)
        {
            if (ClientSocket->Recv(RecvData, sizeof(RecvData), BytesRead) && BytesRead > 0)
            {
                FString Reply = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData))).TrimStartAndEnd();
                if (Reply.Contains(TEXT("PONG")))
                {
                    UE_LOG(LogTemp, Display, TEXT("Listener connection OK (PONG received)."));
                    UE_LOG(LogTemp, Display, TEXT("%s"), *Reply);

                    if (Reply.Contains("SNAPSHOT")) {
                        FString SnapshotPath;
                        if (Reply.Split(TEXT("SNAPSHOT:"), nullptr, &SnapshotPath))
                        {
                            SnapshotPath = SnapshotPath.TrimStartAndEnd(); // 혹시 공백 제거
                            UE_LOG(LogTemp, Log, TEXT("Extracted snapshot path: %s"), *SnapshotPath);

                            FString Lower = SnapshotPath.ToLower();

                            // 파일 구분
                            if (Lower.Contains("encoder"))
                            {
                                CurrentNetworkSet.Encoder = SnapshotPath;
                            }
                            else if (Lower.Contains("decoder"))
                            {
                                CurrentNetworkSet.Decoder = SnapshotPath;
                            }
                            else if (Lower.Contains("policy"))
                            {
                                CurrentNetworkSet.Policy = SnapshotPath;
                            }
                            else if (Lower.Contains("critic"))
                            {
                                CurrentNetworkSet.Critic = SnapshotPath;
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Snapshot file does not match any known type: %s"), *SnapshotPath);
                            }

                            // 네 가지 파일이 모두 모였는지 체크
                            if (CurrentNetworkSet.IsComplete())
                            {
                                // 추가
                                SavedNetworks.Add(CurrentNetworkSet);
                                UE_LOG(LogTemp, Warning, TEXT("Network Set Completed. Total Sets = %d"), SavedNetworks.Num());
                                OverwriteSaveGame();

                                // 저장세트가 4개 초과하면 오래된 것 삭제
                                const int32 MaxSets = 4;
                                while (SavedNetworks.Num() > MaxSets)
                                {
                                    SavedNetworks.RemoveAt(0);
                                    UE_LOG(LogTemp, Warning, TEXT("Removed oldest network set. Now count = %d"), SavedNetworks.Num());
                                }

                                // 현재 세트 초기화
                                CurrentNetworkSet = FNetworkPaths();
                            }
                        }
                    }
                    
                    bGotPong = true;
                    break;
                }
            }
            FPlatformProcess::Sleep(0.05f);
        }

        if (!bGotPong)
        {
            HandleNoPong();
        }
    }
    else {
        HandleNoPong();
    }

    ClientSocket->Close();
    SocketSubsystem->DestroySocket(ClientSocket);
}

void UTrainingGameInstance::HandleNoPong()
{
    if (!bIsInPingLoop)
        return;

    UE_LOG(LogTemp, Warning, TEXT("[PingThread] No PONG received."));

    if (LaunchedProcessHandle.IsValid() && !FPlatformProcess::IsProcRunning(LaunchedProcessHandle))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PingThread] Detected that headless instance is no longer running."));
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
        LaunchedProcessHandle.Reset();
    }

    // 반드시 GameThread에서 실행
    AsyncTask(ENamedThreads::GameThread, [this]()
        {
            SetTrainingState(ETrainingState::Disconnected);
            ShowTrainingStatusUI();
        });

    bIsInPingLoop = false;
}

void UTrainingGameInstance::SaveGameData(const FString& SlotName, int32 StageValue)
{
    UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())
    );

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create SaveGameInstance"));
        return;
    }

    CurrentSaveSlot = SlotName;

    // 전달받은 값 저장
    SaveGameInstance->Stage = StageValue;
    SaveGameInstance->TrainingCountResource = TrainingCountResource;
    if (SavedNetworks.Num() > 0)
    {
        SaveGameInstance->EncoderPath = SavedNetworks.Last().Encoder;
        SaveGameInstance->DecoderPath = SavedNetworks.Last().Decoder;
        SaveGameInstance->PolicyPath = SavedNetworks.Last().Policy;
        SaveGameInstance->CriticPath = SavedNetworks.Last().Critic;
    }
    

    // 슬롯 이름으로 저장
    if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("Game saved successfully to slot: %s (Stage: %d)"), *SlotName, StageValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save game to slot: %s"), *SlotName);
    }
}

void UTrainingGameInstance::LoadGameData(const FString& SlotName)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save slot not found: %s"), *SlotName);
        return;
    }

    UMySaveGame* LoadedGame = Cast<UMySaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load game from slot: %s"), *SlotName);
        return;
    }

    CurrentSaveSlot = SlotName;

    UE_LOG(LogTemp, Log, TEXT("Loaded slot: %s, Stage: %d"), *SlotName, LoadedGame->Stage);
    CurrentStage = LoadedGame->Stage;
    TrainingCountResource = LoadedGame->TrainingCountResource;

    if (!LoadedGame->EncoderPath.IsEmpty() && !LoadedGame->DecoderPath.IsEmpty() &&
        !LoadedGame->PolicyPath.IsEmpty() && !LoadedGame->CriticPath.IsEmpty()) {
        CurrentNetworkSet = FNetworkPaths();
        SavedNetworks.Empty();

        CurrentNetworkSet.Encoder = LoadedGame->EncoderPath;
        CurrentNetworkSet.Decoder = LoadedGame->DecoderPath;
        CurrentNetworkSet.Policy = LoadedGame->PolicyPath;
        CurrentNetworkSet.Critic = LoadedGame->CriticPath;
        if (CurrentNetworkSet.IsComplete())
        {
            SavedNetworks.Add(CurrentNetworkSet);

            const int32 MaxSets = 4;
            while (SavedNetworks.Num() > MaxSets)
            {
                SavedNetworks.RemoveAt(0);
            }

            CurrentNetworkSet = FNetworkPaths();

        }
    }
}

void UTrainingGameInstance::OverwriteSaveGame()
{
    if (!UGameplayStatics::DoesSaveGameExist(CurrentSaveSlot, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot overwrite: Save slot not found: %s"), *CurrentSaveSlot);
        return;
    }

    UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(
        UGameplayStatics::LoadGameFromSlot(CurrentSaveSlot, 0)
    );

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load save for overwrite: %s"), *CurrentSaveSlot);
        return;
    }

    // 값만 갱신
    SaveGameInstance->Stage = CurrentStage;
    SaveGameInstance->TrainingCountResource = TrainingCountResource;

    if (SavedNetworks.Num() > 0)
    {
        SaveGameInstance->EncoderPath = SavedNetworks.Last().Encoder;
        SaveGameInstance->DecoderPath = SavedNetworks.Last().Decoder;
        SaveGameInstance->PolicyPath = SavedNetworks.Last().Policy;
        SaveGameInstance->CriticPath = SavedNetworks.Last().Critic;
    }

    if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, CurrentSaveSlot, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("Save overwritten successfully: %s"), *CurrentSaveSlot);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to overwrite save: %s"), *CurrentSaveSlot);
    }
}

void UTrainingGameInstance::LoadPathsFromJSON()
{
    FString LoadFilePath = FPaths::ProjectSavedDir() / TEXT("Config/AIPaths.json");
    FString JsonString;

    if (!FPaths::FileExists(LoadFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("[LoadPaths] JSON file not found: %s"), *LoadFilePath);
        return;
    }

    if (!FFileHelper::LoadFileToString(JsonString, *LoadFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("[LoadPaths] Failed to load JSON file: %s"), *LoadFilePath);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[LoadPaths] Failed to parse JSON: %s"), *LoadFilePath);
        return;
    }

    auto GetSafeField = [&](const TCHAR* FieldName) -> FString
        {
            return JsonObject->HasTypedField<EJson::String>(FieldName)
                ? JsonObject->GetStringField(FieldName)
                : FString();
        };

    FileBrowserpath = GetSafeField(TEXT("FileBrowserpath"));
    DefaultEncoderpath = GetSafeField(TEXT("DefaultEncoderpath"));
    DefaultPolicypath = GetSafeField(TEXT("DefaultPolicypath"));
    DefaultDecoderpath = GetSafeField(TEXT("DefaultDecoderpath"));

    Encoderpath1 = GetSafeField(TEXT("Encoderpath1"));
    Policypath1 = GetSafeField(TEXT("Policypath1"));
    Decoderpath1 = GetSafeField(TEXT("Decoderpath1"));

    Encoderpath2 = GetSafeField(TEXT("Encoderpath2"));
    Policypath2 = GetSafeField(TEXT("Policypath2"));
    Decoderpath2 = GetSafeField(TEXT("Decoderpath2"));

    Encoderpath3 = GetSafeField(TEXT("Encoderpath3"));
    Policypath3 = GetSafeField(TEXT("Policypath3"));
    Decoderpath3 = GetSafeField(TEXT("Decoderpath3"));

    EngineRelativepath = GetSafeField(TEXT("EngineRelativepath"));
    IntermediateRelativepath = GetSafeField(TEXT("IntermediateRelativepath"));

    Processpath = GetSafeField(TEXT("Processpath"));
    //Processpath = Processpath.TrimStartAndEnd();
    UE_LOG(LogTemp, Warning, TEXT("Processpath: %s"), *Processpath);

    UE_LOG(LogTemp, Warning, TEXT("Len(Processpath) = %d"), Processpath.Len());
    UE_LOG(LogTemp, Warning, TEXT("Len(Hardcoded) = %d"), FString(TEXT("C:/Unreal_Projects/InGameRL/Saved/StagedBuilds/Windows/InGameRL/Binaries/Win64/InGameRL.exe")).Len());

    auto ToAbsolutePath = [&](const FString& RelativePath) -> FString
        {
            if (RelativePath.IsEmpty())
            {
                return FString();
            }

            // 프로젝트 루트 기준으로 상대 경로 정규화
            FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), RelativePath);

            // 슬래시 정규화
            FPaths::NormalizeDirectoryName(FullPath);

            return FullPath;
        };

    if (!EngineRelativepath.IsEmpty())
    {
        FString AbsEnginePath = ToAbsolutePath(EngineRelativepath);
        FString FullProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
        UE_LOG(LogTemp, Warning, TEXT("Full ProjectDir: %s"), *FullProjectDir);
        UE_LOG(LogTemp, Log, TEXT("[LoadPaths] EngineRelativepath Abs: %s"), *AbsEnginePath);

        TrainerProcessSettings.EditorEngineRelativePath.Path = AbsEnginePath;
        TrainerProcessSettings.NonEditorEngineRelativePath = AbsEnginePath;
    }

    // IntermediateRelativepath (FDirectoryPath 적용)
    if (!IntermediateRelativepath.IsEmpty())
    {
        FString AbsIntermediatePath = ToAbsolutePath(IntermediateRelativepath);

        UE_LOG(LogTemp, Log, TEXT("[LoadPaths] IntermediateRelativepath Abs: %s"), *AbsIntermediatePath);

        TrainerProcessSettings.EditorIntermediateRelativePath.Path = AbsIntermediatePath;
        TrainerProcessSettings.NonEditorIntermediateRelativePath = AbsIntermediatePath;
    }

    UE_LOG(LogTemp, Log, TEXT("[LoadPaths] Successfully loaded JSON: %s"), *LoadFilePath);
}

TArray<FString> UTrainingGameInstance::GetAllSaveSlots() const
{
    TArray<FString> FoundFiles;

    // SaveGames 폴더 경로
    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames/");

    // *.sav 파일 검색
    IFileManager::Get().FindFiles(FoundFiles, *SaveDir, TEXT("*.sav"));

    // 확장자 제거 ("Slot1.sav" → "Slot1")
    for (FString& FileName : FoundFiles)
    {
        FileName = FPaths::GetBaseFilename(FileName);
    }

    return FoundFiles;
}

void UTrainingGameInstance::SetTrainingState(ETrainingState NewState)
{
    TrainingState = NewState;

    if (TrainingStatusWidget)
    {
        TrainingStatusWidget->UpdateTrainingState(NewState);
    }
}

void UTrainingGameInstance::ShowTrainingStatusUI()
{
    if (TrainingStatusWidget && TrainingStatusWidget->IsInViewport())
        return;

    if (!TrainingStatusWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TrainingStatusWidgetClass is not set in GameInstance!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    TrainingStatusWidget = CreateWidget<UTrainingStatusWidget>(World, TrainingStatusWidgetClass);
    if (TrainingStatusWidget)
    {
        TrainingStatusWidget->AddToViewport(1000);
        TrainingStatusWidget->UpdateTrainingState(TrainingState);
    }
}

void UTrainingGameInstance::HideTrainingStatusUI()
{
    if (TrainingStatusWidget)
    {
        TrainingStatusWidget->RemoveFromParent();
        TrainingStatusWidget = nullptr;
    }
}

void UTrainingGameInstance::LaunchNewInstance(
    const FString& MapName,
    float RewardWeight11,
    float RewardWeight22,
    float RewardWeight33,
    int32 TrainingCount1,
    const FString& EncoderPath1,
    const FString& DecoderPath1,
    const FString& PolicyPath1,
    const FString& CriticPath1
)
{
    if (LaunchedProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(LaunchedProcessHandle))
    {
        UE_LOG(LogTemp, Warning, TEXT("Headless instance already running."));
        return;
    }

    //FString ExePath = TEXT("C:/Unreal_Projects/InGameRL/Saved/StagedBuilds/Windows/InGameRL/Binaries/Win64/InGameRL.exe");
    FString ExePath = Processpath;
    FString Params = FString::Printf(
        TEXT("%s -RenderOffScreen -nosound -Unattended -Port=%d -RewardWeight1=%f -RewardWeight2=%f -RewardWeight3=%f -TrainingCount=%d -Train=1 -EncoderPath=%s -DecoderPath=%s -PolicyPath=%s -CriticPath=%s"),
        *MapName, DefaultPort, RewardWeight11, RewardWeight22, RewardWeight33, TrainingCount1, *EncoderPath1, *DecoderPath1, *PolicyPath1, *CriticPath1
    );

    LaunchedProcessHandle = FPlatformProcess::CreateProc(*ExePath, *Params, true, false, false, nullptr, 0, nullptr, nullptr);

    if (LaunchedProcessHandle.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Launched headless instance: %s"), *MapName);

        Async(EAsyncExecution::Thread, [this]()
            {
                FPlatformProcess::Sleep(5.0f);
                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        SetTrainingState(ETrainingState::Running);
                        ShowTrainingStatusUI();

                        if (!IsPingThreadRunning()) StartPingThread();
                    });
            });
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to launch headless instance."));
    }
}

void UTrainingGameInstance::TerminateInstance()
{
    if (!LaunchedProcessHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No running instance to terminate."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Requesting EXIT..."));
    SendExitSignal();
}

void UTrainingGameInstance::SendExitSignal()
{
    Async(EAsyncExecution::Thread, [this]()
        {
            const FString AddrStr = TEXT("127.0.0.1");
            ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
            if (!SocketSubsystem) return;

            TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
            bool bValid = false;
            Addr->SetIp(*AddrStr, bValid);
            Addr->SetPort(DefaultPort);

            if (!bValid) return;

            FSocket* Sock = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("ExitSocket"), false);
            if (!Sock) return;

            if (!Sock->Connect(*Addr))
            {
                Sock->Close();
                SocketSubsystem->DestroySocket(Sock);
                return;
            }

            FString Msg = TEXT("EXIT");
            FTCHARToUTF8 Converter(*Msg);
            int32 BytesSent = 0;
            Sock->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

            uint8 RecvData[256] = { 0 };
            int32 BytesRead = 0;
            bool bAck = false;
            double Start = FPlatformTime::Seconds();

            while (FPlatformTime::Seconds() - Start < 3.0)
            {
                if (Sock->Recv(RecvData, sizeof(RecvData), BytesRead) && BytesRead > 0)
                {
                    FString Reply = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData)));
                    if (Reply.Contains(TEXT("ACK_EXIT")))
                    {
                        bAck = true;
                        break;
                    }
                }
                FPlatformProcess::Sleep(0.05f);
            }

            Sock->Close();
            SocketSubsystem->DestroySocket(Sock);

            AsyncTask(ENamedThreads::GameThread, [this, bAck]() { OnExitSignalCompleted(bAck); });
        });
}

void UTrainingGameInstance::OnExitSignalCompleted(bool bAckReceived)
{
    StopPingThread();
    SetTrainingState(ETrainingState::Disconnected);
    ShowTrainingStatusUI();

    if (!LaunchedProcessHandle.IsValid()) return;

    if (bAckReceived)
    {
        UE_LOG(LogTemp, Log, TEXT("Headless instance closed gracefully."));
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Forcefully terminating instance."));
        if (FPlatformProcess::IsProcRunning(LaunchedProcessHandle))
            FPlatformProcess::TerminateProc(LaunchedProcessHandle);
        FPlatformProcess::CloseProc(LaunchedProcessHandle);
    }

    LaunchedProcessHandle.Reset();
}

void UTrainingGameInstance::RequestActorInfo()
{
    Async(EAsyncExecution::Thread, [this]()
        {
            const FString AddrStr = TEXT("127.0.0.1");
            ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
            if (!SocketSubsystem) return;

            TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
            bool bValid = false;
            Addr->SetIp(*AddrStr, bValid);
            Addr->SetPort(DefaultPort);
            if (!bValid) return;

            FSocket* Sock = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("ActorInfoSocket"), false);
            if (!Sock) return;
            if (!Sock->Connect(*Addr))
            {
                Sock->Close();
                SocketSubsystem->DestroySocket(Sock);
                return;
            }

            FString Msg = TEXT("GET_ACTOR_INFO");
            FTCHARToUTF8 Conv(*Msg);
            int32 BytesSent = 0;
            Sock->Send((uint8*)Conv.Get(), Conv.Length(), BytesSent);

            uint8 RecvData[1024] = { 0 };
            int32 BytesRead = 0;
            double Start = FPlatformTime::Seconds();
            FString Result;

            while (FPlatformTime::Seconds() - Start < 3.0)
            {
                if (Sock->Recv(RecvData, sizeof(RecvData), BytesRead) && BytesRead > 0)
                {
                    Result = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(RecvData))).TrimStartAndEnd();
                    break;
                }
                FPlatformProcess::Sleep(0.05f);
            }

            Sock->Close();
            SocketSubsystem->DestroySocket(Sock);

            if (!Result.IsEmpty())
            {
                UE_LOG(LogTemp, Log, TEXT("Actor Info:\n%s"), *Result);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No actor info received."));
            }
        });
}


