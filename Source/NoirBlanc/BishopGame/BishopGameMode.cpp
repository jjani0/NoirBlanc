// Fill out your copyright notice in the Description page of Project Settings.

#include "BishopGameMode.h"
#include "BishopPawn.h"
#include "NoirBlancPlayerState.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "NoirBlanc/TP_ThirdPerson/TP_ThirdPersonCharacter.h"
#include "BishopWeapon.h"
#include "NoirBlancGameInstance.h"
#include "TaggerCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PawnMovementComponent.h"

ABishopGameMode::ABishopGameMode()
{
	TArray<AActor*> AllStartPointActors;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), AllStartPointActors);

	for (int i = 0; i < AllStartPointActors.Num(); ++i)
	{
		APlayerStart* _PlayerStart = Cast<APlayerStart>(AllStartPointActors[i]);
		if (_PlayerStart)
		{
			AllStartPoints.Add(_PlayerStart);
		}
	}
}

AActor* ABishopGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ANoirBlancPlayerState* _PlayerState = Player->GetPlayerState<ANoirBlancPlayerState>();
	UNoirBlancGameInstance* _NoirBlancGameInstance = GetGameInstance<UNoirBlancGameInstance>();

	// 게임모드 안에서 LocalPlayer다? 서버이다 => 하얀색
	if (Player->IsLocalPlayerController())
	{
		_PlayerState->PieceColor = EPieceColor::White;

		if (_NoirBlancGameInstance->AttackerColor == EPieceColor::White)
		{
			_PlayerState->bIsAttaker = true;
			for (int i = 0; i < AllStartPoints.Num(); ++i)
			{
				if (AllStartPoints[i]->PlayerStartTag == TEXT("Tagger"))
				{
					return AllStartPoints[i];
				}
			}
		}
		else
		{
			_PlayerState->bIsAttaker = false;
			for (int i = 0; i < AllStartPoints.Num(); ++i)
			{
				if (AllStartPoints[i]->PlayerStartTag == TEXT("Bishop"))
				{
					return AllStartPoints[i];
				}
			}
		}
	}
	// 게임모드 안에서 LocalPlayer가 아니다? 클라이언트다 => 검은색
	else
	{
		_PlayerState->PieceColor = EPieceColor::Black;

		if (_NoirBlancGameInstance->AttackerColor == EPieceColor::Black)
		{
			_PlayerState->bIsAttaker = true;
			for (int i = 0; i < AllStartPoints.Num(); ++i)
			{
				if (AllStartPoints[i]->PlayerStartTag == TEXT("Tagger"))
				{
					return AllStartPoints[i];
				}
			}
		}
		else
		{
			_PlayerState->bIsAttaker = false;
			for (int i = 0; i < AllStartPoints.Num(); ++i)
			{
				if (AllStartPoints[i]->PlayerStartTag == TEXT("Bishop"))
				{
					return AllStartPoints[i];
				}
			}
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

UClass* ABishopGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ANoirBlancPlayerState* PlayerState = InController->GetPlayerState<ANoirBlancPlayerState>();
	if (PlayerState)
	{
		// If Bishop
		if (PlayerState->bIsAttaker == false)
		{
			return BishopPawnClass;
		}
		// If Tagger
		else
		{
			return TaggerPlayerClass;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState : NULL"));
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ABishopGameMode::BeginPlay()
{
	Super::BeginPlay();

	FText RandomText = PickRandomText();
	CurrentTextToType = RandomText;

	CurrentRemainTime = DefaultRemainTime;
}

void ABishopGameMode::NotifyJoined(APlayerController* JoinedPlayer)
{
	JoinedPlayers.Add(JoinedPlayer);
	// 2명 들어왔으면 게임 시작 (GameMode에게 게임 시작하라고 알려주기)
	if (JoinedPlayers.Num() >= 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("두 명 다 들어옴!"));
		// 게임 시작
		// 1. 3, 2, 1 GO 타이머 고고
		GetWorld()->GetTimerManager().SetTimer
			(
			 StartCountDownTimerHandle,
			 this,
			 &ABishopGameMode::StartCountTimer,
			 1.f,
			 true
			);
	}
}

void ABishopGameMode::StartCountTimer()
{
	// 카운트 다운 내리자!
	if (CountdownNumber > -1)
	{
		--CountdownNumber;
		for (int i = 0; i < JoinedPlayers.Num(); ++i)
		{
			// UIUpdatable 인터페이스 구현 여부 확인
			if (JoinedPlayers[i]->GetPawn()->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
			{
				if (CountdownNumber > 0)
				{
					IUIUpdatable::Execute_MulticastRPC_UpdateStartCountdownUI
						(
						 JoinedPlayers[i]->GetPawn(),
						 FText::AsNumber(CountdownNumber)
						);
				}
				else
				{
					IUIUpdatable::Execute_MulticastRPC_UpdateStartCountdownUI
						(
						 JoinedPlayers[i]->GetPawn(),
						 FText::FromString(TEXT("GO!"))
						);
				}
			}
		}
	}

	// Countdown Number가 3, 2, 1, 0 땡하면
	// 게임을 시작하자!
	if (CountdownNumber == -1)
	{
		GetWorld()->GetTimerManager().ClearTimer(StartCountDownTimerHandle); // 타이머 클리어

		for (int i = 0; i < JoinedPlayers.Num(); ++i)
		{
			// UIUpdatable 인터페이스 구현 여부 확인
			if (JoinedPlayers[i]->GetPawn()->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
			{
				// 3. 모든 플레이어의 입력 제한 해제
				IUIUpdatable::Execute_MulticastRPC_SetInput(JoinedPlayers[i]->GetPawn(), true);

				// 4. Wait UI 삭제하고
				IUIUpdatable::Execute_MulticastRPC_UpdateStartCountdownUI(JoinedPlayers[i]->GetPawn(), FText());

				// 5. 게임 UI 띄우기
				IUIUpdatable::Execute_MulticastRPC_InitializeTypingUI(JoinedPlayers[i]->GetPawn());

				// 6. Main Timer 똑딱똑딱 시작
				IUIUpdatable::Execute_MulticastRPC_UpdateMainTimerUI(JoinedPlayers[i]->GetPawn(),
				                                                     FText::AsNumber(CurrentRemainTime));
				GetWorld()->GetTimerManager().SetTimer
					(
					 MainTimerHandle,
					 this,
					 &ABishopGameMode::UpdateTimer,
					 1.f,
					 true
					);
			}
		}
	}
}

void ABishopGameMode::UpdateTimer()
{
	CurrentRemainTime--;

	if (CurrentRemainTime < 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(MainTimerHandle);

		// Game Over
		for (int i = 0; i < JoinedPlayers.Num(); ++i)
		{
			if (JoinedPlayers[i]->GetPawn() == nullptr) return;

			if (JoinedPlayers[i]->GetPawn()->IsA(ABishopPawn::StaticClass()))
			{
				// Game Over 처리 (Pawn 승리)
				GameOver(JoinedPlayers[i]->GetPawn());
			}
		}
		return;
	}

	for (int i = 0; i < JoinedPlayers.Num(); ++i)
	{
		// UIUpdatable 인터페이스 구현 여부 확인
		if (JoinedPlayers[i]->GetPawn()->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
		{
			IUIUpdatable::Execute_MulticastRPC_UpdateMainTimerUI(JoinedPlayers[i]->GetPawn(),
			                                                     FText::AsNumber(CurrentRemainTime));
		}
	}
}

bool ABishopGameMode::CheckCommittedText(const FText& TypedText)
{
	// 현재 입력한 문장이 제시된 문장과 일치하는지 검사
	return CurrentTextToType.EqualTo(TypedText);
}

void ABishopGameMode::UpdateInputtedText(const FText& TypedText)
{
	TArray<bool> StringCorrectArray = CheckTypingCorrect(TypedText);

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), AllPawns);
	for (AActor* _Pawn : AllPawns)
	{
		// UIUpdatable 인터페이스 구현 여부 확인
		if (_Pawn && _Pawn->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
		{
			// 복붙 했는지 검사
			bool bIsCheatting = CheckCheatting(TypedText);
			if (bIsCheatting)
			{
				// 복붙했으면 입력한 내용 다 초기화 시켜버리기
				UE_LOG(LogTemp, Warning, TEXT("Don't cheat!! GO AWAY!!!!"));
				IUIUpdatable::Execute_MulticastRPC_SetUITextTo
					(
					 _Pawn,
					 FText::FromString(""),
					 CurrentTextToType,
					 TArray<bool>()
					);
				CurrentTypedText = TEXT("");
			}
			else
			{
				CurrentTypedText = TypedText.ToString();
				// 메시지 호출
				IUIUpdatable::Execute_MulticastRPC_SetUITextTo(_Pawn, TypedText, CurrentTextToType, StringCorrectArray);
			}
		}
	}
}

void ABishopGameMode::PickRandomTextAndUpdateUI()
{
	FText RandomText = PickRandomText();

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), AllPawns);
	for (AActor* _Pawn : AllPawns)
	{
		// UIUpdatable 인터페이스 구현 여부 확인
		if (_Pawn && _Pawn->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
		{
			// 메시지 호출
			IUIUpdatable::Execute_MulticastRPC_SetUITextTo
				(
				 _Pawn,
				 FText::FromString(""),
				 CurrentTextToType,
				 TArray<bool>()
				);
		}
	}
}

void ABishopGameMode::CommitText(const FText& TypedText)
{
	bool bIsCorrect = CheckCommittedText(TypedText);
	// UE_LOG(LogTemp, Warning, TEXT("%s / %p"), *TypedText.ToString(),
	//        *CurrentTextToType.ToString());
	// 친 문장이 맞는 경우
	if (bIsCorrect)
	{
		FText _RandomText = PickRandomText(); // 새로운 문장 생성

		TArray<AActor*> AllPawns;
		UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), AllPawns);
		for (AActor* _Pawn : AllPawns)
		{
			ABishopPawn* BishopPawn = Cast<ABishopPawn>(_Pawn);
			if (BishopPawn)
			{
				// 무기 소환!
				FVector SpawnPoint = BishopPawn->WeaponSpawnPoint->GetComponentLocation();
				FRotator SpawnRotation = BishopPawn->WeaponSpawnPoint->GetComponentRotation();
				BishopPawn->MulticastRPC_SpawnWeapon(SpawnPoint, SpawnRotation, BishopWeaponClass);
			}

			// UIUpdatable 인터페이스 구현 여부 확인
			if (_Pawn && _Pawn->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
			{
				// 메시지 호출
				IUIUpdatable::Execute_MulticastRPC_SetUITextTo
					(
					 _Pawn,
					 FText::FromString(""),
					 _RandomText,
					 TArray<bool>()
					);
			}
		}
	}
	// 친 문장이 틀린 경우
	else
	{
		// TODO: 무엇을 할까?
	}
}


FText ABishopGameMode::PickRandomText()
{
	if (TextsToType.IsEmpty()) return FText();

	FText RandomText;
	do
	{
		const int32 MaxNum = TextsToType.Num();
		const int32 RandomNumber = FMath::RandRange(0, MaxNum - 1);
		RandomText = TextsToType[RandomNumber];
		// UE_LOG(LogTemp, Warning, TEXT("Current Text: %s"), *CurrentTextToType.ToString());
		// UE_LOG(LogTemp, Warning, TEXT("Random Text: %s"), *RandomText.ToString());
	}
	while (TextsToType.Num() >= 2 && RandomText.EqualTo(CurrentTextToType));
	CurrentTextToType = RandomText;

	return RandomText;
}

void ABishopGameMode::GameOver(APawn* Winner)
{
	GetWorld()->GetTimerManager().ClearTimer(StartCountDownTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(MainTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("들어옴!"));
	// UIUpdatable 인터페이스 구현 여부 확인
	if (Winner->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
	{
		UNoirBlancGameInstance* _NoirBlancGameInstance = GetGameInstance<UNoirBlancGameInstance>();
		if (_NoirBlancGameInstance)
		{
			EPieceColor _WinnerColor = IUIUpdatable::Execute_GetPieceColor(Winner);
			for (int i = 0; i < JoinedPlayers.Num(); ++i)
			{
				// UIUpdatable 인터페이스 구현 여부 확인
				if (JoinedPlayers[i]->GetPawn()->GetClass()->ImplementsInterface(UUIUpdatable::StaticClass()))
				{
					IUIUpdatable::Execute_MulticastRPC_SetWinner(JoinedPlayers[i]->GetPawn(), _WinnerColor);

					// 승리자가 아닐 경우 Destory!
					if (JoinedPlayers[i]->GetPawn() && JoinedPlayers[i]->GetPawn() != Winner)
					{
						JoinedPlayers[i]->GetPawn()->Destroy();
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NoirBlanc Game Instance Not Exist!!!"));
		}
	}
}

bool ABishopGameMode::CheckCheatting(const FText& TypedText)
{
	// 이미 입력된 것을 한번에 지우는 것은 제외
	if (CurrentTypedText.Len() > TypedText.ToString().Len())
	{
		return false;
	}

	// 다른 곳에서 바로 복붙하는 것을 방지
	// Commit된 텍스트가 방금 전 Update 된 Text보다 2 ~ 3글자 이상 차이 난다면?
	// -> 복붙했을 가능성이 높음
	int Difference = FMath::Abs(TypedText.ToString().Len() - CurrentTypedText.Len());
	if (Difference >= 2)
	{
		return true; // Cheatting!!
	}
	return false; // not cheatting
}

void ABishopGameMode::OnTaggerOverlapped(AActor* OtherActor)
{
	ATaggerCharacter* TaggerCharacter = Cast<ATaggerCharacter>(OtherActor);
	if (TaggerCharacter == nullptr) return;
	if (TaggerCharacter->GetMovementComponent()->IsFalling() == false) return;

	// Game Over : Bishop Win
	for (APlayerController* JoinedPlayer : JoinedPlayers)
	{
		if (JoinedPlayer->GetPawn()->IsA(ABishopPawn::StaticClass()))
		{
			GameOver(JoinedPlayer->GetPawn());
			return;
		}
	}
}

void ABishopGameMode::OnButtonPressed()
{
	for (int i = 0; i < JoinedPlayers.Num(); ++i)
	{
		if (JoinedPlayers[i]->GetPawn() == nullptr) return;

		if (JoinedPlayers[i]->GetPawn()->IsA(ATaggerCharacter::StaticClass()))
		{
			// Game Over 처리 (Tagger 승리)
			GameOver(JoinedPlayers[i]->GetPawn());
		}
	}
}

TArray<bool> ABishopGameMode::CheckTypingCorrect(const FText& TypedText)
{
	TArray<bool> StringCorrectArray;
	for (int i = 0; i < TypedText.ToString().Len() && i < CurrentTextToType.ToString().Len(); i++)
	{
		if (TypedText.ToString()[i] == CurrentTextToType.ToString()[i])
		{
			StringCorrectArray.Add(true);
		}
		else
		{
			StringCorrectArray.Add(false);
		}
	}

	return StringCorrectArray;
}
