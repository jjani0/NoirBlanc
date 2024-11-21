// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceTypes.h"

#include "ChessBoard.generated.h"

class ABoardFloor;
class AChessPiece;
UCLASS()
class NOIRBLANC_API AChessBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChessBoard();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//////////////////////////////////////////
/////Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessBoard")
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessBoard")
	TSubclassOf<ABoardFloor> FloorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessBoard")
	TSubclassOf<AChessPiece> PieceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GamePlay")
	AChessPiece* SelectedPiece;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GamePlay")
	ABoardFloor* TargetFloor;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessBoard")
	TArray<EPieceType> PiecesOnBoard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessBoard")
	TArray<ABoardFloor*> BoardFloors;
private:
	const int32 Chess_Num  = 8;
	bool bIsClickedOnce = false;
	bool bIsClickedTwice = false;

	UPROPERTY()
	class AChessPlayerController* Controller;

	UPROPERTY()
	ABoardFloor* ClickedFloor_1 = nullptr;

	UPROPERTY()
	ABoardFloor* ClickedFloor_2 = nullptr;

//////////////////////////////////////////
/////FUNCTION
public:
	UFUNCTION()
	void ClickFloor();
	void MovePiece();
	
protected:
	UFUNCTION()
	ABoardFloor* SpawnFloor(int32 col, int32 row);

	UFUNCTION()
	void InitBoard();

	UFUNCTION()
	void InitPiece(int32 num, EPieceType type, EPieceColor color);

private:
	
};