// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_Environment.h"

#include "RLCharacter.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "LearningAgentsManagerListener.h"

void ULA_Environment::GatherAgentReward_Implementation(
    float& OutReward, const int32 AgentId
)
{
    UObject* RewardActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ARLCharacter* RewardCharacter = Cast<ARLCharacter>(RewardActor);
    if (RewardCharacter)
    {
        // UE_LOG(LogTemp, Warning, TEXT("RewardActor: %s"), *RewardActor->GetFName().ToString());
        
        RewardCharacter->AddCurrentEpisodeStep(1);

        // 기본 reward
        // 적과의 거리 reward
        FVector MyLocation = RewardCharacter->GetActorLocation();
        const TArray<FVector>& EnemyLocations = RewardCharacter->GetEnemyLocation();
        if (EnemyLocations.Num() <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyLocations.Num() <= 0"));
            return;
        }
        // DistanceReward 계산방식 1
        float DistanceReward = ULearningAgentsRewards::MakeRewardFromLocationSimilarity(
            MyLocation, EnemyLocations[0], 1.0f, 1.0f);

        // DistanceReward 계산방식 2
        float MinDistance = 200.0f;
        float LocationScale = 10.0f;
        float RewardScale = 1.0f;
        const float LocationDifference = FVector::Dist(MyLocation, EnemyLocations[0]);
        float SimilarityShifted = FMath::InvExpApprox(
            FMath::Square((LocationDifference - MinDistance) / FMath::Max(LocationScale, UE_SMALL_NUMBER))
        );
        //if (LocationDifference <= MinDistance)
        //{
        //    SimilarityShifted = 1.0f;
        //}
        float DistanceReward_2 = SimilarityShifted * RewardScale;

        // 적 죽음 reward
        const TArray<ARLCharacter*>& Enemies = RewardCharacter->GetEnemyCharacters();
        if (Enemies.Num() <= 0)
        {
            return;
        }
        float EnemyDeadReward = ULearningAgentsRewards::MakeRewardOnCondition(
            Enemies[0]->GetIsDead(), 1.0f);
        if (EnemyDeadReward > 0.0f)
        {
            // UE_LOG(LogTemp, Warning, TEXT("EnemyDeadReward"));
        }
        // 적 타격 reward
        float HitReward = ULearningAgentsRewards::MakeRewardOnCondition(
            RewardCharacter->IsHit(), 0.3f);

        // custom 조절 reward
        float EnemyHealthReward = ULearningAgentsRewards::MakeReward(
            1 - Enemies[0]->GetHealth() / Enemies[0]->GetMaxHealth(), 0.3f
        );
        float MyHealthReward = ULearningAgentsRewards::MakeReward(
            RewardCharacter->GetHealth() / RewardCharacter->GetMaxHealth(), 0.3f
        );
        float StaminaReward = ULearningAgentsRewards::MakeReward(
            RewardCharacter->GetStamina() / RewardCharacter->GetMaxStamina(), 0.3f
        );

        float StaminaPenalty = ULearningAgentsRewards::MakeReward(
            RewardCharacter->GetStamina(), -0.0001);

        float MyDeadPenalty = ULearningAgentsRewards::MakeRewardOnCondition(
            RewardCharacter->GetIsDead(), -0.3f);

        float DistancePenalty = ULearningAgentsRewards::MakeRewardOnCondition(
            FVector::Dist(MyLocation, EnemyLocations[0]) > (RewardCharacter->GetMaxEnemyDistance() * 0.95f),
            -1.0f
        );

        float OutofArenaPenalty = 0.0f;
        if (RewardCharacter->IsArenaInitialized())
        {
            float DistFromArenaCenter = FVector::Dist2D(MyLocation, RewardCharacter->GetArenaCenter());

            OutofArenaPenalty = ULearningAgentsRewards::MakeRewardOnCondition(
                DistFromArenaCenter > RewardCharacter->GetArenaRadius() * 0.95f, -1.0f);
        }

        float StuckPenalty = ULearningAgentsRewards::MakeRewardOnCondition(
            RewardCharacter->GetStuckTimer() > RewardCharacter->GetMaxStuckTime(), -1.0f);

        FVector MyForward = RewardCharacter->GetActorForwardVector().GetSafeNormal();
        FVector ToEnemy = (EnemyLocations[0] - MyLocation).GetSafeNormal();

        float DirectionReward = ULearningAgentsRewards::MakeRewardFromDirectionSimilarity(
            MyForward, ToEnemy, 1.0f);
        // Gaussian-like 변환
        float k = 10.0f; // 민감도 (값이 클수록 정면일 때만 크게 보상)
        float GaussianDirectionReward = FMath::Exp(-k * FMath::Square(1.0f - DirectionReward));

        // UI Test
        /*OutReward = DistanceReward_2 + GaussianDirectionReward +
            DistancePenalty * RewardCharacter->GetRewardScale1() +
            OutofArenaPenalty * RewardCharacter->GetRewardScale2() +
            StuckPenalty * RewardCharacter->GetRewardScale3();*/

            //UE_LOG(LogTemp, Warning, TEXT("Name: %s, RS1:%f, RS2:%f, RS3:%f"), *RewardCharacter->GetFName().ToString(),
            //    RewardCharacter->GetRewardScale1(), RewardCharacter->GetRewardScale2(), RewardCharacter->GetRewardScale3());

            // Curriculum Step 1
            // OutReward = DistanceReward_2 + GaussianDirectionReward + DistancePenalty + OutofArenaPenalty + StuckPenalty;

        OutReward = DistanceReward_2 +
            GaussianDirectionReward +
            DistancePenalty +
            OutofArenaPenalty +
            StuckPenalty +
            EnemyDeadReward +
            MyDeadPenalty +
            HitReward +
            EnemyHealthReward * RewardCharacter->GetRewardScale1() +
            MyHealthReward * RewardCharacter->GetRewardScale2() +
            StaminaReward * RewardCharacter->GetRewardScale3();

        //OutReward = DistanceReward +
        //    EnemyDeadReward +
        //    HitReward +
        //    StaminaPenalty +
        //    MyDeadPenalty +
        //    DistancePenalty +
        //    DirectionReward;

        //OutReward = DistanceReward + 
        //            EnemyDeadReward + 
        //            HitReward + 
        //            EnemyHealthReward + 
        //            MyHealthReward + 
        //            StaminaReward;
    }
}

void ULA_Environment::GatherAgentCompletion_Implementation(
    ELearningAgentsCompletion& OutCompletion, const int32 AgentId
)
{
    UObject* CompletionActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ARLCharacter* CompletionCharacter = Cast<ARLCharacter>(CompletionActor);
    if (CompletionCharacter)
    {
        // UE_LOG(LogTemp, Warning, TEXT("CompletionCharacter: %s, Stamina: %d"), 
        //   *CompletionCharacter->GetFName().ToString(), CompletionCharacter->GetStamina());

        const TArray<ARLCharacter*>& Enemies = CompletionCharacter->GetEnemyCharacters();
        if (Enemies.Num() <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemies.Num() <= 0"));
            return;
        }

        FVector MyLocation = CompletionCharacter->GetActorLocation();
        const TArray<FVector>& EnemyLocations = CompletionCharacter->GetEnemyLocation();
        if (EnemyLocations.Num() <= 0)
        {
            return;
        }

        // Curriculum Step 1
        if (FVector::Distance(MyLocation, EnemyLocations[0]) > CompletionCharacter->GetMaxEnemyDistance())
        //if (Enemies[0]->GetIsDead() || CompletionCharacter->GetIsDead() ||
        //    FVector::Distance(MyLocation, EnemyLocations[0]) > CompletionCharacter->GetMaxEnemyDistance())
        {
            //UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion"), *CompletionCharacter->GetFName().ToString());
            if (!CompletionCharacter->GetIsCompletionReceiver()) {
                CompletionCharacter->SetIsCompletion(true);

                Enemies[0]->SetIsCompletion(true);
                Enemies[0]->SetIsCompletionReceiver(true);
            }
        }
        if (CompletionCharacter->IsArenaInitialized())
        {
            float DistFromArenaCenter = FVector::Dist2D(MyLocation, CompletionCharacter->GetArenaCenter());
            if (DistFromArenaCenter > CompletionCharacter->GetArenaRadius()) {
                if (!CompletionCharacter->GetIsCompletionReceiver()) {
                    CompletionCharacter->SetIsCompletion(true);

                    Enemies[0]->SetIsCompletion(true);
                    Enemies[0]->SetIsCompletionReceiver(true);
                }
            }

        }

        ELearningAgentsCompletion DeadCompletion =
            ULearningAgentsCompletions::MakeCompletionOnCondition(Enemies[0]->GetIsDead());
        if (DeadCompletion == ELearningAgentsCompletion::Termination)
        {
            //UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion Triggered: Enemy Dead"),
            //    *CompletionCharacter->GetFName().ToString());
        }

        bool IsStaminaOver =
            false;
            //CompletionCharacter->GetStamina() > CompletionCharacter->GetMaxStamina();
        ELearningAgentsCompletion StaminaCompletion =
            ULearningAgentsCompletions::MakeCompletionOnCondition(IsStaminaOver);

        
        ELearningAgentsCompletion DistanceCompletion =
            ULearningAgentsCompletions::MakeCompletionOnLocationDifferenceAboveThreshold(
                MyLocation, EnemyLocations[0], CompletionCharacter->GetMaxEnemyDistance());
        if (DistanceCompletion == ELearningAgentsCompletion::Termination)
        {
            UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion Triggered: Too Far"),
                *CompletionCharacter->GetFName().ToString());
        }
        //UE_LOG(LogTemp, Warning, TEXT("FName: %s (Distance: %f / Max: %f)"),
        //    *CompletionCharacter->GetFName().ToString(), 
        //    FVector::Distance(MyLocation, EnemyLocations[0]), 
        //    CompletionCharacter->GetMaxEnemyDistance());
        //FRotator Rot = CompletionCharacter->GetActorRotation();
        //UE_LOG(LogTemp, Log, TEXT("Rotation: Pitch=%f, Yaw=%f, Roll=%f"), Rot.Pitch, Rot.Yaw, Rot.Roll);
        
        ELearningAgentsCompletion MyDeadCompletion =
            ULearningAgentsCompletions::MakeCompletionOnCondition(CompletionCharacter->GetIsDead());
        if (MyDeadCompletion == ELearningAgentsCompletion::Termination)
        {
            //UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion Triggered: Player Dead"),
            //    *CompletionCharacter->GetFName().ToString());
        }

        ELearningAgentsCompletion ArenaCompletion = ELearningAgentsCompletion::Running;
        if (CompletionCharacter->IsArenaInitialized())
        {
            float DistFromArenaCenter = FVector::Dist2D(MyLocation, CompletionCharacter->GetArenaCenter());

            ArenaCompletion =
                ULearningAgentsCompletions::MakeCompletionOnCondition(
                    DistFromArenaCenter > CompletionCharacter->GetArenaRadius());
            if (ArenaCompletion == ELearningAgentsCompletion::Termination)
            {
                UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion Triggered: Out of Arena"),
                    *CompletionCharacter->GetName());
            }
        }

        ELearningAgentsCompletion StuckCompletion =
            ULearningAgentsCompletions::MakeCompletionOnCondition(
                CompletionCharacter->GetStuckTimer() > CompletionCharacter->GetMaxStuckTime());
        if (StuckCompletion == ELearningAgentsCompletion::Termination)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] Completion Triggered: Stuck"),
                *CompletionCharacter->GetName());
        }

        ELearningAgentsCompletion Completion =
            ULearningAgentsCompletions::MakeCompletionOnCondition(
                CompletionCharacter->GetIsCompletion() || Enemies[0]->GetIsCompletion());
        if (Completion == ELearningAgentsCompletion::Termination)
        {
            UE_LOG(LogTemp, Warning, TEXT("FName: %s, Completion Triggered"),
                *CompletionCharacter->GetFName().ToString());
        }
        // UE_LOG(LogTemp, Warning, TEXT("DeadCompletion: %s"), *StaticEnum<ELearningAgentsCompletion>()->GetNameStringByValue((int64)DeadCompletion));
        // UE_LOG(LogTemp, Warning, TEXT("StaminaCompletion: %s"), *StaticEnum<ELearningAgentsCompletion>()->GetNameStringByValue((int64)StaminaCompletion));
        // UE_LOG(LogTemp, Warning, TEXT("DistanceCompletion: %s"), *StaticEnum<ELearningAgentsCompletion>()->GetNameStringByValue((int64)DistanceCompletion));

        // Curriculum Step 1
        OutCompletion = ULearningAgentsCompletions::CompletionOr(
            ULearningAgentsCompletions::CompletionOr(
                ULearningAgentsCompletions::CompletionOr(
                    Completion, DistanceCompletion), ArenaCompletion), StuckCompletion);

        //OutCompletion =
        //    ULearningAgentsCompletions::CompletionOr(
        //        ULearningAgentsCompletions::CompletionOr(
        //            ULearningAgentsCompletions::CompletionOr(
        //                DeadCompletion, MyDeadCompletion), DistanceCompletion), Completion);
    }
}

void ULA_Environment::ResetAgentEpisode_Implementation(
    const int32 AgentId
)
{
    UObject* ResetActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ARLCharacter* ResetCharacter = Cast<ARLCharacter>(ResetActor);
    if (ResetCharacter)
    {
        // UE_LOG(LogTemp, Warning, TEXT("Reset !!!"));
        // UE_LOG(LogTemp, Warning, TEXT("FName: %s"), *ResetCharacter->GetFName().ToString());
        ResetCharacter->SetCurrentEpisodeStep(0);
        ResetCharacter->RLResetCharacter();
    }
}