// Fill out your copyright notice in the Description page of Project Settings.


#include "TaggerCharacter.h"

#include "BishopGameMode.h"
#include "BishopPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TypingUIWidget.h"
#include "BishopWeapon.h"
#include "TaggerUIWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NoirBlanc/TP_ThirdPerson/TP_ThirdPersonCharacter.h"

class ABishopGameMode;
// Sets default values
ATaggerCharacter::ATaggerCharacter()
{
	SetReplicates(true);

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	// MeshComponent->SetIsReplicated(true);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(50.f, 60.f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// // Configure character movement
	// GetCharacterMovement()->BrakingFriction = 10000.0f;
	// GetCharacterMovement()->GroundFriction = 10000.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 10000.f;
	// GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	// GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	//
	// // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// // instead of recompiling to adjust them
	// GetCharacterMovement()->JumpZVelocity = 700.f;
	// GetCharacterMovement()->AirControl = 0.35f;
	// GetCharacterMovement()->MaxWalkSpeed = 500.f;
	// GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	// GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	// GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
}

// Called to bind functionality to input
void ATaggerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(TaggerMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATaggerCharacter::Look);

		// Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATaggerCharacter::JumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::None, this, &ATaggerCharacter::JumpEnded);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

// Called when the game starts or when spawned
void ATaggerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		// Tagger 플레이어에게 타이핑 UI 띄우기
		if (TypingUIWidgetClass == nullptr) return;
		TypingUIWidget = CreateWidget<UTypingUIWidget>(GetWorld()->GetFirstPlayerController(), TypingUIWidgetClass);
		if (TypingUIWidget == nullptr) return;
		TypingUIWidget->AddToViewport();
		TypingUIWidget->UserInput->SetVisibility(ESlateVisibility::Hidden);

		// TODO: 비즈니스 로직은 서버에서 해줘야 함
		// 서버에서 랜덤 텍스트 요청하기
		ServerRPC_SetRandomText();
		
		// Taager UI 띄우기 (점프 Power Progress Bar)
		if (TaggerUIWidgetClass == nullptr) return;
		TaggerUIWidget = CreateWidget<UTaggerUIWidget>(GetWorld()->GetFirstPlayerController(), TaggerUIWidgetClass);
		if (TaggerUIWidget == nullptr) return;
		TaggerUIWidget->AddToViewport();
	}
}

void ATaggerCharacter::ServerRPC_SetRandomText_Implementation()
{
	// GameMode에게 RandomText 생성해 달라고 요청하기
	ABishopGameMode* BishopGameMode = Cast<ABishopGameMode>(GetWorld()->GetAuthGameMode());
	if (BishopGameMode)
	{
		BishopGameMode->PickRandomTextAndUpdateUI();
	}
}

void ATaggerCharacter::UpdateText(const FText& InputedText)
{
	ServerRPC_UpdateText(InputedText);
}

void ATaggerCharacter::ServerRPC_UpdateText_Implementation(const FText& InputedText)
{
	// 받은 텍스트를 모두에게 띄워야 함.
	ABishopGameMode* BishopGameMode = Cast<ABishopGameMode>(GetWorld()->GetAuthGameMode());
	if (BishopGameMode == nullptr) return;

	BishopGameMode->UpdateInputtedText(InputedText);
	// MulticastRPC_SetUITextTo(InputedText, BishopGameMode->CurrentTextToType, TODO);
}

void ATaggerCharacter::MulticastRPC_SpawnWeapon_Implementation(FVector Location, FRotator Rotation, UClass* WeaponClass)
{
	// 공격하라고 Multicast
	GetWorld()->SpawnActor<ABishopWeapon>(WeaponClass, Location, Rotation);
}

void ATaggerCharacter::MulticastRPC_SetUITextTo_Implementation
(
	const FText& InputedText,
	const FText& CurrentTextToType,
	const TArray<bool>& StringCorrectArray
)
{
	if (TypingUIWidget == nullptr) return;
	// TypingUIWidget->TextToTypeUI->SetText(CurrentTextToType);
	TypingUIWidget->SetStyledTextToTypeUI(CurrentTextToType, StringCorrectArray);
	TypingUIWidget->TypedTextUI->SetText(InputedText);
}


void ATaggerCharacter::Look(const FInputActionValue& Value)
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

void ATaggerCharacter::JumpStarted(const struct FInputActionValue& Value)
{
	ServerRPC_JumpStarted(Value);
}

void ATaggerCharacter::JumpEnded(const FInputActionValue& Value)
{
	ServerRPC_JumpEnded(Value);
}

void ATaggerCharacter::ServerRPC_JumpStarted_Implementation(const FInputActionValue& Value)
{
	// 현재 공중에 있으면 Return
	if (GetMovementComponent()->IsFalling()) return;

	// 힘 쌓기 시작
	GetWorld()->GetTimerManager().SetTimer(JumpChargineTimerHandle, this, &ATaggerCharacter::ChargeJumpPower, 0.01f,
	                                       true);
}

void ATaggerCharacter::ServerRPC_JumpEnded_Implementation(const FInputActionValue& Value)
{
	if (CurrentJumpPower <= 0.f) return;
	if (GetMovementComponent()->IsFalling()) return;
	// 현재 쌓아놓은 힘 만큼 캐릭터 발사!

	// 타이머 초기화
	GetWorld()->GetTimerManager().ClearTimer(JumpChargineTimerHandle);

	// TODO: Bishop이 죽으면 동작을 안함. 수정할 것.
	// Bishop을 향하는 Vector 구하기
	TArray<AActor*> AllBishopPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABishopPawn::StaticClass(), AllBishopPawns);
	for (AActor* BishopPawn : AllBishopPawns)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bishop? : %s"), *BishopPawn->GetActorNameOrLabel());
		FVector ToBishop = BishopPawn->GetActorLocation() - GetActorLocation();
		ToBishop.Normalize();
		
		FVector ForwardUpVector = ToBishop + FVector::UpVector;
		ForwardUpVector.Normalize();
		
		// 캐릭터 발사!
		float PercentJumpPower = FMath::GetRangePct(MinJumpPower, MaxJumpPower, CurrentJumpPower);
		float EaseCurrentJumpPower = FMath::InterpEaseOut(MinJumpPower, MaxJumpPower, PercentJumpPower, 5.f);
		float ClampCurrentJumpPower = FMath::Clamp(EaseCurrentJumpPower, MinJumpPower, MaxJumpPower);

		UE_LOG(LogTemp, Warning, TEXT("Ease Jump Power : %f"), ClampCurrentJumpPower);
		LaunchCharacter(ForwardUpVector * EaseCurrentJumpPower, true, true);

		// 현재 JumpPower 초기화
		CurrentJumpPower = 0.f;
		MeshComponent->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
		MulticastRPC_UpdateProgressBar(0.f);
	}
}

void ATaggerCharacter::ChargeJumpPower()
{
	CurrentJumpPower += MultiplyJumpPower * 0.01f;
	CurrentJumpPower = FMath::Clamp(CurrentJumpPower, MinJumpPower, MaxJumpPower);
	// UE_LOG(LogTemp, Warning, TEXT("Jump Power : %f"), CurrentJumpPower);
	
	UpdateScale();

	// Progress Bar Update
	float PercentJumpPower = FMath::GetRangePct(MinJumpPower, MaxJumpPower, CurrentJumpPower);
	float EaseCurrentJumpPower = FMath::InterpEaseOut(0.f, 1.f, PercentJumpPower, 5.f);
	float ClampCurrentJumpPower = FMath::Clamp(EaseCurrentJumpPower, 0.f, 1.f);

	MulticastRPC_UpdateProgressBar(ClampCurrentJumpPower);
}

void ATaggerCharacter::MulticastRPC_UpdateProgressBar_Implementation(float Value)
{
	if (TaggerUIWidget == nullptr) return;
	TaggerUIWidget->JumpChargePower->SetPercent(Value);
}

void ATaggerCharacter::UpdateScale()
{
	// CurrentJumpPower를 0~1 사이의 값으로 치환해야 함.
	// MinJumpPower, MaxJumpPower를 0과 1로 봐야 함.
	float JumpScale = FMath::GetRangePct(MaxJumpPower, MinJumpPower, CurrentJumpPower);
	// 다시 범위 조정
	float EaseJumpScale = FMath::InterpEaseIn(MinSquashScale, MaxSquashScale, JumpScale, 5.f);
	// UE_LOG(LogTemp, Warning, TEXT("JumpScale : %f"), EaseJumpScale);
	MeshComponent->SetWorldScale3D(FVector(1.f, 1.f, EaseJumpScale));
}

// Called every frame
void ATaggerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}