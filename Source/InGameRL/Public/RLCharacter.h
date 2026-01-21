// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LearningAgentsManager.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/ConstraintInstance.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RLCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class INGAMERL_API ARLCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Selected, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SelectedCircle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Selected, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SelectedRing;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	/** Sprint Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* NumAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PlusMinusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotationAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USceneComponent* RightPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USceneComponent* LeftPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* RightHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* LeftHandle;

	UPROPERTY(VisibleAnywhere)
	UPhysicalAnimationComponent* PhysicalAnim;

	FConstraintInstance* RightConstraint;

	UBoxComponent* WeaponCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsHit = false;

	float HitDuration = 0.2f;

	FTimerHandle HitResetTimerHandle;

public:
	// Sets default values for this character's properties
	ARLCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShowDebugSphere();

	void ShowRightHandAngle();

	UFUNCTION(BlueprintCallable)
	void RLMove(FVector2D MovementVector);
	UFUNCTION(BlueprintCallable)
	void RLLook(FVector2D LookAxisVector);

	UFUNCTION(BlueprintCallable)
	void RLRightPointMove(FVector RightOffset);
	UFUNCTION(BlueprintCallable)
	void RLLeftPointMove(FVector LeftOffset);

	void RLResetCharacter();
	void ResetCharacterInTrainMap();

	UFUNCTION(BlueprintCallable)
	void ResetCharacterInTestBattleMap(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	void InitializeArena();

	// Getter, Setter
	const TArray<FVector>& GetEnemyLocation() const;
	const TArray<FVector>& GetEnemyDirection() const;
	const TArray<ARLCharacter*>& GetEnemyCharacters() const;

	USceneComponent* GetRightPoint() const;
	USceneComponent* GetLeftPoint() const;

	int32 GetMaxStamina() const;
	int32 GetStamina() const;
	void SetStamina(int32 NewStamina);

	bool GetIsDead() const;
	void SetIsDead(bool bDead);

	bool IsHit() const;
	void SetIsHit(bool bHit);

	float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintCallable)
	float GetHealth() const { return Health; }
	void SetHealth(float NewHealth) { Health = FMath::Clamp(NewHealth, 0.f, 100.f); }
	UFUNCTION(BlueprintCallable)
	float GetDamage() const { return Damage; }

	void SetIsCompletion(bool bCompletion) { IsCompletion = bCompletion; }
	bool GetIsCompletion() const { return IsCompletion; }

	void SetIsCompletionReceiver(bool bCompletionReceiver) { IsCompletionReceiver = bCompletionReceiver; }
	bool GetIsCompletionReceiver() const { return IsCompletionReceiver; }

	float GetMaxEnemyDistance() const { return MaxEnemyDistance; }

	float GetEHRScale() const { return EnemyHealthRewardScale; }
	float GetMHRScale() const { return MyHealthRewardScale; }
	float GetSRScale() const { return StaminaRewardScale; }

	void SetArenaInitialized(bool bInit) { bArenaInitialized = bInit; }
	bool IsArenaInitialized() const { return bArenaInitialized; }

	void SetArenaCenter(const FVector& Center) { ArenaCenter = Center; }
	FVector GetArenaCenter() const { return ArenaCenter; }

	void SetArenaRadius(float Radius) { ArenaRadius = Radius; }
	float GetArenaRadius() const { return ArenaRadius; }

	float GetStuckTimer() const { return StuckTimer; }
	float GetMaxStuckTime() const { return MaxStuckTime; }

	UFUNCTION(BlueprintCallable, Category = "RewardScale")
	void SetRewardScale(float r1, float r2, float r3) {	RewardSacle1 = r1; RewardSacle2 = r2; RewardSacle3 = r3; }
	float GetRewardScale1() const { return RewardSacle1; }
	float GetRewardScale2() const { return RewardSacle2; }
	float GetRewardScale3() const { return RewardSacle3; }

	void SetSelected(bool set) { bSelected = set; }

	UFUNCTION(BlueprintCallable, Category = "IL")
	FVector GetDeltaLoc() const { return DeltaLoc; }
	UFUNCTION(BlueprintCallable, Category = "IL")
	FRotator GetDeltaRot() const { return DeltaRot; }
	UFUNCTION(BlueprintCallable, Category = "IL")
	bool GetbHasPrev() const { return bHasPrev; }

	FTimerHandle GetArenaInitTimerHandle() const { return ArenaInitTimerHandle; }

	void SetAttackTarget(ARLCharacter* Target);

	int32 GetTeamID() const { return TeamID; }

	UFUNCTION(BlueprintCallable)
	void SetBattleStarted(bool Started) { bBattleStarted = Started; }

	int32 GetCurrentEpisodeStep() const { return CurrentEpisodeStep; }
	void AddCurrentEpisodeStep(int Add) { CurrentEpisodeStep += Add; }
	void SetCurrentEpisodeStep(int Step) { CurrentEpisodeStep = Step; }
protected:
	void ResetHitState();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void HandleRotationInput(const FInputActionValue& Value);

	void HandlePlusMinus(const FInputActionValue& Value);

	void HandleActorRotation(const FInputActionValue& Value);

	void StartSprint();
	void StopSprint();

	UFUNCTION()
	void OnMeshHit(
		UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit
	);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CalculateMaxRange();

	void InitSimulatePhysics();

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	TArray<ARLCharacter*> EnemyCharacters;
	TArray<FVector> EnemyLocation;
	TArray<FVector> EnemyDirection;

	void MakeEnemyInformation();
	void UpdateEnemyInfo();
	void InitPointHandle();

	void RegisterToLearningManager();
	void UnregisterFromLearningManager();
	void UpdateCombat();

	void OnDeath();

	FName hand_rSocket = TEXT("HandGrip_R");
	FName hand_r = TEXT("hand_r");
	FName lowerarm_r = TEXT("lowerarm_r");

	FName hand_lSocket = TEXT("HandGrip_L");
	FName hand_l = TEXT("hand_l");
	FName lowerarm_l = TEXT("lowerarm_l");

	FRotator RightRotator = FRotator::ZeroRotator;
	FRotator LeftRotator = FRotator::ZeroRotator;

	FVector PrevLocation = FVector::ZeroVector;
	FRotator PrevRotation = FRotator::ZeroRotator;
	FVector DeltaLoc = FVector::ZeroVector;
	FRotator DeltaRot = FRotator::ZeroRotator;
	bool bHasPrev = false;

	float DefaultWalkSpeed = 250.f;
	float SprintSpeed = 500.f;  // 달리기 속도
	float MoveAmount = 1.f;
	float MaxRange = 1.f;
	float Damage = 10.f;
	float ResetDistance = 600.f; // 리셋 후 랜덤으로 배치할 때 캐릭터들 간의 거리
	float MaxRadius = 400.f; // 훈련장 중심에서 캐릭터가 리셋되는 범위
	float MaxEnemyDistance = 800.f; // 도망갔다고 판단하는 거리
	float ArenaMaxOffset = 400.f; 
	
	bool bArenaInitialized = false;
	FVector ArenaCenter;
	float ArenaRadius = 800.0f; // 가상의 경기장 반지름
	FTimerHandle ArenaInitTimerHandle;
	FTimerHandle ResetTimerHandle;
	EMovementMode Mode;

	// Stuck 판정용
	FVector LastLocation;
	float StuckTimer = 0.0f;
	float StuckThreshold = 0.1f;   // 위치 변화 최소 거리 (유닛)
	float MaxStuckTime = 10.0f;      // 최대 허용 시간 (초)
	

	float EnemyHealthRewardScale = 1.0f;
	float MyHealthRewardScale = 1.0f;
	float StaminaRewardScale = 1.0f;

	float RewardSacle1 = 1.0f;
	float RewardSacle2 = 1.0f;
	float RewardSacle3 = 1.0f;

	int32 CurrentEpisodeStep = 0;

	bool bSelected = false;
	bool bBattleStarted = false;

	ULearningAgentsManager* Manager;
	int32 AgentId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = TeamNumber)
	int32 TeamID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = ManagerTag)
	FName ManagerTag;
	bool FoundManager = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = OriginTag)
	FName OriginTag;
	FVector OriginLocation = FVector::ZeroVector;
	FVector InitialLocation;
	FRotator InitialRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsTraining = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsTestBattle = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsDead = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 Stamina = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 MaxStamina = 5000;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsCompletion = false;
	bool IsCompletionReceiver = false;

	ARLCharacter* AttackTarget = NULL;

	// 얘네들 c++에서 생성하려면 에디터 완전히 닫고 컴파일, 빌드 해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> HandRight;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> HandLeft;

};
