// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_Interactor.h"

#include "LearningAgentsObservations.h"
#include "LearningAgentsManagerListener.h"
#include "LearningAgentsActions.h"
#include "RLCharacter.h"

void ULA_Interactor::SpecifyAgentObservation_Implementation(
    FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
    ULearningAgentsObservationSchema* InObservationSchema
)
{
    TMap<FName, FLearningAgentsObservationSchemaElement> Map;
    TMap<FName, FLearningAgentsObservationSchemaElement> EnemyMap;
    TMap<FName, FLearningAgentsObservationSchemaElement> ArmPointMap;

    // Specify Enemy Map
    FLearningAgentsObservationSchemaElement EnemyLocation =
        ULearningAgentsObservations::SpecifyLocationObservation(
            InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement EnemyDirection =
        ULearningAgentsObservations::SpecifyDirectionObservation(
            InObservationSchema);

    EnemyMap.Add(TEXT("Location"), EnemyLocation);
    EnemyMap.Add(TEXT("Direction"), EnemyDirection);

    FLearningAgentsObservationSchemaElement EnemyStruct =
        ULearningAgentsObservations::SpecifyStructObservation(
            InObservationSchema, EnemyMap);

    FLearningAgentsObservationSchemaElement EnemyArray =
        ULearningAgentsObservations::SpecifyArrayObservation(
            InObservationSchema, EnemyStruct, MaxEnemyArrayNum
        );

    // Specify Arm Point Map
    FLearningAgentsObservationSchemaElement RightLocation =
        ULearningAgentsObservations::SpecifyLocationObservation(
            InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement RightRotation =
        ULearningAgentsObservations::SpecifyRotationObservation(
            InObservationSchema);

    FLearningAgentsObservationSchemaElement LeftLocation =
        ULearningAgentsObservations::SpecifyLocationObservation(
            InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement LeftRotation =
        ULearningAgentsObservations::SpecifyRotationObservation(
            InObservationSchema);

    ArmPointMap.Add(TEXT("RLocation"), RightLocation);
    ArmPointMap.Add(TEXT("RRotation"), RightRotation);
    ArmPointMap.Add(TEXT("LLocation"), LeftLocation);
    ArmPointMap.Add(TEXT("LRotation"), LeftRotation);

    FLearningAgentsObservationSchemaElement ArmPointStruct =
        ULearningAgentsObservations::SpecifyStructObservation(
            InObservationSchema, ArmPointMap);

    // Specify Map
    FLearningAgentsObservationSchemaElement Location =
        ULearningAgentsObservations::SpecifyLocationObservation(
            InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement Direction =
        ULearningAgentsObservations::SpecifyDirectionObservation(
            InObservationSchema);

    Map.Add(TEXT("MyLocation"), Location);
    Map.Add(TEXT("MyDirection"), Direction);
    Map.Add(TEXT("Enemy"), EnemyArray);
    Map.Add(TEXT("ArmPoint"), ArmPointStruct);

    OutObservationSchemaElement =
        ULearningAgentsObservations::SpecifyStructObservation(
            InObservationSchema, Map);
}

void ULA_Interactor::GatherAgentObservation_Implementation(
    FLearningAgentsObservationObjectElement& OutObservationObjectElement,
    ULearningAgentsObservationObject* InObservationObject,
    const int32 AgentId
)
{
    TMap<FName, FLearningAgentsObservationObjectElement> Map;
    TMap<FName, FLearningAgentsObservationObjectElement> EnemyMap;
    TMap<FName, FLearningAgentsObservationObjectElement> ArmPointMap;

    UObject* ObsActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ARLCharacter* ObsCharacter = Cast<ARLCharacter>(ObsActor);
    if (ObsCharacter)
    {
        // Gather Enemy Map
        FTransform Transform = ObsCharacter->GetActorTransform();

        TArray<FLearningAgentsObservationObjectElement> EnemyElement;
        EnemyElement.Empty();

        const TArray<FVector>& Locations = ObsCharacter->GetEnemyLocation();
        const TArray<FVector>& Directions = ObsCharacter->GetEnemyDirection();
        int32 Count = FMath::Min(Locations.Num(), Directions.Num());

        for (int32 i = 0; i < Count; ++i)
        {
            const FVector& Location = Locations[i];
            const FVector& Direction = Directions[i];

            FLearningAgentsObservationObjectElement EnemyLocation =
                ULearningAgentsObservations::MakeLocationObservation(
                    InObservationObject, Location, Transform);

            FLearningAgentsObservationObjectElement EnemyDirection =
                ULearningAgentsObservations::MakeDirectionObservation(
                    InObservationObject, Direction, Transform);

            EnemyMap.Add(TEXT("Location"), EnemyLocation);
            EnemyMap.Add(TEXT("Direction"), EnemyDirection);

            FLearningAgentsObservationObjectElement EnemyStruct =
                ULearningAgentsObservations::MakeStructObservation(
                    InObservationObject, EnemyMap);

            EnemyElement.Add(EnemyStruct);
        }

        FLearningAgentsObservationObjectElement EnemyArray =
            ULearningAgentsObservations::MakeArrayObservation(
                InObservationObject, EnemyElement, MaxEnemyArrayNum
            );

        // Gather Arm Point Map
        FRotator Rotation = ObsCharacter->GetActorRotation();

        FVector RLocation = ObsCharacter->GetRightPoint()->GetComponentLocation();
        FRotator RRotation = ObsCharacter->GetRightPoint()->GetComponentRotation();
        FVector LLocation = ObsCharacter->GetLeftPoint()->GetComponentLocation();
        FRotator LRotation = ObsCharacter->GetLeftPoint()->GetComponentRotation();

        FLearningAgentsObservationObjectElement RightLocation =
            ULearningAgentsObservations::MakeLocationObservation(
                InObservationObject, RLocation, Transform
            );
        FLearningAgentsObservationObjectElement RightRotation =
            ULearningAgentsObservations::MakeRotationObservation(
                InObservationObject, RRotation, Rotation
            );
        FLearningAgentsObservationObjectElement LeftLocation =
            ULearningAgentsObservations::MakeLocationObservation(
                InObservationObject, LLocation, Transform
            );
        FLearningAgentsObservationObjectElement LeftRotation =
            ULearningAgentsObservations::MakeRotationObservation(
                InObservationObject, LRotation, Rotation
            );

        ArmPointMap.Add(TEXT("RLocation"), RightLocation);
        ArmPointMap.Add(TEXT("RRotation"), RightRotation);
        ArmPointMap.Add(TEXT("LLocation"), LeftLocation);
        ArmPointMap.Add(TEXT("LRotation"), LeftRotation);

        FLearningAgentsObservationObjectElement ArmPointStruct =
            ULearningAgentsObservations::MakeStructObservation(
                InObservationObject, ArmPointMap);

        // Gather Map
        FLearningAgentsObservationObjectElement Location =
            ULearningAgentsObservations::MakeLocationObservation(
                InObservationObject, ObsCharacter->GetActorLocation(), Transform);
        FLearningAgentsObservationObjectElement Direction =
            ULearningAgentsObservations::MakeDirectionObservation(
                InObservationObject, ObsCharacter->GetActorForwardVector(), Transform);

        Map.Add(TEXT("MyLocation"), Location);
        Map.Add(TEXT("MyDirection"), Direction);
        Map.Add(TEXT("Enemy"), EnemyArray);
        Map.Add(TEXT("ArmPoint"), ArmPointStruct);

        OutObservationObjectElement =
            ULearningAgentsObservations::MakeStructObservation(
                InObservationObject, Map);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cast to ARLCharacter failed!"));
    }
}

FLearningAgentsActionSchemaElement ULA_Interactor::MakeStructAction3Location3Rotation(
    ULearningAgentsActionSchema* InActionSchema
)
{
    TMap<FName, FLearningAgentsActionSchemaElement> ActionMap;

    FLearningAgentsActionSchemaElement LocationX =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement LocationY =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement LocationZ =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});

    FLearningAgentsActionSchemaElement RotationX =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement RotationY =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement RotationZ =
        ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
            InActionSchema, DiscreteActionSize, {});

    ActionMap.Add(TEXT("LocationX"), LocationX);
    ActionMap.Add(TEXT("LocationY"), LocationY);
    ActionMap.Add(TEXT("LocationZ"), LocationZ);
    ActionMap.Add(TEXT("RotationX"), RotationX);
    ActionMap.Add(TEXT("RotationY"), RotationY);
    ActionMap.Add(TEXT("RotationZ"), RotationZ);

    return ULearningAgentsActions::SpecifyStructAction(InActionSchema, ActionMap);
}

void ULA_Interactor::SpecifyAgentAction_Implementation(
    FLearningAgentsActionSchemaElement& OutActionSchemaElement,
    ULearningAgentsActionSchema* InActionSchema
)
{
    TMap<FName, FLearningAgentsActionSchemaElement> Map;

    // Specify Movement
    TMap<FName, FLearningAgentsActionSchemaElement> MovementMap;

    FLearningAgentsActionSchemaElement XMovement =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 3.0f);
    FLearningAgentsActionSchemaElement YMovement =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 3.0f);

    MovementMap.Add(TEXT("X"), XMovement);
    MovementMap.Add(TEXT("Y"), YMovement);

    FLearningAgentsActionSchemaElement MovementStruct =
        ULearningAgentsActions::SpecifyStructAction(
            InActionSchema, MovementMap);

    // Specify Rotation
    FLearningAgentsActionSchemaElement Rotation =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 3.0f);

    // Specify Right, Left Action
    FLearningAgentsActionSchemaElement RightStruct =
        MakeStructAction3Location3Rotation(InActionSchema);
    FLearningAgentsActionSchemaElement LeftStruct =
        MakeStructAction3Location3Rotation(InActionSchema);

    // Sepcify LocatoinRotation Amount
    TMap<FName, FLearningAgentsActionSchemaElement> AmountMap;

    FLearningAgentsActionSchemaElement RightXLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);
    FLearningAgentsActionSchemaElement RightYLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);
    FLearningAgentsActionSchemaElement RightZLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);

    FLearningAgentsActionSchemaElement RightXRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);
    FLearningAgentsActionSchemaElement RightYRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);
    FLearningAgentsActionSchemaElement RightZRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);

    FLearningAgentsActionSchemaElement LeftXLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);
    FLearningAgentsActionSchemaElement LeftYLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);
    FLearningAgentsActionSchemaElement LeftZLocationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, LocationAmount);

    FLearningAgentsActionSchemaElement LeftXRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);
    FLearningAgentsActionSchemaElement LeftYRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);
    FLearningAgentsActionSchemaElement LeftZRotationAmount =
        ULearningAgentsActions::SpecifyFloatAction(InActionSchema, RotationAmount);

    AmountMap.Add(TEXT("RXL"), RightXLocationAmount);
    AmountMap.Add(TEXT("RYL"), RightYLocationAmount);
    AmountMap.Add(TEXT("RZL"), RightZLocationAmount);

    AmountMap.Add(TEXT("RXR"), RightXRotationAmount);
    AmountMap.Add(TEXT("RYR"), RightYRotationAmount);
    AmountMap.Add(TEXT("RZR"), RightZRotationAmount);

    AmountMap.Add(TEXT("LXL"), LeftXLocationAmount);
    AmountMap.Add(TEXT("LYL"), LeftYLocationAmount);
    AmountMap.Add(TEXT("LZL"), LeftZLocationAmount);

    AmountMap.Add(TEXT("LXR"), LeftXRotationAmount);
    AmountMap.Add(TEXT("LYR"), LeftYRotationAmount);
    AmountMap.Add(TEXT("LZR"), LeftZRotationAmount);

    FLearningAgentsActionSchemaElement AmountStruct =
        ULearningAgentsActions::SpecifyStructAction(
            InActionSchema, AmountMap);

    // Specify Map
    Map.Add(TEXT("Movement"), MovementStruct);
    Map.Add(TEXT("Rotation"), Rotation);
    Map.Add(TEXT("Right"), RightStruct);
    Map.Add(TEXT("Left"), LeftStruct);
    Map.Add(TEXT("Amount"), AmountStruct);

    OutActionSchemaElement =
        ULearningAgentsActions::SpecifyStructAction(
            InActionSchema, Map);

}

void ULA_Interactor::PerformAgentAction_Implementation(
    const ULearningAgentsActionObject* InActionObject,
    const FLearningAgentsActionObjectElement& InActionObjectElement,
    const int32 AgentId
)
{
    UObject* ActActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ARLCharacter* ActCharacter = Cast<ARLCharacter>(ActActor);
    if (ActCharacter)
    {
        TMap<FName, FLearningAgentsActionObjectElement> OutActions;

        ULearningAgentsActions::GetStructAction(OutActions, InActionObject, InActionObjectElement);

        // Perform Movement
        TMap<FName, FLearningAgentsActionObjectElement> MovementActions;
        ULearningAgentsActions::GetStructAction(
            MovementActions, InActionObject, *OutActions.Find(TEXT("Movement")));

        float XMovement;
        ULearningAgentsActions::GetFloatAction(
            XMovement, InActionObject, *MovementActions.Find(TEXT("X"))
        );
        ActCharacter->RLMove(FVector2D(XMovement, 0.0f));

        float YMovement;
        ULearningAgentsActions::GetFloatAction(
            YMovement, InActionObject, *MovementActions.Find(TEXT("Y"))
        );
        ActCharacter->RLMove(FVector2D(0.0f, YMovement));

        // Perform Rotation
        float Rotation;
        ULearningAgentsActions::GetFloatAction(
            Rotation, InActionObject, *OutActions.Find(TEXT("Rotation"))
        );
        //UE_LOG(LogTemp, Warning, TEXT("Rotation: %f"), Rotation);
        ActCharacter->AddActorLocalRotation(FRotator(0, Rotation, 0));
        //ActCharacter->RLLook(FVector2D(Rotation, 0.0f));

        // Perform Amount
        TMap<FName, FLearningAgentsActionObjectElement> Amount;
        ULearningAgentsActions::GetStructAction(
            Amount, InActionObject, *OutActions.Find(TEXT("Amount")));
        float RightXLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightXLocationAmount, InActionObject, *Amount.Find(TEXT("RXL"))
        );
        float RightYLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightYLocationAmount, InActionObject, *Amount.Find(TEXT("RYL"))
        );
        float RightZLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightZLocationAmount, InActionObject, *Amount.Find(TEXT("RZL"))
        );
        float RightXRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightXRotationAmount, InActionObject, *Amount.Find(TEXT("RXR"))
        );
        float RightYRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightYRotationAmount, InActionObject, *Amount.Find(TEXT("RYR"))
        );
        float RightZRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            RightZRotationAmount, InActionObject, *Amount.Find(TEXT("RZR"))
        );

        float LeftXLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftXLocationAmount, InActionObject, *Amount.Find(TEXT("LXL"))
        );
        float LeftYLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftYLocationAmount, InActionObject, *Amount.Find(TEXT("LYL"))
        );
        float LeftZLocationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftZLocationAmount, InActionObject, *Amount.Find(TEXT("LZL"))
        );
        float LeftXRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftXRotationAmount, InActionObject, *Amount.Find(TEXT("LXR"))
        );
        float LeftYRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftYRotationAmount, InActionObject, *Amount.Find(TEXT("LYR"))
        );
        float LeftZRotationAmount;
        ULearningAgentsActions::GetFloatAction(
            LeftZRotationAmount, InActionObject, *Amount.Find(TEXT("LZR"))
        );

        // Perform Right
        TMap<FName, FLearningAgentsActionObjectElement> Right;
        ULearningAgentsActions::GetStructAction(
            Right, InActionObject, *OutActions.Find(TEXT("Right")));

        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationX"),
            ActCharacter, FVector(1, 0, 0), RightXLocationAmount, &ARLCharacter::RLRightPointMove);
        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationY"),
            ActCharacter, FVector(0, 1, 0), RightYLocationAmount, &ARLCharacter::RLRightPointMove);
        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationZ"),
            ActCharacter, FVector(0, 0, 1), RightZLocationAmount, &ARLCharacter::RLRightPointMove);

        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationX"),
            ActCharacter, FRotator(1, 0, 0), RightXRotationAmount, &ARLCharacter::GetRightPoint);
        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationY"),
            ActCharacter, FRotator(0, 1, 0), RightYRotationAmount, &ARLCharacter::GetRightPoint);
        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationZ"),
            ActCharacter, FRotator(0, 0, 1), RightZRotationAmount, &ARLCharacter::GetRightPoint);

        // Perform Left
        TMap<FName, FLearningAgentsActionObjectElement> Left;
        ULearningAgentsActions::GetStructAction(
            Left, InActionObject, *OutActions.Find(TEXT("Left")));

        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationX"),
            ActCharacter, FVector(1, 0, 0), LeftXLocationAmount, &ARLCharacter::RLLeftPointMove);
        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationY"),
            ActCharacter, FVector(0, 1, 0), LeftYLocationAmount, &ARLCharacter::RLLeftPointMove);
        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationZ"),
            ActCharacter, FVector(0, 0, 1), LeftZLocationAmount, &ARLCharacter::RLLeftPointMove);

        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationX"),
            ActCharacter, FRotator(1, 0, 0), LeftXRotationAmount, &ARLCharacter::GetLeftPoint);
        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationY"),
            ActCharacter, FRotator(0, 1, 0), LeftYRotationAmount, &ARLCharacter::GetLeftPoint);
        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationZ"),
            ActCharacter, FRotator(0, 0, 1), LeftZRotationAmount, &ARLCharacter::GetLeftPoint);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cast to ARLCharacter failed!"));
    }
}

// 원본 코드
// int32 LocationX;
// ULearningAgentsActions::GetExclusiveDiscreteAction(
//     LocationX, InActionObject, *Right.Find(TEXT("LocationX")));
// LocationX = (LocationX - 1) * LocationAmount;
// ActCharacter->RLRightPointMove(FVector((float)LocationX, 0.0f, 0.0f));

void ULA_Interactor::ApplyDiscreteActionMove(
    const ULearningAgentsActionObject* InActionObject,
    const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
    const FName& KeyName,
    ARLCharacter* ActCharacter,
    const FVector& MoveAxis,
    float Amount,
    void (ARLCharacter::* MoveFunc)(FVector)
)
{
    int32 LocationIndex;
    if (ULearningAgentsActions::GetExclusiveDiscreteAction(LocationIndex, InActionObject, *ActionMap.Find(KeyName)))
    {
        int32 AdjustedLocation = (LocationIndex - 1) * Amount;
        FVector MoveVector = MoveAxis * (float)AdjustedLocation;
        (ActCharacter->*MoveFunc)(MoveVector);

        if (AdjustedLocation != 0)
        {
            ActCharacter->SetStamina(ActCharacter->GetStamina() + 1);
        }
    }
}

void ULA_Interactor::ApplyDiscreteActionRotate(
    const ULearningAgentsActionObject* InActionObject,
    const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
    const FName& KeyName,
    ARLCharacter* ActCharacter,
    const FRotator& RotateAxis,
    float Amount,
    USceneComponent* (ARLCharacter::* GetPointFunc)() const
)
{
    int32 RotationIndex;
    if (ULearningAgentsActions::GetExclusiveDiscreteAction(RotationIndex, InActionObject, *ActionMap.Find(KeyName)))
    {
        int32 AdjustedRotation = (RotationIndex - 1) * Amount;
        FRotator RotateVector = RotateAxis * (float)AdjustedRotation;

        USceneComponent* TargetComponent = (ActCharacter->*GetPointFunc)();
        if (TargetComponent)
        {
            TargetComponent->AddLocalRotation(RotateVector);

            if (AdjustedRotation != 0)
            {
                ActCharacter->SetStamina(ActCharacter->GetStamina() + 1);
            }
        }
    }
}
