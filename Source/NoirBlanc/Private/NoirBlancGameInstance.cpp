// Fill out your copyright notice in the Description page of Project Settings.


#include "NoirBlancGameInstance.h"
#include "Net/UnrealNetwork.h"

void UNoirBlancGameInstance::Init()
{
	Super::Init();
	
	BoardTypeData = InitTypeValue;
	BoardColorData = InitColorValue;

	for(int i = 0 ; i < 64; i++)
	{
		EncounterCountData.Add(0);
		MoveCountData.Add(0);
	}
}

void UNoirBlancGameInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNoirBlancGameInstance, DeffenderColor);
	DOREPLIFETIME(UNoirBlancGameInstance, DeffenderType);
	DOREPLIFETIME(UNoirBlancGameInstance, AttackerColor);
	DOREPLIFETIME(UNoirBlancGameInstance, AttackerType);
	DOREPLIFETIME(UNoirBlancGameInstance, WinnerColor);
	DOREPLIFETIME(UNoirBlancGameInstance, BoardTypeData);
	DOREPLIFETIME(UNoirBlancGameInstance, BoardColorData);
	DOREPLIFETIME(UNoirBlancGameInstance, EncounterCountData);
	DOREPLIFETIME(UNoirBlancGameInstance, MoveCountData);
	DOREPLIFETIME(UNoirBlancGameInstance, Turn);
	
}
