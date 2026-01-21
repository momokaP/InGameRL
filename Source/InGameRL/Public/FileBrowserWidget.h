// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FileBrowserWidget.generated.h"

/**
 * 
 */

class UListView;

UCLASS()
class INGAMERL_API UFileBrowserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "FileBrowser")
    FString CurrentPath;

    UPROPERTY(BlueprintReadWrite, Category = "FileBrowser")
    FString SelectedFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileBrowser")
    FString RootPath;

    UPROPERTY(meta = (BindWidget))
    UListView* FileListView;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> GroupFiles;

    UPROPERTY(BlueprintReadOnly, Category = "FileBrowser")
    FString EncoderPath;

    UPROPERTY(BlueprintReadOnly, Category = "FileBrowser")
    FString DecoderPath;

    UPROPERTY(BlueprintReadOnly, Category = "FileBrowser")
    FString PolicyPath;

    UPROPERTY(BlueprintReadOnly, Category = "FileBrowser")
    FString CriticPath;

    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void RefreshDirectory(TArray<FString>& OutFolders, TArray<FString>& OutFiles);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void GoToFolder(const FString& FolderName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void GoUpOneLevel();

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void SelectFile(const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    bool CreateFolder(const FString& FolderName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    bool DeleteFolder(const FString& FolderName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    bool DeleteFile(const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    bool MoveFileToFolder(const FString& FileName, const FString& TargetFolderName);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void LoadDirectory(const FString& DirectoryPath, const FString& FilterPattern /*="*.*"*/);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void LoadGroupedFiles(const FString& DirectoryPath, const FString& FilterPattern /*= "*.bin"*/);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void HandleGroupedFileClick(UObject* ClickedItem);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    bool IsGroupedDirectory(const FString& DirectoryPath);

    UFUNCTION(BlueprintCallable, Category = "FileBrowser")
    void LoadDir(const FString& Path);
};
