// Fill out your copyright notice in the Description page of Project Settings.


#include "Fortress/FortressGameMode.h"

#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Fortress/Cannon.h"
#include "Fortress/FortressUI.h"
#include "Kismet/GameplayStatics.h"


AFortressGameMode::AFortressGameMode()
{
	DefaultPawnClass = nullptr;
}

void AFortressGameMode::BeginPlay()
{
	Super::BeginPlay();
}

// bind player controller to each pawn
void AFortressGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	TArray<AActor*> CannonActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACannon::StaticClass(), CannonActors);

	// if it is server
	// it is possible for Newplayer to not be assigned any pawn, even in PostLogin
	if (NewPlayer->GetPawn() != nullptr && NewPlayer->GetPawn()->HasAuthority())
	{
		ACannon* Player1Pawn = Cast<ACannon>(CannonActors[0]);

		if (Player1Pawn != nullptr)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Player1: %s"), *Player1Pawn->GetName());

			APlayerController* Player1Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (Player1Controller != nullptr)
				Player1Controller->Possess(Player1Pawn);
		}
	}
	else
	{
		ACannon* Player2Pawn = Cast<ACannon>(CannonActors[1]);
		if (Player2Pawn != nullptr)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Player2: %s"), *Player2Pawn->GetName());

			// client creates its on client's Player Controller
			APlayerController* Player2Controller = UGameplayStatics::GetPlayerController(GetWorld(), 1);
			if (Player2Controller == nullptr)
				Player2Controller = UGameplayStatics::CreatePlayer(GetWorld(), 1);

			if (Player2Controller != nullptr)
				Player2Controller->Possess(Player2Pawn);
		}
	}
}

void AFortressGameMode::ChangeTurn()
{
	AllPlayers[turnIdx]->bIsturn = false;
	turnIdx++;
	turnIdx %= AllPlayers.Num();
	AllPlayers[turnIdx]->bIsturn = true;

	for (int32 i = 0; i < AllPlayers.Num(); i++)
	{
		// announce turn by widget
		if (turnIdx == 0)
			AllPlayers[i]->turnCannon = FText::FromString(FString(TEXT("Change")));
		else
			AllPlayers[i]->turnCannon = FText::FromString(FString(TEXT("Change")));
	}
}

void AFortressGameMode::SetWind()
{
	float windStrength = FMath::RandRange(-WindMaxStrength, WindMaxStrength);
	FVector windForce = FVector(1, 0, 0) * windStrength;

	for (int32 i = 0; i < AllPlayers.Num(); i++)
	{
		AllPlayers[i]->MulticastRPC_SetWindForce(windForce, WindMaxStrength);
	}
}

void AFortressGameMode::StartCountdown()
{
	ACannon* player = Cast<ACannon>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (player != nullptr)
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownTimer,
			player, &ACannon::MulticastRPC_UpdateCountdown, 1.0f, true);
	}
}
