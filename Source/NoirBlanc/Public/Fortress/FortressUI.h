// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cannon.h"
#include "Blueprint/UserWidget.h"

#include "FortressUI.generated.h"


class ACannon;
class UProgressBar;

/**
 * 
 */
UCLASS()
class NOIRBLANC_API UFortressUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* Player1pg;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* Player2pg;

	ACannon* Player1;
	// Cannon* Player2;

	UPROPERTY(BlueprintReadOnly)
	float Player1Percentage;
	
	UPROPERTY(BlueprintReadOnly)
	float Player2Percentage;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// when the cannon's health change, change the progress bar
	UFUNCTION(BlueprintCallable)
	void ChangeHPBar(ACannon* Cannon);

	UFUNCTION(BlueprintCallable)
	void ApplyDamageHPBar(ACannon* Cannon, ACannon*  player);

	UFUNCTION(BlueprintCallable)
	void TakeDamageHPBar(ACannon* Cannon);
};
