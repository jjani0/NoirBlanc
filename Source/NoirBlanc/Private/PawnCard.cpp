// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnCard.h"
#include "PawnCardDataAsset.h"

// Sets default values
APawnCard::APawnCard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PawnCard StaticMesh"));
	SetRootComponent(StaticMeshComp);
	
	bIsSelectable = false;

	FrontBackState = CardState::Back;
	
	bReplicates = true;
}

// Called when the game starts or when spawned
void APawnCard::BeginPlay()
{
	Super::BeginPlay();

	bIsSelectable = true;
	
	if(PawnCardData)
	{
		PawnCardData->InitMeshMaterial(StaticMeshComp);
	}
}

void APawnCard::ChangeFrontBackState()
{
	if(FrontBackState == CardState::Front)
	{
		FrontBackState = CardState::Back;
	}
	else
	{
		FrontBackState = CardState::Front;
	}
}

void APawnCard::InitCard()
{
	bIsSelectable = true;
	FrontBackState = CardState::Back;
}

int32 APawnCard::MatchingSuccess()
{
	if(PawnCardData)
	{
		PawnCardData->SetMatchingMaterial(StaticMeshComp);
		return PawnCardData->Score;
	}
	return 1;
}

void APawnCard::CancelMatching()
{
	bIsSelectable = true;
	OwnerPlayerState = nullptr;
}
