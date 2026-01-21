// Fill out your copyright notice in the Description page of Project Settings.


#include "StageController.h"
#include "RLCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "RLCharAIController.h"
#include "TrainingGameInstance.h"

void AStageController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    // Enhanced Input Subsystem 연결
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(BattleMappingContext, 0);
        }
    }

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(), ARLCharacter::StaticClass(), Actors);

    if (Actors.Num() > 0)
    {
        SelectedUnit = Cast<ARLCharacter>(Actors[0]);
    }

    if (SelectedUnit)
    {
        SetControlRotation((SelectedUnit->GetActorLocation() - GetPawn()->GetActorLocation()).Rotation());
    }

    SetStage();
}

void AStageController::SetStage()
{
    // ---- 현재 스테이지 정보 불러오기 ----
    UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance());
    if (!GI) return;

    int32 CurrentStage = GI->GetCurrentStage();

    // ---- "stage" 태그를 가진 모든 액터 검색 ----
    TArray<AActor*> StageActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("stage"), StageActors);

    for (AActor* Actor : StageActors)
    {
        if (!Actor) continue;

        // 액터의 태그 목록에서 스테이지 번호 찾기
        int32 StageNumber = -1;
        for (const FName& Tag : Actor->Tags)
        {
            FString TagStr = Tag.ToString();
            if (TagStr.IsNumeric())
            {
                StageNumber = FCString::Atoi(*TagStr);
                break;
            }
        }

        if (StageNumber == -1)
            continue;

        // StaticMeshComponent 찾기
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (!MeshComp) continue;

        // 변경할 머티리얼 선택
        UMaterialInterface* TargetMat = nullptr;

        if (StageNumber < CurrentStage)
            TargetMat = StageMaterial_clear;   // 지나온 스테이지
        else if (StageNumber > CurrentStage)
            TargetMat = StageMaterial_unclear;   // 아직 도달하지 않은 스테이지
        else
            TargetMat = StageMaterial_selected;   // 현재 스테이지

        if (TargetMat)
        {
            MeshComp->SetMaterial(0, TargetMat);
        }
    }
}

void AStageController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Triggered, this, &AStageController::OnLeftClickTriggered);
        EnhancedInput->BindAction(RightClickAction, ETriggerEvent::Triggered, this, &AStageController::OnRightClickTriggered);
    }
    else
    {
        // UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed"));
    }
}

void AStageController::OnLeftClickTriggered(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnLeftClick"));
 
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {

    }
}

void AStageController::OnRightClickTriggered(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnRightClick"));

    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {
        AActor* HitActor = Hit.GetActor();

        // "stage" 태그가 있는 액터 클릭 시
        if (HitActor && HitActor->ActorHasTag("stage"))
        {
            UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance());
            if (!GI) return;

            int32 StageNumber = -1;
            for (const FName& Tag : HitActor->Tags)
            {
                FString TagStr = Tag.ToString();
                if (TagStr.IsNumeric())
                {
                    StageNumber = FCString::Atoi(*TagStr);
                    break;
                }
            }
            if (StageNumber == -1)
                return;

            // ---- MeshComponent 찾기 ----
            UStaticMeshComponent* MeshComp = HitActor->FindComponentByClass<UStaticMeshComponent>();
            if (!MeshComp)
                return;

            UMaterialInterface* CurrentMat = MeshComp->GetMaterial(0);

            if (CurrentMat != StageMaterial_unclear)
            {
                FVector TargetLocation = HitActor->GetActorLocation();
                GI->SetCurrentSelectedStage(StageNumber);

                if (SelectedUnit)
                {
                    if (ARLCharAIController* UnitAI = Cast<ARLCharAIController>(SelectedUnit->GetController()))
                    {
                        UnitAI->MoveToTargetLocation(TargetLocation);
                    }
                }
            }
        }
    }
}

void AStageController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (SelectedUnit)
    {
        FVector TargetPos = SelectedUnit->GetActorLocation() + FVector(0.0f, 170.0f, 50.0f);

        // ---- 오프셋 설정 ----
        FVector Offset = FVector(-330.0f, 0.0f, 150.0f); // 뒤쪽/위쪽에서 바라보는 시점
        // FVector DesiredPos = TargetPos + SelectedUnit->GetActorRotation().RotateVector(Offset);
        FVector DesiredPos = TargetPos + Offset;

        // ---- 보간 이동 ----
        FVector CurrentPos = GetPawn()->GetActorLocation();
        FVector NewPos = FMath::VInterpTo(CurrentPos, DesiredPos, DeltaTime, 5.f);
        GetPawn()->SetActorLocation(NewPos);

        // ---- 바라보는 방향 ----
        // FRotator LookRot = (TargetPos - DesiredPos).Rotation();
        // GetPawn()->SetActorRotation(LookRot);
        SetControlRotation((TargetPos - GetPawn()->GetActorLocation()).Rotation());
    }
}