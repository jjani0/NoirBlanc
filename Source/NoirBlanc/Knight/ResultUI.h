// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PieceTypes.h"
#include "Blueprint/UserWidget.h"
#include "ResultUI.generated.h"

/**
 * 
 */
UCLASS()
class NOIRBLANC_API UResultUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* ResultAnim1;

	void ShowResult(EPieceType Attacker, EPieceColor AttackColor, EPieceType Defender, EPieceColor DefendColor, EPieceColor Winner);
};