// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "ActorFactories/ActorFactoryCameraActor.h"
#include "Player/BM_PlayerPawn.h"
#include "Tiles/BM_TileBase.h"
#include "BM_GameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_GameMode, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnswerSentSignature, int32, PlayerID);

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
	UPROPERTY(Replicated, BlueprintReadWrite, Category="Game settings")
	FQuestion LastQuestion;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float TurnTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfActivePlayers = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfPlayerTurns = 6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfTerritoryTurns = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialAttackMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> CastleMaterialMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialNeighborMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, FColor> ColorMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, FString> NicknameMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	int32 CurrentPlayerID = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	FTimerHandle PlayerTurnHandle;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players settings")
	TArray<FQuestion> CurrentAnsweredQuestions;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<ABM_TileBase*> CurrentPlayerAvailableTiles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ChooseQuestionCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ShotQuestionCamera;
	UPROPERTY(BlueprintReadWrite)
	int32 DefendingPlayerID;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable)
	FAnswerSentSignature OnAnswerSent;
	UFUNCTION(BlueprintCallable)
	void InitPlayer(APlayerController* NewPlayer);
	
	UFUNCTION(BlueprintCallable)
	void OpenQuestion(EQuestionType QuestionType);
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	UFUNCTION()
	void ResetQuestionTimer(int32 LastSentPlayer);
	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();
	UFUNCTION(BlueprintCallable)
	void ShowCorrectAnswers();
	void SetNextGameRound();
	UFUNCTION(BlueprintCallable)
	void VerifyAnswers();
	UFUNCTION()
	void VerifyChooseAnswers();
	UFUNCTION()
	void VerifyShotAnswers();
	UFUNCTION()
	EGameRound NextGameRound() const;
	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer(int32 PlayerID);
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableTileForPlayer(int32 PlayerID);
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerTurnTimers();
	UFUNCTION(BlueprintCallable)
	void CountResults();
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;
	// Find row in the corresponding Question table
	UFUNCTION()
	int32 FindNextQuestion(EQuestionType Question, TArray<FName> Array, FString& ContextString);
	UFUNCTION()
	void AssignAnsweringPlayers();
	// Move Players Cameras to Question Location
	UFUNCTION()
	void SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const;

	UPROPERTY(BlueprintReadWrite, Category = "Questions")
	TMap<FName, FQuestion> UsedQuestions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTablesChoose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTablesShot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<AActor*> Tiles;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Players settings")
	TSubclassOf<ABM_PlayerPawn> PawnClass;
	TArray<int32> AnsweringPlayers;
	FTimerHandle PauseHandle;
	FTimerDelegate QuestionDelegate;
	float CurrentTurnTimer;
	int32 NumberOfTotalTurns;
	bool bShotQuestionIsNeeded = false;
	int32 NumberOfSentAnswers = 0;
};
