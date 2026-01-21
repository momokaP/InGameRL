// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"

#include "BattleController.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ABattleHUD::DrawHUD()
{
    Super::DrawHUD();

    ABattleController* PC = Cast<ABattleController>(GetOwningPlayerController());
    if (!PC || !PC->bIsDragging) return;

    FVector2D Start = PC->DragStartScreenPos;
    FVector2D Current;
    PC->GetMousePosition(Current.X, Current.Y);

    // 최소/최대 정리
    const float MinX = FMath::Min(Start.X, Current.X);
    const float MinY = FMath::Min(Start.Y, Current.Y);
    const float MaxX = FMath::Max(Start.X, Current.X);
    const float MaxY = FMath::Max(Start.Y, Current.Y);

    const float Width = MaxX - MinX;
    const float Height = MaxY - MinY;

    // 반투명 박스 내부 색
    FLinearColor BoxColor(0.0f, 0.4f, 1.0f, 0.15f);

    // 테두리 색
    FLinearColor LineColor(0.0f, 0.6f, 1.0f, 0.8f);

    // 사각형 내부 채우기
    DrawRect(BoxColor, MinX, MinY, Width, Height);

    // 테두리 선 4개
    const float Thickness = 1.5f;
    DrawLine(MinX, MinY, MaxX, MinY, LineColor, Thickness);
    DrawLine(MaxX, MinY, MaxX, MaxY, LineColor, Thickness);
    DrawLine(MaxX, MaxY, MinX, MaxY, LineColor, Thickness);
    DrawLine(MinX, MaxY, MinX, MinY, LineColor, Thickness);
}
