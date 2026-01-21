// Fill out your copyright notice in the Description page of Project Settings.


#include "TrainingActor.h"
#include "Engine/World.h"
#include "Misc/ScopeLock.h"
#include "TrainingGameInstance.h"

// Sets default values
ATrainingActor::ATrainingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATrainingActor::BeginPlay()
{
	Super::BeginPlay();
	
    if (UTrainingGameInstance* GI = Cast<UTrainingGameInstance>(GetGameInstance()))
    {
        SetListenerRunnable(GI->GetListenerRunnable());
        UE_LOG(LogTemp, Warning, TEXT("found Runnable"));

        RewardWeight1 = GI->GetRewardWeight1();
        RewardWeight2 = GI->GetRewardWeight2();
        RewardWeight3 = GI->GetRewardWeight3();
        TrainingCount = GI->GetTrainingCount();
    }
}

void ATrainingActor::SetListenerRunnable(TrainingListenerRunnable* InRunnable)
{
	ListenerRunnable = InRunnable;
}

void ATrainingActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ListenerRunnable)
        return;

    if (!bInitDone)
    {
        bInitDone = true;
        SetActorRewardWeight();
    }

    FString Msg;
    // Runnable → Actor 메시지 확인
    while (ListenerRunnable->IncomingQueue.Dequeue(Msg))
    {
        if (Msg.Contains(TEXT("GET_ACTOR_INFO")))
        {
            FString ActorInfo = GetActorInfo();
            ListenerRunnable->OutgoingQueue.Enqueue(ActorInfo);
        }
        else
        {
            // 기본 에코
            FString Echo = FString::Printf(TEXT("ECHO: %s"), *Msg);
            ListenerRunnable->OutgoingQueue.Enqueue(Echo);
        }
    }
}

void ATrainingActor::SetActorRewardWeight()
{
    FString Result;

    if (!GetWorld() || GetWorld()->bIsTearingDown)
    {
        UE_LOG(LogTemp, Warning, TEXT("No world."));
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARLCharacter::StaticClass(), FoundActors);

    if (FoundActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No RLCharacter actors found in world."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("SetActorRewardWeight"));
    for (AActor* Actor : FoundActors)
    {
        ARLCharacter* RLChar = Cast<ARLCharacter>(Actor);
        if (RLChar)
        {
            RLChar->SetRewardScale(RewardWeight1, RewardWeight2, RewardWeight3);
        }
    }
}

FString ATrainingActor::GetActorInfo() const
{
    FString Result;

    if (!GetWorld() || GetWorld()->bIsTearingDown)
    {
        return TEXT("World not found");
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARLCharacter::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        ARLCharacter* Agent = Cast<ARLCharacter>(FoundActors[0]);
        if (Agent)
        {
            Result = FString::Printf(TEXT("%s, %d | RewardScale1: %f | RewardScale2: %f | RewardScale3: %f"),
                *Agent->GetName(),
                TrainingCount,
                Agent->GetRewardScale1(),
                Agent->GetRewardScale2(),
                Agent->GetRewardScale3()
            );
        }
        else
        {
            Result = TEXT("FIRST ACTOR INVALID");
        }
    }
    else
    {
        Result = TEXT("NO ACTORS FOUND");
    }

    return Result;
}