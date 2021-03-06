// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "Tiles/BM_TileBase.h"
#include "BM_GameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_GameMode, Display, All);
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 QuestionsCount = 0;
	UPROPERTY(BlueprintReadWrite, Category="Game settings")
	FQuestion LastQuestion;
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
	TMap<int32, UMaterialInterface*> MaterialNeighborMap;
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
	void OpenQuestion(EQuestionType QuestionType);
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();
	UFUNCTION(BlueprintCallable)
	void ShowCorrectAnswers();
	UFUNCTION(BlueprintCallable)
	void VerifyAnswers();
	UFUNCTION()
	void VertifyChooseAnswers();
	UFUNCTION()
	void VerifyShotAnswers();
	UFUNCTION()
	EGameRound NextGameRound();
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
	TArray<UDataTable*> QuestionTablesChoose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTablesShot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<AActor*> Tiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<ABM_TileBase*> CurrentPlayerAvailableTiles;

	FTimerHandle PauseHandle;
	FTimerDelegate QuestionDelegate;
	float CurrentTurnTimer;
};
