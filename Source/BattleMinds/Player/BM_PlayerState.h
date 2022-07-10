// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Core/BM_Types.h"
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetPoints();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerTurn();
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	TArray<FQuestion> AnsweredQuestions;
};
