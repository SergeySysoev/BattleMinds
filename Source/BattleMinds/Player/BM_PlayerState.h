// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Core/BM_Types.h"
#include "Tiles/BM_TileBase.h"
#include "BM_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API ABM_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	FString Nickname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	UMaterialInterface* MaterialTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	UMaterialInterface* MaterialCastle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	UMaterialInterface* MaterialAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Stats")
	TArray<ABM_TileBase*> OwnedTiles;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Stats")
	bool bHasChosenTile;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	TArray<FQuestion> AnsweredQuestions;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetPoints();
	UFUNCTION(BlueprintCallable)
	void AddPoints(int32 inPoints);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerTurn();
	UFUNCTION(BlueprintCallable)
	void SetPlayerTurn(bool inTurn);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasArtillery();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetCorrectAnswersNumber();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWrongAnswersNumber();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	float Points;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasArtillery;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasTurn;
	
};
