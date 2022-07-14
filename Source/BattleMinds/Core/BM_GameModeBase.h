// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "BM_GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API ABM_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Game settings")
	EGameRound Round;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	FTimerHandle QuestionTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float QuestionTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float TurnTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfActivePlayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialAttackMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> CastleMaterialMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, FString> NicknameMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	int32 CurrentPlayerID = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	FTimerHandle PlayerTurnHandle;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players settings")
	TArray<FQuestion> CurrentAnsweredQuestions;
	UFUNCTION(BlueprintCallable)
	void InitPlayer(APlayerController* NewPlayer);
	UFUNCTION(BlueprintCallable)
	void OpenQuestion();
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();
	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer(int32 PlayerID);
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableTileForPlayer(int32 PlayerID);
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerTurnTimers();
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(BlueprintReadWrite, Category = "Questions")
	TMap<FName, FQuestion> UsedQuestions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTables;
	
	float CurrentTurnTimer;
};
