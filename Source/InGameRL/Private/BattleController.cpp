// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleController.h"
#include "RLCharAIController.h"
#include "RLCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h" 
#include "Kismet/GameplayStatics.h"

void ABattleController::BeginPlay()
{
    Super::BeginPlay();

    if (APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->bUseControllerRotationYaw = true;
        ControlledPawn->bUseControllerRotationPitch = true;
    }

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    SetIgnoreLookInput(true);

    // Enhanced Input Subsystem 연결
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(BattleMappingContext, 0);
        }
    }

    bBattleStarted = false;

}

void ABattleController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Triggered, this, &ABattleController::OnLeftClickTriggered);
        EnhancedInput->BindAction(RightClickAction, ETriggerEvent::Triggered, this, &ABattleController::OnRightClickTriggered);
    
        EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ABattleController::OnLeftClickStarted);
        EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Canceled, this, &ABattleController::OnLeftClickReleased);
        EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &ABattleController::OnLeftClickReleased);
        //EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::None, this, &ABattleController::OnLeftClickReleased);
    
        EnhancedInput->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ABattleController::MoveForward);
        EnhancedInput->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ABattleController::MoveRight);
        EnhancedInput->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &ABattleController::MoveUp);
        EnhancedInput->BindAction(RotateYawAction, ETriggerEvent::Triggered, this, &ABattleController::RotateYaw);
        EnhancedInput->BindAction(RotatePitchAction, ETriggerEvent::Triggered, this, &ABattleController::RotatePitch);
    }
    else
    {
        // UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed"));
    }
}

void ABattleController::MoveForward(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        float Axis = Value.Get<float>();
        ControlledPawn->AddMovementInput(ControlledPawn->GetActorForwardVector(), Axis);
    }
}

void ABattleController::MoveRight(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        float Axis = Value.Get<float>();
        ControlledPawn->AddMovementInput(ControlledPawn->GetActorRightVector(), Axis);
    }
}

void ABattleController::MoveUp(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        float Axis = Value.Get<float>();
        ControlledPawn->AddMovementInput(FVector::UpVector, Axis);
    }
}

void ABattleController::RotateYaw(const FInputActionValue& Value)
{
    float Axis = Value.Get<float>();
    // UE_LOG(LogTemp, Warning, TEXT("RotateYaw Axis: %f"), Axis);
    FRotator ControlRot = GetControlRotation();
    ControlRot.Yaw += Axis;
    SetControlRotation(ControlRot);
}

void ABattleController::RotatePitch(const FInputActionValue& Value)
{
    float Axis = Value.Get<float>();
    // UE_LOG(LogTemp, Display, TEXT("RotatePitch Axis: %f"), Axis);
    FRotator ControlRot = GetControlRotation();
    ControlRot.Pitch += Axis;
    SetControlRotation(ControlRot);
}

void ABattleController::OnLeftClickTriggered(const FInputActionValue& Value)
{
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    // UE_LOG(LogTemp, Warning, TEXT("OnLeftClick"));

    if (Hit.bBlockingHit)
    {
        /*UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s (%s)"),
            *Hit.GetActor()->GetName(),
            *Hit.GetActor()->GetClass()->GetName());*/

        ARLCharacter* HitUnit = Cast<ARLCharacter>(Hit.GetActor());
        if (HitUnit)
        {
            SelectUnit(HitUnit);
        }
        else
        {
            DeselectAll();
        }
    }
}

void ABattleController::OnRightClickTriggered(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnRightClick"));

    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (AActor* HitActor = Hit.GetActor())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit Actor: None"));
    }

    // 선택된 유닛이 없으면 종료
    if (!SelectedUnit && SelectedUnits.Num() == 0) return;

    if (Hit.bBlockingHit)
    {
        FVector TargetLocation = Hit.ImpactPoint;

        // 여러 유닛 선택된 경우
        if (SelectedUnits.Num() > 0)
        {
            // 여러 유닛 이동 처리
            int32 Index = 0;
            const float FormationSpacing = 150.f; // 유닛 간격

            for (ARLCharacter* Unit : SelectedUnits)
            {
                if (!Unit) continue;
                if (Unit->GetTeamID() != 1) continue;

                FVector Offset(
                    (Index % 3 - 1) * FormationSpacing,   // X축 (좌우)
                    (Index / 3) * FormationSpacing,       // Y축 (앞뒤)
                    0.0f
                );
                FVector MoveLocation = TargetLocation + Offset;

                if (bBattleStarted)
                {
                    ARLCharAIController* UnitAI = Cast<ARLCharAIController>(Unit->GetController());
                    if (UnitAI)
                    {
                        UnitAI->MoveToTargetLocation(MoveLocation);
                    }
                }
                else
                {
                    AActor* HitActor = Hit.GetActor();
                    if (HitActor && HitActor->ActorHasTag("DeploymentPlane") && HitActor->ActorHasTag("1"))
                    {
                        if (CanPlaceAtLocation(MoveLocation))
                        {
                            Unit->SetActorLocation(MoveLocation, false, nullptr, ETeleportType::TeleportPhysics);
                        }
                    }
                }

                Index++;
            }

            return;
        }

        // 단일 유닛 선택 시 기존 로직 그대로
        if (SelectedUnit && SelectedUnit->GetTeamID() == 1)
        {
            if (bBattleStarted)
            {
                ARLCharacter* TargetUnit = Cast<ARLCharacter>(Hit.GetActor());
                if (TargetUnit && TargetUnit != SelectedUnit)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Target Unit clicked: %s"), *TargetUnit->GetName());
                    SelectedUnit->SetAttackTarget(TargetUnit);

                    FVector MyLocation = SelectedUnit->GetActorLocation();
                    FVector EnemyLocation = TargetUnit->GetActorLocation();
                    FVector Direction = (EnemyLocation - MyLocation).GetSafeNormal();

                    float DesiredDistance = 150.f;
                    FVector MoveLocation = EnemyLocation - Direction * DesiredDistance;

                    if (ARLCharAIController* UnitAI = Cast<ARLCharAIController>(SelectedUnit->GetController()))
                    {
                        UnitAI->MoveToTargetLocation(MoveLocation);
                        UE_LOG(LogTemp, Warning, TEXT("Moving near target to: %s"), *MoveLocation.ToString());
                    }
                    return;
                }

                if (ARLCharAIController* UnitAI = Cast<ARLCharAIController>(SelectedUnit->GetController()))
                {
                    UnitAI->MoveToTargetLocation(TargetLocation);
                }
            }
            else
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor && HitActor->ActorHasTag("DeploymentPlane") && HitActor->ActorHasTag("1"))
                {
                    if (CanPlaceAtLocation(TargetLocation))
                    {
                        SelectedUnit->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("이미 다른 유닛이 근처에 있습니다."));
                    }
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("Unit moved to: %s"), *TargetLocation.ToString());
        }
    }
}

void ABattleController::SelectUnit(ARLCharacter* Unit)
{
    DeselectAll();
    
    SelectedUnit = Unit;
    SelectedUnit->SetSelected(true);

    if (AController* Controller = SelectedUnit->GetController())
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller: %s"), *Controller->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Controller found for unit!"));
    }
}

void ABattleController::DeselectAll()
{
    if (SelectedUnit)
    {
        SelectedUnit->SetSelected(false);
        SelectedUnit = NULL;
    }

    for (auto* U : SelectedUnits)
        U->SetSelected(false);
    SelectedUnits.Empty();
}

bool ABattleController::CanPlaceAtLocation(const FVector& Location)
{
    FCollisionShape Sphere = FCollisionShape::MakeSphere(50.f); // 반경 100
    TArray<FOverlapResult> Overlaps;

    bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    for (const FOverlapResult& Result : Overlaps)
    {
        if (Result.GetActor() && Result.GetActor()->IsA<ARLCharacter>())
        {
            return false;
        }
    }

    return true;

    // return !bHasOverlap; // 겹치지 않으면 true
}

void ABattleController::OnLeftClickStarted(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("OnLeftClickStarted"));
    bIsDragging = true;
    GetMousePosition(DragStartScreenPos.X, DragStartScreenPos.Y);
}

void ABattleController::OnLeftClickReleased(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("OnLeftClickReleased"));
    bIsDragging = false;
    GetMousePosition(DragEndScreenPos.X, DragEndScreenPos.Y);

    // 드래그 거리 계산
    const float DragThreshold = 5.0f;
    if (FVector2D::Distance(DragStartScreenPos, DragEndScreenPos) > DragThreshold)
    {
        // 박스 선택
        SelectUnitsInDragBox();
    }
    else
    {
        // 클릭 선택
        FHitResult Hit;
        GetHitResultUnderCursor(ECC_Visibility, false, Hit);
        if (ARLCharacter* HitUnit = Cast<ARLCharacter>(Hit.GetActor()))
        {
            SelectUnit(HitUnit);
        }
        else
        {
            DeselectAll();
        }
    }
}

void ABattleController::SelectUnitsInDragBox()
{
    DeselectAll();

    FVector2D MinPoint(
        FMath::Min(DragStartScreenPos.X, DragEndScreenPos.X),
        FMath::Min(DragStartScreenPos.Y, DragEndScreenPos.Y)
    );
    FVector2D MaxPoint(
        FMath::Max(DragStartScreenPos.X, DragEndScreenPos.X),
        FMath::Max(DragStartScreenPos.Y, DragEndScreenPos.Y)
    );

    FVector WorldOrigin, WorldDir;
    FVector WorldCorners[4];

    auto ScreenToWorld = [&](const FVector2D& ScreenPos, FVector& OutWorldPos)
        {
            DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldOrigin, WorldDir);
            FVector End = WorldOrigin + WorldDir * 100000.f;
            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, End, ECC_Visibility))
            {
                OutWorldPos = Hit.Location;
            }
            else
            {
                OutWorldPos = End;
            }
        };

    // 박스의 네 모서리 월드 좌표 계산
    ScreenToWorld(MinPoint, WorldCorners[0]); // 좌상
    ScreenToWorld(FVector2D(MaxPoint.X, MinPoint.Y), WorldCorners[1]); // 우상
    ScreenToWorld(MaxPoint, WorldCorners[2]); // 우하
    ScreenToWorld(FVector2D(MinPoint.X, MaxPoint.Y), WorldCorners[3]); // 좌하

    // 중심점과 평면 계산
    FVector Center = (WorldCorners[0] + WorldCorners[2]) / 2.0f;
    FVector Normal = FVector::CrossProduct(WorldCorners[1] - WorldCorners[0], WorldCorners[3] - WorldCorners[0]).GetSafeNormal();

    for (TActorIterator<ARLCharacter> It(GetWorld()); It; ++It)
    {
        ARLCharacter* Unit = *It;
        if (!Unit) continue;

        if (Unit->GetTeamID() != 1 && SelectedUnits.Num() > 0) continue;
        // 플레이어 캐릭터가 아니라면 무시

        FVector UnitLoc = Unit->GetActorLocation();

        // 화면 투영 후 간단한 2D check (1차 필터)
        FVector2D ScreenPos;
        ProjectWorldLocationToScreen(UnitLoc, ScreenPos);
        if (ScreenPos.X < MinPoint.X || ScreenPos.X > MaxPoint.X ||
            ScreenPos.Y < MinPoint.Y || ScreenPos.Y > MaxPoint.Y)
        {
            continue;
        }

        // 2차 필터: 시야 앞쪽에 있는가?
        FVector CamLoc = PlayerCameraManager->GetCameraLocation();
        FVector CamDir = (UnitLoc - CamLoc).GetSafeNormal();

        float Dot = FVector::DotProduct(CamDir, PlayerCameraManager->GetActorForwardVector());
        if (Dot < 0.5f) // 0.5~1 사이 값: 카메라가 유닛을 향하고 있을 때
        {
            continue; // 너무 측면 혹은 뒤쪽이면 무시
        }

        // 유닛 통과!
        Unit->SetSelected(true);
        SelectedUnits.Add(Unit);
    }

    bool bHasPlayerUnit = false;
    for (ARLCharacter* Unit : SelectedUnits)
    {
        if (Unit && Unit->GetTeamID() == 1)
        {
            bHasPlayerUnit = true;
            break;
        }
    }

    if (bHasPlayerUnit)
    {
        for (int32 i = SelectedUnits.Num() - 1; i >= 0; --i)
        {
            ARLCharacter* Unit = SelectedUnits[i];
            if (Unit && Unit->GetTeamID() != 1)
            {
                Unit->SetSelected(false);
                SelectedUnits.RemoveAt(i);
            }
        }
    }

    if (SelectedUnits.Num() == 1) {
        SelectedUnit = SelectedUnits[0];
    }

    UE_LOG(LogTemp, Warning, TEXT("%d units selected (filtered by depth)"), SelectedUnits.Num());
}

void ABattleController::DrawSelectionBoxOnGround()
{
    FVector2D Min(
        FMath::Min(DragStartScreenPos.X, DragEndScreenPos.X),
        FMath::Min(DragStartScreenPos.Y, DragEndScreenPos.Y)
    );
    FVector2D Max(
        FMath::Max(DragStartScreenPos.X, DragEndScreenPos.X),
        FMath::Max(DragStartScreenPos.Y, DragEndScreenPos.Y)
    );

    FVector WorldOrigin, WorldDir;
    FHitResult Hit;
    FVector Corners[4];

    auto ScreenToGround = [&](FVector2D ScreenPos, FVector& OutPoint)
        {
            DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldOrigin, WorldDir);
            FVector End = WorldOrigin + WorldDir * 100000.f;
            if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, End, ECC_Visibility))
            {
                OutPoint = Hit.Location;
            }
        };

    // 드래그 박스의 4개 꼭짓점 계산
    ScreenToGround(FVector2D(Min.X, Min.Y), Corners[0]); // 좌상
    ScreenToGround(FVector2D(Max.X, Min.Y), Corners[1]); // 우상
    ScreenToGround(FVector2D(Max.X, Max.Y), Corners[2]); // 우하
    ScreenToGround(FVector2D(Min.X, Max.Y), Corners[3]); // 좌하

    // 박스 라인 시각화
    DrawDebugLine(GetWorld(), Corners[0], Corners[1], FColor::Green, false, -1, 0, 2);
    DrawDebugLine(GetWorld(), Corners[1], Corners[2], FColor::Green, false, -1, 0, 2);
    DrawDebugLine(GetWorld(), Corners[2], Corners[3], FColor::Green, false, -1, 0, 2);
    DrawDebugLine(GetWorld(), Corners[3], Corners[0], FColor::Green, false, -1, 0, 2);
}

void ABattleController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsDragging)
    {
        //GetMousePosition(DragEndScreenPos.X, DragEndScreenPos.Y);
        //DrawSelectionBoxOnGround();
    }
}

void ABattleController::RemoveDeploymentPlanes()
{
    TArray<AActor*> DeploymentPlanes;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("DeploymentPlane"), DeploymentPlanes);

    for (AActor* Plane : DeploymentPlanes)
    {
        if (Plane)
        {
            Plane->Destroy();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Removed %d DeploymentPlane actors."), DeploymentPlanes.Num());
}

void ABattleController::NotifyCharacterDeath(ARLCharacter* DeadCharacter)
{
    if (!DeadCharacter) return;
    UE_LOG(LogTemp, Display, TEXT("NotifyCharacterDeath"));

    // 팀별 생존자 수 다시 계산
    int32 PlayerAliveCount = 0;
    int32 EnemyAliveCount = 0;

    for (TActorIterator<ARLCharacter> It(GetWorld()); It; ++It)
    {
        ARLCharacter* Unit = *It;
        if (!Unit || Unit->GetIsDead()) continue;

        if (Unit->GetTeamID() == 1)
        {
            PlayerAliveCount++;
        }
        else if (Unit->GetTeamID() == 2)
        {
            EnemyAliveCount++;
        }
            
    }
    UE_LOG(LogTemp, Display, TEXT("PlayerAliveCount:%d, EnemyAliveCount:%d"), PlayerAliveCount, EnemyAliveCount);
    if (PlayerAliveCount == 0 || EnemyAliveCount == 0)
    {
        EndBattle(PlayerAliveCount > 0);
    }
}

void ABattleController::EndBattle(bool bPlayerWin)
{
    UE_LOG(LogTemp, Display, TEXT("EndBattle"));
    IsBattleEnd = true;
    IsWin = bPlayerWin;
}