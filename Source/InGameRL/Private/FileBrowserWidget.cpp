// Fill out your copyright notice in the Description page of Project Settings.


#include "FileBrowserWidget.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Components/ListView.h"
#include "UFileItemObject.h"

void UFileBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 루트 경로 지정 (예: Saved/Networks/)
    if (RootPath.IsEmpty())
    {
        RootPath = FPaths::ProjectSavedDir() / TEXT("Networks");
    }

    CurrentPath = RootPath;
    SelectedFile.Empty();
    UE_LOG(LogTemp, Warning, TEXT("CurrentPath %s"), *CurrentPath);
}

void UFileBrowserWidget::RefreshDirectory(TArray<FString>& OutFolders, TArray<FString>& OutFiles)
{
    OutFolders.Empty();
    OutFiles.Empty();

    IFileManager& FileManager = IFileManager::Get();

    // 폴더
    FileManager.FindFiles(OutFolders, *(CurrentPath / TEXT("*")), false, true);

    // 파일
    FileManager.FindFiles(OutFiles, *(CurrentPath / TEXT("*")), true, false);
}

void UFileBrowserWidget::GoToFolder(const FString& FolderName)
{
    FString NewPath = FPaths::Combine(CurrentPath, FolderName);
    if (IFileManager::Get().DirectoryExists(*NewPath))
    {
        CurrentPath = NewPath;
        SelectedFile.Empty();
    }
}

void UFileBrowserWidget::GoUpOneLevel()
{
    FString ParentPath = FPaths::GetPath(CurrentPath);

    if (CurrentPath != RootPath && CurrentPath.StartsWith(RootPath))
    {
        CurrentPath = ParentPath;
        SelectedFile.Empty();
    }
}

void UFileBrowserWidget::SelectFile(const FString& FileName)
{
    SelectedFile = FPaths::Combine(CurrentPath, FileName);
    EncoderPath.Empty();
    DecoderPath.Empty();
    PolicyPath.Empty();
    CriticPath.Empty();
}

bool UFileBrowserWidget::CreateFolder(const FString& FolderName)
{
    FString NewFolderPath = FPaths::Combine(CurrentPath, FolderName);
    return IFileManager::Get().MakeDirectory(*NewFolderPath, true);
}

bool UFileBrowserWidget::DeleteFolder(const FString& FolderName)
{
    FString FolderPath = FPaths::Combine(CurrentPath, FolderName);
    return IFileManager::Get().DeleteDirectory(*FolderPath, false, true);
}

bool UFileBrowserWidget::DeleteFile(const FString& FileName)
{
    FString FilePath = FPaths::Combine(CurrentPath, FileName);
    return IFileManager::Get().Delete(*FilePath);
}

bool UFileBrowserWidget::MoveFileToFolder(const FString& FileName, const FString& TargetFolderName)
{
    // 원본 파일 경로
    FString SourceFilePath = FPaths::Combine(CurrentPath, FileName);

    // 목적지 폴더 (CurrentPath/TargetFolderName)
    FString TargetFolderPath = FPaths::Combine(CurrentPath, TargetFolderName);

    if (!IFileManager::Get().DirectoryExists(*TargetFolderPath))
    {
        return false; // 대상 폴더가 없으면 실패
    }

    // 파일명만 추출
    FString JustFileName = FPaths::GetCleanFilename(FileName);

    // 최종 목적지 파일 경로
    FString DestFilePath = FPaths::Combine(TargetFolderPath, JustFileName);

    // 이동 시도
    return IFileManager::Get().Move(*DestFilePath, *SourceFilePath, true, true);
}

void UFileBrowserWidget::LoadDirectory(const FString& DirectoryPath, const FString& FilterPattern /*="*.*"*/)
{
    FileListView->ClearListItems();

    IFileManager& FM = IFileManager::Get();

    if (!DirectoryPath.Equals(RootPath, ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Warning, TEXT("ParentItem"));
        UUFileItemObject* ParentItem = NewObject<UUFileItemObject>(this);
        ParentItem->Name = TEXT("..");
        ParentItem->Path = FPaths::GetPath(DirectoryPath); // 상위 경로
        ParentItem->bIsDirectory = true;
        FileListView->AddItem(ParentItem);
    }

    // 먼저 폴더
    TArray<FString> FoundFolders;
    FM.FindFiles(FoundFolders, *(DirectoryPath / TEXT("*")), false, true);
    for (const FString& F : FoundFolders)
    {
        UUFileItemObject* Item = NewObject<UUFileItemObject>(this);
        Item->Name = F;
        Item->Path = FPaths::Combine(DirectoryPath, F);
        Item->bIsDirectory = true;
        // 아이템 추가
        FileListView->AddItem(Item);
    }

    // 파일 (필터 적용)
    TArray<FString> FoundFiles;
    FM.FindFiles(FoundFiles, *(DirectoryPath / FilterPattern), true, false);
    for (const FString& F : FoundFiles)
    {
        UUFileItemObject* Item = NewObject<UUFileItemObject>(this);
        Item->Name = F;
        Item->Path = FPaths::Combine(DirectoryPath, F);
        Item->bIsDirectory = false;
        Item->ModifiedTime = FM.GetTimeStamp(*Item->Path);
        FileListView->AddItem(Item);
    }

    // 정렬: ListView에 넣기 전에 TArray 정렬을 해도 되고, 위젯이 나오고 난 후 FileListView->RequestListRefresh() 호출
}

void UFileBrowserWidget::LoadGroupedFiles(const FString& DirectoryPath, const FString& FilterPattern /*= "*.bin"*/)
{
    FileListView->ClearListItems();
    IFileManager& FM = IFileManager::Get();

    // 상위 폴더 아이템 추가 (원하면 제거 가능)
    if (!DirectoryPath.Equals(RootPath, ESearchCase::IgnoreCase))
    {
        UUFileItemObject* ParentItem = NewObject<UUFileItemObject>(this);
        ParentItem->Name = TEXT("..");
        ParentItem->Path = FPaths::GetPath(DirectoryPath);
        ParentItem->bIsDirectory = true;
        FileListView->AddItem(ParentItem);
    }

    // 파일 검색
    TArray<FString> FoundFiles;
    FM.FindFiles(FoundFiles, *(DirectoryPath / FilterPattern), true, false);

    // 그룹화용 맵 (예: "1100_100" → 관련 파일 목록)
    TMap<FString, TArray<FString>> FileGroups;

    for (const FString& F : FoundFiles)
    {
        // 예시: 1100_critic_100.bin
        FString BaseName = FPaths::GetBaseFilename(F);
        TArray<FString> Parts;
        BaseName.ParseIntoArray(Parts, TEXT("_"), true);

        if (Parts.Num() == 3)
        {
            FString Prefix = Parts[0];
            FString Type = Parts[1]; // critic, encoder, decoder, policy 등
            FString Suffix = Parts[2];

            FString Key = Prefix + TEXT("_") + Suffix; // 그룹 이름: "1100_100"
            FileGroups.FindOrAdd(Key).Add(F);
        }
    }

    // 그룹 키 정렬
    TArray<FString> SortedKeys;
    FileGroups.GetKeys(SortedKeys);
    SortedKeys.Sort([](const FString& A, const FString& B)
        {
            return A.Compare(B, ESearchCase::IgnoreCase) < 0;
        });

    // 그룹을 UI에 표시
    for (const FString& Key : SortedKeys)
    {
        UUFileItemObject* Item = NewObject<UUFileItemObject>(this);
        Item->Name = Key;
        Item->Path = DirectoryPath;
        Item->bIsDirectory = false;
        Item->GroupFiles = FileGroups[Key];
        Item->bIsGroupFile = true;
        FileListView->AddItem(Item);
    }
}

void UFileBrowserWidget::HandleGroupedFileClick(UObject* ClickedItem)
{
    UUFileItemObject* FileItem = Cast<UUFileItemObject>(ClickedItem);
    if (!FileItem)
        return;

    EncoderPath.Empty();
    DecoderPath.Empty();
    PolicyPath.Empty();
    CriticPath.Empty();

    // 키워드별 변수 포인터 매핑
    TMap<FString, FString*> PathMap = {
        { TEXT("encoder"), &EncoderPath },
        { TEXT("decoder"), &DecoderPath },
        { TEXT("policy"),  &PolicyPath },
        { TEXT("critic"),  &CriticPath }
    };

    const TArray<FString>& TargetFiles =
        (FileItem->GroupFiles.Num() > 0) ? FileItem->GroupFiles : TArray<FString>{ FileItem->Name };

    for (const FString& FileName : TargetFiles)
    {
        FString FullPath = FPaths::Combine(FileItem->Path, FileName);
        for (const auto& Pair : PathMap)
        {
            if (FileName.Contains(Pair.Key, ESearchCase::IgnoreCase))
            {
                *Pair.Value = FullPath;
                break;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Encoder: %s"), *EncoderPath);
    UE_LOG(LogTemp, Warning, TEXT("Decoder: %s"), *DecoderPath);
    UE_LOG(LogTemp, Warning, TEXT("Policy: %s"), *PolicyPath);
    UE_LOG(LogTemp, Warning, TEXT("Critic: %s"), *CriticPath);
}

bool UFileBrowserWidget::IsGroupedDirectory(const FString& DirectoryPath)
{
    // 현재 폴더 내 파일만 탐색 (하위폴더는 제외)
    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *(DirectoryPath / TEXT("*.*")), true, false);

    TMap<FString, int32> BaseNameCount;

    for (const FString& FileName : Files)
    {
        FString BaseName = FileName;
        bool bMatched = false;

        const TArray<FString> Keywords = { TEXT("_encoder"), TEXT("_decoder"), TEXT("_policy"), TEXT("_critic") };

        for (const FString& Key : Keywords)
        {
            int32 Index = FileName.Find(Key, ESearchCase::IgnoreCase, ESearchDir::FromStart);
            if (Index != INDEX_NONE)
            {
                BaseName = FileName.Left(Index);
                BaseNameCount.FindOrAdd(BaseName)++;
                bMatched = true;
                break;
            }
        }
    }

    // 한 세트라도 4개가 있으면 Grouped Directory로 판단
    for (const auto& Pair : BaseNameCount)
    {
        if (Pair.Value >= 3)
        {
            return true;
        }
    }

    return false;
}

void UFileBrowserWidget::LoadDir(const FString& Path)
{
    if (IsGroupedDirectory(Path))
    {
        UE_LOG(LogTemp, Log, TEXT("Grouped directory detected. Using LoadGroupedFiles."));
        LoadGroupedFiles(Path, "*.bin");
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Normal directory detected. Using LoadNormalFiles."));
        LoadDirectory(Path, "*.*");
    }
}