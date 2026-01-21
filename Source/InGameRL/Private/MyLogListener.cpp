// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLogListener.h"

MyLogListener::MyLogListener()
{
}

MyLogListener::~MyLogListener()
{
}

void MyLogListener::Serialize(
    const TCHAR* V,
    ELogVerbosity::Type Verbosity,
    const FName& Category
)
{
    // LearningAgents Python Subprocess 로그만 처리
    if (Category != TEXT("LogLearning"))
        return;

    FString Message(V);

    // 관심있는 Snapshot 로그인지 확인
    if (!Message.Contains(TEXT("Saved")) ||
        !Message.Contains(TEXT("Snapshot to:")))
    {
        return;
    }

    // 큰따옴표 안의 파일 경로를 추출
    int32 QuoteStart;
    if (Message.FindChar('"', QuoteStart))
    {
        int32 QuoteEnd;
        if (Message.FindLastChar('"', QuoteEnd) && QuoteEnd > QuoteStart)
        {
            FString FilePath = Message.Mid(
                QuoteStart + 1,
                QuoteEnd - QuoteStart - 1
            );

            // 경로를 외부로 전달
            if (OnSnapshotPathFound.IsBound())
            {
                OnSnapshotPathFound.Execute(FilePath);
            }
        }
    }
}