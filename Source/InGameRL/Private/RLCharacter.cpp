// Fill out your copyright notice in the Description page of Project Settings.


#include "RLCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/PoseableMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Weapon.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "LearningAgentsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "RLCharAIController.h"
#include "BattleController.h"

// Sets default values
ARLCharacter::ARLCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(16.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	//GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->bOrientRotationToMovement = true; // aicontroller 테스트를 위해 일단 true
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SelectedCircle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedCircle"));
	SelectedCircle->SetupAttachment(RootComponent);
	SelectedRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedRing"));
	SelectedRing->SetupAttachment(SelectedCircle);

	RightPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightPoint"));
	RightPoint->SetupAttachment(GetMesh());
	LeftPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftPoint"));
	LeftPoint->SetupAttachment(GetMesh());

	RightHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("RightHandle"));
	LeftHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("LeftHandle"));
	PhysicalAnim = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnim"));

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, 270.f, 0.f));

	// AI를 자동으로 붙이기
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// RLUnitAIController를 AI 컨트롤러로 지정
	AIControllerClass = ARLCharAIController::StaticClass();

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARLCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ARLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARLCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARLCharacter::Look);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ARLCharacter::Look);
		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARLCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARLCharacter::StopSprint);

		// Number
		EnhancedInputComponent->BindAction(NumAction, ETriggerEvent::Triggered, this, &ARLCharacter::HandleRotationInput);
		// +, -
		EnhancedInputComponent->BindAction(PlusMinusAction, ETriggerEvent::Started, this, &ARLCharacter::HandlePlusMinus);
		// Rotation
		EnhancedInputComponent->BindAction(RotationAction, ETriggerEvent::Triggered, this, &ARLCharacter::HandleActorRotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARLCharacter::RLMove(FVector2D MovementVector)
{
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARLCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		//const FRotator Rotation = Controller->GetControlRotation();
		//const FRotator YawRotation(0, Rotation.Yaw, 0);

		//const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		//const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector ForwardDirection = GetActorForwardVector();
		const FVector RightDirection = GetActorRightVector();

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARLCharacter::RLLook(FVector2D LookAxisVector)
{
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARLCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARLCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;  // 이동 속도를 SprintSpeed(800.f)로 변경
}

void ARLCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;  // 다시 원래 속도(500.f)로 복귀
}

void ARLCharacter::HandleRotationInput(const FInputActionValue& Value)
{
	// 입력 값 가져오기
	float InputValue = Value.Get<float>();

	// 입력이 0이면 (누르지 않은 상태) 무시
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	// 문자열을 숫자로 변환하는 맵
	static TMap<FString, int32> KeyMap = {
		{ "One", 1 }, { "Two", 2 }, { "Three", 3 },
		{ "Four", 4 }, { "Five", 5 }, { "Six", 6 },
		{ "Seven", 7 }, { "Eight", 8 }, { "Nine", 9 }
	};

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// NumAction에 매핑된 모든 키 가져오기
			TArray<FKey> BoundKeys = InputSubsystem->QueryKeysMappedToAction(NumAction);

			for (const FKey& Key : BoundKeys)
			{
				if (PC->IsInputKeyDown(Key))
				{
					FString KeyName = Key.ToString();
					if (KeyMap.Contains(KeyName))
					{
						int32 KeyIndex = KeyMap[KeyName];

						FVector Origin = GetMesh()->GetSocketLocation("neck_01");
						FVector RightOffset;
						FVector LeftOffset;

						bool bApplyRightOffset = false;
						bool bApplyLeftOffset = false;

						switch (KeyIndex)
						{
						case 1: RightOffset = FVector(MoveAmount, 0, 0); bApplyRightOffset = true; break;
						case 2: RightOffset = FVector(0, MoveAmount, 0); bApplyRightOffset = true; break;
						case 3: RightOffset = FVector(0, 0, MoveAmount); bApplyRightOffset = true; break;

						case 4: LeftOffset = FVector(MoveAmount, 0, 0); bApplyLeftOffset = true; break;
						case 5: LeftOffset = FVector(0, MoveAmount, 0); bApplyLeftOffset = true; break;
						case 6: LeftOffset = FVector(0, 0, MoveAmount); bApplyLeftOffset = true; break;

						case 7: //UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.25f);
							AddActorLocalRotation(FRotator(0, MoveAmount, 0)); break;
							//RightPoint->AddLocalRotation(FRotator(MoveAmount, 0, 0)); break;
						case 8: //UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f); 
							RightPoint->AddLocalRotation(FRotator(0, MoveAmount, 0)); break;
						case 9: //UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 4.0f); 
							RightPoint->AddLocalRotation(FRotator(0, 0, MoveAmount)); break;

						default:
							UE_LOG(LogTemp, Warning, TEXT("Invalid Key: %s"), *KeyName);
							break;
						}

						if (bApplyRightOffset)
						{
							FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(RightOffset);
							float NewDistance = FVector::Dist(Origin, NewLocation);
							if (NewDistance <= MaxRange)
							{
								RightPoint->AddLocalOffset(RightOffset);
							}
						}

						if (bApplyLeftOffset)
						{
							FVector NewLocation = LeftPoint->GetComponentLocation() + LeftPoint->GetComponentTransform().TransformVector(LeftOffset);
							float NewDistance = FVector::Dist(Origin, NewLocation);
							if (NewDistance <= MaxRange)
							{
								LeftPoint->AddLocalOffset(LeftOffset);
							}
						}
					}
				}
			}
		}
	}
}

void ARLCharacter::HandlePlusMinus(const FInputActionValue& Value) {
	float InputValue = Value.Get<float>();
	UE_LOG(LogTemp, Log, TEXT("MoveAmount: %f"), MoveAmount);

	MoveAmount = -MoveAmount;
}

void ARLCharacter::HandleActorRotation(const FInputActionValue& Value) {
	// input is a Vector1D
	float RotationVector = Value.Get<float>();
	UE_LOG(LogTemp, Log, TEXT("HandleActorRotation"));
	AddActorLocalRotation(FRotator(0, RotationVector, 0));
}

void ARLCharacter::RLRightPointMove(FVector RightOffset) {
	// FVector Direction(
	//     FMath::Sign(RightOffset.X),
	//     FMath::Sign(RightOffset.Y),
	//     FMath::Sign(RightOffset.Z)
	// );

	// FVector Origin = GetMesh()->GetSocketLocation("neck_01");
	// FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(Direction);
	// float NewDistance = FVector::Dist(Origin, NewLocation);

	// if (NewDistance <= MaxRange)
	// {
	//     RightPoint->AddLocalOffset(Direction);
	// }

	FVector Origin = GetMesh()->GetSocketLocation("neck_01");

	FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(RightOffset);
	float NewDistance = FVector::Dist(Origin, NewLocation);
	if (NewDistance <= MaxRange)
	{
		RightPoint->AddLocalOffset(RightOffset);
	}
}

void ARLCharacter::RLLeftPointMove(FVector LeftOffset) {
	FVector Origin = GetMesh()->GetSocketLocation("neck_01");

	FVector NewLocation = LeftPoint->GetComponentLocation() + LeftPoint->GetComponentTransform().TransformVector(LeftOffset);
	float NewDistance = FVector::Dist(Origin, NewLocation);
	if (NewDistance <= MaxRange)
	{
		LeftPoint->AddLocalOffset(LeftOffset);
	}
}

void ARLCharacter::MakeEnemyInformation()
{
	EnemyCharacters.Empty();
	EnemyLocation.Empty();
	EnemyDirection.Empty();

	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), ARLCharacter::StaticClass(), EnemyActors);

	struct FEnemyInfo
	{
		ARLCharacter* EnemyChar;
		FVector Location;
		FVector Direction;
		float Distance;

		FEnemyInfo(ARLCharacter* InChar, const FVector& InLoc, const FVector& InDir, float InDist)
			: EnemyChar(InChar), Location(InLoc), Direction(InDir), Distance(InDist) {
		}
	};

	TArray<FEnemyInfo> EnemyInfoList;

	for (AActor* Actor : EnemyActors)
	{
		ARLCharacter* OtherChar = Cast<ARLCharacter>(Actor);
		if (OtherChar && OtherChar != this && OtherChar->TeamID != this->TeamID)
		{
			if (bBattleStarted && !IsTraining && !IsTestBattle)
			{
				if (OtherChar->GetIsDead())
					continue;
			}

			FVector Location = OtherChar->GetActorLocation();
			float Distance = FVector::Dist(GetActorLocation(), Location);

			EnemyInfoList.Add(FEnemyInfo(OtherChar, Location, OtherChar->GetActorForwardVector(), Distance));
		}
	}

	// 거리 기준으로 정렬 (오름차순)
	EnemyInfoList.Sort([](const FEnemyInfo& A, const FEnemyInfo& B)
		{
			return A.Distance < B.Distance;
		});

	// 정렬된 정보 복사
	if (IsTraining) {
		for (const FEnemyInfo& Info : EnemyInfoList)
		{
			EnemyCharacters.Add(Info.EnemyChar);
			EnemyLocation.Add(Info.Location);
			EnemyDirection.Add(Info.Direction);
		}
	}
	else {
		if (AttackTarget)
		{
			bool bFound = false;
			for (const FEnemyInfo& Info : EnemyInfoList)
			{
				if (Info.EnemyChar == AttackTarget)
				{
					EnemyCharacters.Add(Info.EnemyChar);
					EnemyLocation.Add(Info.Location);
					EnemyDirection.Add(Info.Direction);
					bFound = true;
					break;
				}
			}

			// AttackTarget이 유효하지 않거나 못 찾은 경우 → 가장 가까운 적
			if (!bFound && EnemyInfoList.Num() > 0)
			{
				const FEnemyInfo& Info = EnemyInfoList[0];
				EnemyCharacters.Add(Info.EnemyChar);
				EnemyLocation.Add(Info.Location);
				EnemyDirection.Add(Info.Direction);
			}
		}
		else
		{
			// AttackTarget이 없으면 그냥 가장 가까운 적
			if (EnemyInfoList.Num() > 0)
			{
				const FEnemyInfo& Info = EnemyInfoList[0];
				EnemyCharacters.Add(Info.EnemyChar);
				EnemyLocation.Add(Info.Location);
				EnemyDirection.Add(Info.Direction);
			}
		}
	}
}

void ARLCharacter::SetAttackTarget(ARLCharacter* Target)
{ 
	AttackTarget = Target; 
	MakeEnemyInformation();
}

void ARLCharacter::UpdateEnemyInfo()
{
    EnemyLocation.Empty();
    EnemyDirection.Empty();

    for (ARLCharacter* Enemy : EnemyCharacters)
    {
        if (Enemy)
        {
            EnemyLocation.Add(Enemy->GetActorLocation());
            EnemyDirection.Add(Enemy->GetActorForwardVector());
        }
        else
        {
            // 죽었거나 nullptr이면 기본값
            EnemyLocation.Add(FVector::ZeroVector);
            EnemyDirection.Add(FVector::ForwardVector);
        }
    }
}

// Called when the game starts or when spawned
void ARLCharacter::BeginPlay()
{
	Super::BeginPlay();

	// GetMesh()->SetNotifyRigidBodyCollision(true);
	// GetMesh()->OnComponentHit.AddDynamic(this, &ARLCharacter::OnMeshHit);

	SelectedCircle->SetVisibility(false, true);

	FTransform HandRightTransform = GetMesh()->GetSocketTransform(hand_r, ERelativeTransformSpace::RTS_World);
	FVector HandRightLocation = HandRightTransform.GetLocation();
	FRotator HandRightRotation = HandRightTransform.GetRotation().Rotator();

	FTransform HandLeftTransform = GetMesh()->GetSocketTransform(hand_l, ERelativeTransformSpace::RTS_World);
	FVector HandLeftLocation = HandLeftTransform.GetLocation();
	FRotator HandLeftRotation = HandLeftTransform.GetRotation().Rotator();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

	if (HandRight)
	{
		AWeapon* HandRightActor = GetWorld()->SpawnActor<AWeapon>(HandRight, HandRightLocation, HandRightRotation, SpawnParams);
		if (HandRightActor)
		{
			HandRightActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HandGrip_R"));
			HandRightActor->SetActorRelativeTransform(
				FTransform(
					FRotator(29.999263, 0.000216f, -19.999396f), // 회전
					FVector(-26.369668f, -10.794922f, 32.354082f), // 위치
					FVector(1.f, 1.f, 1.f)    // 스케일
				)
			);
		}

		if (HandRightActor->BoxComponent)
		{
			HandRightActor->BoxComponent->OnComponentHit.AddDynamic(this, &ARLCharacter::OnMeshHit);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandRight is invalid or not an Actor class"));
	}

	if (HandLeft)
	{
		AWeapon* HandLeftActor = GetWorld()->SpawnActor<AWeapon>(HandLeft, HandLeftLocation, HandLeftRotation, SpawnParams);
		if (HandLeftActor)
		{
			HandLeftActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HandGrip_L"));
			HandLeftActor->SetActorRelativeTransform(
				FTransform(
					FRotator(90.000000f, 62.606395f, -177.376906f), // 회전
					FVector(3.061203f, 5.487673f, 5.872030f), // 위치
					FVector(0.5f, 0.5f, 0.5f)    // 스케일
				)
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandLeft is invalid or not an Actor class"));
	}

	// LearningAgentsManager 찾기
	if(IsTraining)
		RegisterToLearningManager();

	// Origin 찾기
	TArray<AActor*> Origins;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(), OriginTag, Origins
	);
	float Distance = 10000.0f;
	AActor* NearestOrigin =
		UGameplayStatics::FindNearestActor(GetActorLocation(), Origins, Distance);
	if (NearestOrigin)
	{
		OriginLocation = NearestOrigin->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("OriginLocation: %s"), *OriginLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" origin found within %.2f units"), Distance);
	}

	InitSimulatePhysics();
	InitPointHandle();

	CalculateMaxRange();

	MakeEnemyInformation();
	if (IsTraining)
	{
		RLResetCharacter();
	}
	
	LastLocation = GetActorLocation();

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();
	/*UE_LOG(LogTemp, Warning, TEXT("InitialLocation: %s, InitialRotation: %s"), 
		*InitialLocation.ToString(), *InitialRotation.ToString());*/
	Mode = GetCharacterMovement()->MovementMode;
}

void ARLCharacter::RegisterToLearningManager()
{
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(), ManagerTag, Managers
	);
	for (AActor* Actor : Managers)
	{
		Manager = Cast<ULearningAgentsManager>(
			Actor->GetComponentByClass(ULearningAgentsManager::StaticClass()));
		if (Manager)
		{
			if (Manager->HasAgent(AgentId))
			{
				// UE_LOG(LogTemp, Log, TEXT("Agent %d already registered."), AgentId);
			}
			else
			{
				AgentId = Manager->AddAgent(this);
				// UE_LOG(LogTemp, Log, TEXT("Registered new Agent: %d"), AgentId);
			}

			FoundManager = true;
			break;
		}
	}
	if (!FoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find Learning Agents manager."));
	}
}

void ARLCharacter::UnregisterFromLearningManager()
{
	if (Manager && AgentId != INDEX_NONE)
	{
		Manager->RemoveAgent(AgentId);
		UE_LOG(LogTemp, Log, TEXT("%s: Unregistered from Learning Agents Manager (AgentId=%d)."),
			*GetName(), AgentId);
		AgentId = INDEX_NONE;
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("%s: No valid manager or agent ID to unregister."), *GetName());
	}
}

void ARLCharacter::CalculateMaxRange()
{
	const FVector HandLoc = GetMesh()->GetSocketLocation("hand_r");
	const FVector LowerarmLoc = GetMesh()->GetSocketLocation("lowerarm_r");
	const FVector UpperarmLoc = GetMesh()->GetSocketLocation("upperarm_r");

	float ArmLength1 = FVector::Dist(LowerarmLoc, HandLoc);
	float ArmLength2 = FVector::Dist(LowerarmLoc, UpperarmLoc);
	float ArmLength = ArmLength1 + ArmLength2;

	MaxRange = ArmLength * 4.f;
}

void ARLCharacter::InitSimulatePhysics()
{
	FName Pelvis = TEXT("pelvis");
	FName ProfileTest = TEXT("Test");
	PhysicalAnim->SetSkeletalMeshComponent(GetMesh());
	PhysicalAnim->ApplyPhysicalAnimationProfileBelow(Pelvis, ProfileTest, true, false);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(Pelvis, true, false);

	FName foot_r = TEXT("foot_r");
	FName foot_l = TEXT("foot_l");
	FName spine_03 = TEXT("spine_03");
	FName neck_02 = TEXT("neck_02");
	GetMesh()->SetBodySimulatePhysics(foot_r, false);
	GetMesh()->SetBodySimulatePhysics(foot_l, false);
	GetMesh()->SetBodySimulatePhysics(spine_03, false);
	GetMesh()->SetBodySimulatePhysics(neck_02, false);
}

void ARLCharacter::InitPointHandle()
{
	RightHandle->ReleaseComponent();
	LeftHandle->ReleaseComponent();

	RightPoint->SetWorldLocation(GetMesh()->GetSocketLocation(hand_rSocket));
	RightPoint->SetWorldRotation(FRotator::ZeroRotator);

	RightHandle->GrabComponentAtLocationWithRotation(
		GetMesh(),
		hand_r,
		RightPoint->GetComponentLocation(),
		RightPoint->GetComponentRotation()
	);

	LeftPoint->SetWorldLocation(GetMesh()->GetSocketLocation(hand_lSocket));
	LeftPoint->SetWorldRotation(FRotator::ZeroRotator);

	LeftHandle->GrabComponentAtLocationWithRotation(
		GetMesh(),
		hand_l,
		LeftPoint->GetComponentLocation(),
		LeftPoint->GetComponentRotation()
	);
}

void ARLCharacter::UpdateCombat()
{
	if (EnemyCharacters.Num() <= 0)
		return;

	ARLCharacter* Target = EnemyCharacters[0];
	
	if (Target && !IsDead)
	{
		// UE_LOG(LogTemp, Log, TEXT("[%s] Target is: %s"), *GetName(), *Target->GetName());
	}

	if (Target->GetIsDead()) {
		AttackTarget = NULL;
		UnregisterFromLearningManager();
		MakeEnemyInformation();
	}

	if (!Target || Target->GetIsDead()) {
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	if (Distance <= MaxEnemyDistance/2)
	{
		RegisterToLearningManager();
	}
	else if (Distance >= MaxEnemyDistance/2 + 100)
	{
		AttackTarget = NULL;
		UnregisterFromLearningManager();
		MakeEnemyInformation();
	}
}

// Called every frame
void ARLCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateEnemyInfo();
	
	if(!IsTraining)
		UpdateCombat();

	if (bBattleStarted && TeamID!=1 && !IsTraining && !IsTestBattle && AgentId==INDEX_NONE) {
		ARLCharacter* TargetEnemy = nullptr;
		for (ARLCharacter* Enemy : EnemyCharacters)
		{
			if (Enemy && Enemy->TeamID == 1 && !Enemy->GetIsDead())
			{
				TargetEnemy = Enemy;
				break; // 첫 번째 살아있는 적만
			}
		}

		if (TargetEnemy)
		{
			FVector MyLocation1 = GetActorLocation();
			FVector EnemyLocation1 = TargetEnemy->GetActorLocation();
			FVector Direction = (EnemyLocation1 - MyLocation1).GetSafeNormal();

			float DesiredDistance = 150.f;
			FVector MoveLocation = EnemyLocation1 - Direction * DesiredDistance;

			if (ARLCharAIController* UnitAI = Cast<ARLCharAIController>(GetController()))
			{
				if (!UnitAI->GetIsMoving())
				{
					UnitAI->MoveToTargetLocation(MoveLocation);
					// UE_LOG(LogTemp, Warning, TEXT("Moving near target to: %s"), *MoveLocation.ToString());
				}
			}
		}
	}

	RightHandle->SetTargetLocationAndRotation(
		RightPoint->GetComponentLocation(),
		RightPoint->GetComponentRotation()
	);

	LeftHandle->SetTargetLocationAndRotation(
		LeftPoint->GetComponentLocation(),
		LeftPoint->GetComponentRotation()
	);

	//// 적의 0번째 위치에 박스 디버그 표시
	//if (EnemyLocation.Num() > 0)
	//{
	//	FVector EnemyPos = EnemyLocation[0];

	//	DrawDebugBox(
	//		GetWorld(),
	//		EnemyPos,                 // 박스 중심 (적 위치)
	//		FVector(20.f, 20.f, 20.f),// 박스 크기 (반지름 단위, 40x40x40 크기 됨)
	//		FColor::Red,            // 색상
	//		false,                    // 지속 여부 (true면 계속 표시)
	//		-1.f,                     // 지속 시간 (-1은 한 프레임만 표시)
	//		0,                        // 깊이 우선순위
	//		2.f                       // 선 두께
	//	);
	//}

	//// 적의 바라보는 방향(Forward)을 화살표로 표시
	//if (EnemyLocation.Num() > 0 && EnemyDirection.Num() > 0)
	//{
	//	const float ArrowLen = 150.f; // 화살표 길이
	//	const float ArrowSize = 20.f;  // 화살촉 크기
	//	const float Thickness = 2.f;

	//	const int32 Count = FMath::Min(EnemyLocation.Num(), EnemyDirection.Num());
	//	for (int32 i = 0; i < Count; ++i)
	//	{
	//		const FVector Start = EnemyLocation[i];
	//		const FVector Dir = EnemyDirection[i].GetSafeNormal();

	//		// 방향을 명확히 보이게 하려면 화살표로
	//		DrawDebugDirectionalArrow(
	//			GetWorld(),
	//			Start,                      // 시작점
	//			Start + Dir * ArrowLen,     // 끝점
	//			ArrowSize,                  // 화살촉 크기
	//			FColor::Yellow,             // 색상
	//			false,                      // 지속여부 (false: 1프레임)
	//			-1.f,                       // 지속시간
	//			0,                          // 깊이 우선순위
	//			Thickness                   // 두께
	//		);
	//	}
	//}

	if (EnemyCharacters.Num() > 0) {
		uint32 Hash = GetUniqueID();
		FColor UniqueColor = FColor(
			(Hash * 23) % 255,
			(Hash * 67) % 255,
			(Hash * 101) % 255
		);

		FVector BaseLocation = EnemyCharacters[0]->GetActorLocation();
		FVector TopLocation = BaseLocation + FVector(0, 0, 100.0f);

		// 실린더 표시
		//DrawDebugCylinder(
		//	GetWorld(),
		//	BaseLocation,
		//	TopLocation,
		//	25.0f,
		//	12,
		//	UniqueColor,
		//	false,
		//	-1.0f, // 프레임마다 갱신
		//	0,
		//	1.5f
		//);

		// 캐릭터 이름 표시 (실린더 위)
		//FString NameText = FString::Printf(TEXT("%s"), *GetName());
		//float TextWidthApprox = NameText.Len() * 7.0f;
		//FVector TextLocation = TopLocation + FVector(-TextWidthApprox * 0.5f, 0, 30.0f);

		//DrawDebugString(
		//	GetWorld(),
		//	TextLocation,
		//	NameText,
		//	nullptr,
		//	UniqueColor,
		//	0.0f,   // 한 프레임 표시 (Tick에서 계속 호출됨)
		//	false,   // 카메라 거리 상관없이 고정 크기
		//	1.0f    // 텍스트 크기 스케일
		//);
	}

	if (bArenaInitialized)
	{
		// Arena 반경을 2D 원형으로 표시
		DrawDebugCircle(
			GetWorld(),
			ArenaCenter,                // 중심
			ArenaRadius,                // 반경
			64,                         // 세그먼트 수 (원 부드럽게)
			FColor::Green,              // 색상
			false,                      // 지속 여부 (false = 1프레임만)
			-1.0f,                      // LifeTime (-1 = 한 프레임)
			0,                          // Depth Priority
			10.0f,                       // 선 두께
			FVector(1, 0, 0),             // X축 방향
			FVector(0, 1, 0),             // Y축 방향
			false                       // Z축에서 바라본 원
		);
	}

	float MovedDist = FVector::Dist2D(GetActorLocation(), LastLocation);

	if (MovedDist < StuckThreshold)
	{
		StuckTimer += DeltaTime;
	}
	else
	{
		StuckTimer = 0.0f;
	}

	LastLocation = GetActorLocation();

	ShowDebugSphere();
	ShowRightHandAngle();

	if (bSelected) {
		SelectedCircle->SetVisibility(true, true);
	}
	else {
		SelectedCircle->SetVisibility(false, true);
	}
}

void ARLCharacter::ShowDebugSphere()
{
	DrawDebugSphere(
		GetWorld(),             // 월드 컨텍스트
		LeftPoint->GetComponentLocation(),     // 중심 위치
		5.0f,                  // 반지름
		12,                    // 세그먼트 수 (자세함 정도)
		FColor::Blue,            // 색상
		false,                  // 지속 여부 (true면 계속 표시)
		0.0f,                   // 지속 시간 (false일 때만 유효)
		0,                      // 깊이 우선순위
		0.0f                    // 선 두께
	);

	DrawDebugSphere(
		GetWorld(),             // 월드 컨텍스트
		RightPoint->GetComponentLocation(),     // 중심 위치
		5.0f,                  // 반지름
		12,                    // 세그먼트 수 (자세함 정도)
		FColor::Red,            // 색상
		false,                  // 지속 여부 (true면 계속 표시)
		0.0f,                   // 지속 시간 (false일 때만 유효)
		0,                      // 깊이 우선순위
		0.0f                    // 선 두께
	);

	// // UPhysicsHandleComponent 이동 가능 범위
	// DrawDebugSphere(
	//     GetWorld(),                              // 월드 컨텍스트
	//     GetMesh()->GetSocketLocation("neck_01"), // 중심 위치
	//     MaxRange,                                // 반지름
	//     32,                                      // 세그먼트 수 (자세함 정도)
	//     FColor::Green,                           // 색상
	//     false,                                   // 지속 여부 (true면 계속 표시)
	//     0.0f,                                    // 지속 시간 (false일 때만 유효)
	//     0,                                       // 깊이 우선순위
	//     0.0f                                     // 선 두께
	// );
}

void ARLCharacter::ShowRightHandAngle()
{
	FTransform HandRightTransform = GetMesh()->GetSocketTransform(hand_r, ERelativeTransformSpace::RTS_World);
	FVector BoneLocation = HandRightTransform.GetLocation();
	FRotator BoneRotation = HandRightTransform.GetRotation().Rotator();

	{
		FVector CurrLocation = BoneLocation;
		FRotator CurrRotation = BoneRotation;
		if (bHasPrev)
		{
			DeltaLoc = CurrLocation - PrevLocation;
			DeltaRot = (CurrRotation - PrevRotation).GetNormalized();
		}
		PrevLocation = CurrLocation;
		PrevRotation = CurrRotation;
		bHasPrev = true;
	}

	FRotationMatrix RotMatrix(BoneRotation);

	FVector Forward = RotMatrix.GetUnitAxis(EAxis::X); // 빨강: +X (Forward)
	FVector Right = RotMatrix.GetUnitAxis(EAxis::Y);   // 초록: +Y (Right)
	FVector Up = RotMatrix.GetUnitAxis(EAxis::Z);      // 파랑: +Z (Up)

	float LineLength = 20.f;

	DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Forward * LineLength, FColor::Red, false, -1.f, 0, 2.f);
	DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Right * LineLength, FColor::Green, false, -1.f, 0, 2.f);
	DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Up * LineLength, FColor::Blue, false, -1.f, 0, 2.f);
}

void ARLCharacter::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (this->IsDead)
		return;

	if (OtherActor && OtherActor != this)
	{
		FName HitBone = Hit.BoneName;
		if (HitBone != "hand_r" && HitBone != "hand_l")
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor != nullptr)
			{
				ARLCharacter* Hit_ARLCharacter = Cast<ARLCharacter>(HitActor);
				if (Hit_ARLCharacter->GetTeamID() == this->TeamID)
					return;

				FVector ShotDirection;
				AController* OwnerController = nullptr;

				FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
				APawn* OwnerPawn = Cast<APawn>(GetOwner());
				if (OwnerPawn != nullptr)
				{
					OwnerController = OwnerPawn->GetController();
				}
				HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
			}

			/*UE_LOG(LogTemp, Warning, TEXT("=== OnMeshHit Called ==="));
			UE_LOG(LogTemp, Warning, TEXT("HitComp: %s"), *HitComp->GetName());
			UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Hit BoneName: %s"), *Hit.BoneName.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *GetNameSafe(Hit.GetActor()));*/
			
			// UE_LOG(LogTemp, Warning, TEXT("Hit !!!!!"));
			// UE_LOG(LogTemp, Warning, TEXT("BoneName: %s"), *GetMesh()->FindClosestBone(Hit.ImpactPoint).ToString());

			/*
			UE_LOG(LogTemp, Warning, TEXT("OtherComp: %s"), *OtherComp->GetName());
			UE_LOG(LogTemp, Warning, TEXT("NormalImpulse: %s"), *NormalImpulse.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Hit Location: %s"), *Hit.ImpactPoint.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *GetNameSafe(Hit.GetComponent()));
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *GetNameSafe(Hit.GetActor()));
			*/

			bIsHit = true;
			GetWorld()->GetTimerManager().ClearTimer(HitResetTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				HitResetTimerHandle,
				this,
				&ARLCharacter::ResetHitState,
				HitDuration,
				false
			);

		}
	}
}

void ARLCharacter::ResetHitState()
{
	bIsHit = false;
}

float ARLCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApplied = FMath::Min(Health, DamageToApplied);
	Health = Health - DamageToApplied;

	// UE_LOG(LogTemp, Warning, TEXT("Health : %f"), Health);

	if (Health <= 0)
	{
		IsDead = true;
		// UE_LOG(LogTemp, Warning, TEXT("Dead"));

		if (!IsTraining) {
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetMesh()->WakeAllRigidBodies();

			GetCharacterMovement()->DisableMovement();

			if (IsTestBattle) {
				FTimerHandle ResetTimerHandle1;
				GetWorld()->GetTimerManager().SetTimer(
					ResetTimerHandle1,
					FTimerDelegate::CreateUObject(this, &ARLCharacter::ResetCharacterInTrainMap),
					3.0f, false
				);
			}
			else {
				FTimerHandle ResetTimerHandle1;
				GetWorld()->GetTimerManager().SetTimer(
					ResetTimerHandle1,
					FTimerDelegate::CreateUObject(this, &ARLCharacter::OnDeath),
					3.0f, false
				);
			}
		}
	}

	return DamageToApplied;
}

void ARLCharacter::OnDeath()
{
	UE_LOG(LogTemp, Display, TEXT("OnDeath"));
	// 컨트롤러나 매니저에게 알리기
	ABattleController* BattleController = Cast<ABattleController>(UGameplayStatics::GetPlayerController(this, 0));
	if (BattleController)
	{
		BattleController->NotifyCharacterDeath(this);
	}
}

void ARLCharacter::RLResetCharacter()
{
	if (EnemyCharacters.Num() <= 0)
	{
		return;
	}
	if (OriginLocation == FVector::ZeroVector)
	{
		return;
	}

	GetMesh()->SetSimulatePhysics(false);

	float RandomRadian = FMath::FRandRange(0.f, 6.28f);
	float RandomDistance = FMath::FRandRange(0.5f, 1.f) * ResetDistance;

	float ResetLocationX =
		EnemyLocation[0].X + FMath::Cos(RandomRadian) * RandomDistance;
	float ResetLocationY =
		EnemyLocation[0].Y + FMath::Sin(RandomRadian) * RandomDistance;
	FVector ResetLocation =
		FVector(ResetLocationX, ResetLocationY, EnemyLocation[0].Z);

	// Origin 기준 거리 계산
	FVector Direction = ResetLocation - OriginLocation;
	float Distance = Direction.Size();
	// ResetLocation이 Origin으로부터 너무 멀면 반대편으로 보정
	if (Distance > MaxRadius)
	{
		// Direction = Direction.GetSafeNormal();
		// ResetLocation = OriginLocation + Direction * MaxRadius;

		FVector2D XYDirection = FVector2D(Direction.X, Direction.Y).GetSafeNormal();
		FVector2D OppositeXY = -XYDirection * MaxRadius;
		ResetLocation = FVector(OriginLocation.X + OppositeXY.X,
			OriginLocation.Y + OppositeXY.Y,
			ResetLocation.Z);
	}

	// Enemy와의 거리가 너무 떨어져 있으면 Enemy 방향으로 캐릭터를 위치시킨다
	float EnemyDistance = FVector::Dist(ResetLocation, EnemyLocation[0]);
	if (EnemyDistance > MaxEnemyDistance)
	{
		FVector ToEnemy = (EnemyLocation[0] - ResetLocation).GetSafeNormal();
		float PullAmount = EnemyDistance - MaxEnemyDistance + 100.f;

		ResetLocation += ToEnemy * PullAmount;
	}

	SetActorLocation(ResetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	EnemyCharacters[0]->SetIsDead(false);
	EnemyCharacters[0]->SetHealth(100.0);
	
	Health = 100.0;
	IsDead = false;
	IsCompletion = false;
	IsCompletionReceiver = false;
	Stamina = 0;
	StuckTimer = 0.0f;

	InitSimulatePhysics();
	InitPointHandle();
	LastLocation = GetActorLocation();

	bArenaInitialized = false;
	GetWorld()->GetTimerManager().SetTimer(
		ArenaInitTimerHandle,
		FTimerDelegate::CreateUObject(this, &ARLCharacter::InitializeArena),
		1.0f, false
	);
}

void ARLCharacter::ResetCharacterInTrainMap()
{
	if (EnemyCharacters.Num() <= 0)
	{
		return;
	}

	AttackTarget = NULL;
	UnregisterFromLearningManager();
	MakeEnemyInformation();
	

	GetMesh()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetMesh()->SetAllBodiesSimulatePhysics(false);
	// GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), false, true);
	// GetMesh()->SetPhysicsBlendWeight(0.0f); // 0 = 애니메이션 100%

	// GetMesh()->ResetAllBodiesSimulatePhysics();
	// GetMesh()->RefreshBoneTransforms();
	// GetMesh()->ResetAllBodiesSimulatePhysics();
	// GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	// GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	// GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator::ZeroRotator);

	SetActorLocation(InitialLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(InitialRotation, ETeleportType::TeleportPhysics);

	GetMesh()->SetWorldLocationAndRotation(
		GetCapsuleComponent()->GetComponentLocation() - FVector(0.0f, 0.0f, 90.f),
		GetCapsuleComponent()->GetComponentRotation() - FRotator(0.0f, 90.0f, 0.0f));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform, TEXT("pelvis"));

	GetCharacterMovement()->SetMovementMode(Mode);
	
	// EnemyCharacters[0]->SetIsDead(false);
	// EnemyCharacters[0]->SetHealth(100.0);

	Health = 100.0;
	IsDead = false;
	IsCompletion = false;
	IsCompletionReceiver = false;
	Stamina = 0;
	StuckTimer = 0.0f;

	InitSimulatePhysics();
	InitPointHandle();
}

void ARLCharacter::ResetCharacterInTestBattleMap(FVector Location, FRotator Rotation)
{
	if (EnemyCharacters.Num() <= 0)
	{
		return;
	}

	AttackTarget = NULL;
	UnregisterFromLearningManager();
	MakeEnemyInformation();


	GetMesh()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetMesh()->SetAllBodiesSimulatePhysics(false);
	// GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), false, true);
	// GetMesh()->SetPhysicsBlendWeight(0.0f); // 0 = 애니메이션 100%

	// GetMesh()->ResetAllBodiesSimulatePhysics();
	// GetMesh()->RefreshBoneTransforms();
	// GetMesh()->ResetAllBodiesSimulatePhysics();
	// GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	// GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	// GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator::ZeroRotator);

	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(Rotation, ETeleportType::TeleportPhysics);

	GetMesh()->SetWorldLocationAndRotation(
		GetCapsuleComponent()->GetComponentLocation() - FVector(0.0f, 0.0f, 90.f),
		GetCapsuleComponent()->GetComponentRotation() - FRotator(0.0f, 90.0f, 0.0f));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform, TEXT("pelvis"));

	GetCharacterMovement()->SetMovementMode(Mode);

	// EnemyCharacters[0]->SetIsDead(false);
	// EnemyCharacters[0]->SetHealth(100.0);

	Health = 100.0;
	IsDead = false;
	IsCompletion = false;
	IsCompletionReceiver = false;
	Stamina = 0;
	StuckTimer = 0.0f;

	InitSimulatePhysics();
	InitPointHandle();
}

void ARLCharacter::InitializeArena()
{
	const TArray<ARLCharacter*>& Enemies = GetEnemyCharacters();
	if (Enemies.Num() > 0)
	{
		FVector MyLoc = GetActorLocation();
		FVector EnemyLoc = Enemies[0]->GetActorLocation();

		ArenaCenter = (MyLoc + EnemyLoc) * 0.5f;

		FVector ToOrigin = ArenaCenter - OriginLocation;
		if (ToOrigin.Size() > ArenaMaxOffset)
		{
			ArenaCenter = OriginLocation + ToOrigin.GetSafeNormal() * ArenaMaxOffset;
		}

		bArenaInitialized = true;

		UE_LOG(LogTemp, Log, TEXT("[%s] Arena Initialized: Center=%s"),
			*GetName(), *ArenaCenter.ToString());
	}
}

//////////////////////////////////////////////////////////////////////////
// getter, setter
const TArray<ARLCharacter*>& ARLCharacter::GetEnemyCharacters() const
{
	return EnemyCharacters;
}
const TArray<FVector>& ARLCharacter::GetEnemyLocation() const
{
	return EnemyLocation;
}
const TArray<FVector>& ARLCharacter::GetEnemyDirection() const
{
	return EnemyDirection;
}
USceneComponent* ARLCharacter::GetRightPoint() const
{
	return RightPoint;
}
USceneComponent* ARLCharacter::GetLeftPoint() const
{
	return LeftPoint;
}
int32 ARLCharacter::GetMaxStamina() const
{
	return MaxStamina;
}
int32 ARLCharacter::GetStamina() const
{
	return Stamina;
}
void ARLCharacter::SetStamina(int32 NewStamina)
{
	Stamina = NewStamina;
}
bool ARLCharacter::IsHit() const
{
	return bIsHit;
}
void ARLCharacter::SetIsHit(bool bHit)
{
	bIsHit = bHit;
}
bool ARLCharacter::GetIsDead() const
{
	return IsDead;
}
void ARLCharacter::SetIsDead(bool bDead)
{
	IsDead = bDead;
}


