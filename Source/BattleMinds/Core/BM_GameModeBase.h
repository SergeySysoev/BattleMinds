// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "Player/BM_PlayerPawn.h"
#include "Tiles/BM_TileBase.h"
#include "InstancedStruct.h"
#include "BM_GameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_GameMode, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnswerSentSignature, int32, PlayerID);

/*
 *	General game loop:
 *	1) Choose game round: Set Castle, Set Territory, Fight For Territory, Count Results
 *	2) based on the round, assign Player's turn
 *	3) when all Players have made their turn, open a Question
 *	4) wait until all Players submit their choices or until timer ends
 *	5) collect all answers
 *	6) verify collected answers
 *	7) assign points and territory based on the answers
 *	8) repeat until all Players have no more turns
 *	9) count results and assign places
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 QuestionsCount = 0;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="Game settings", meta=(BaseStruct = "Question"))
	FInstancedStruct LastQuestion;
	
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

	/* Player choices sent (or auto generated) to the LastQuestion */
	UPROPERTY(BlueprintReadWrite, Category="Players settings", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> PlayersCurrentChoices;
	
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
	void UpdatePlayersHUD();
	
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

	UFUNCTION()
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

	UPROPERTY(BlueprintReadWrite, Category = "Questions", meta=(BaseStruct = "Question"))
	TArray<FInstancedStruct> UsedQuestions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTablesChoose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Questions")
	TArray<UDataTable*> QuestionTablesShot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<AActor*> Tiles;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Players settings")
	TSubclassOf<ABM_PlayerPawn> PawnClass;

	UPROPERTY()
	TArray<int32> AnsweringPlayers;
	
	FTimerHandle PauseHandle;
	
	FTimerDelegate QuestionDelegate;

	UPROPERTY()
	float CurrentTurnTimer;
	
	int32 NumberOfTotalTurns;
	
	bool bShotQuestionIsNeeded = false;
	
	int32 NumberOfSentAnswers = 0;
};
