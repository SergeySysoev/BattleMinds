// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BM_Types.h"
#include "Camera/CameraActor.h"
#include "Player/BM_PlayerControllerBase.h"
#include "BM_GameStateBase.generated.h"

class ABM_TileManager;
class ABM_PlayerControllerBase;
class ABM_TileBase;
class ABM_GameModeBase;
class ABM_PlayerStateBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnswerSentSignature, int32, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPointsChanged, int32, PlayerID, float, NewScore);
class ABM_PlayerState;

UCLASS()
class BATTLEMINDS_API ABM_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Players info")
	FAnswerSentSignature OnAnswerSent;
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Players info")
	FOnPlayerPointsChanged OnPlayerPointsChanged;

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetCurrentPlayerID() const { return CurrentPlayerCounter; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE EGameRound GetCurrentRound() const { return Round; }

	UFUNCTION(BlueprintPure)
	TArray<FIntPoint> GetCurrentPlayerAvailableTiles() const;

	UFUNCTION(BlueprintPure)
	FLinearColor GetPlayerColorByID(int32 PlayerID) const;

	UFUNCTION()
	void HandleClickedTile(FIntPoint InClickedTile);

	UFUNCTION(BlueprintCallable)
	void SetDefendingPlayer(FIntPoint InClickedTile);

	UFUNCTION(BlueprintCallable)
	void ClearPlayerTurnTimer();

	UFUNCTION(BlueprintCallable)
	void RequestToOpenQuestion(EQuestionType QuestionType);

	UFUNCTION(BlueprintCallable)
	void StartSiege();

	UFUNCTION(BlueprintCallable)
	void OpenChooseQuestion();
	
	UFUNCTION(BlueprintCallable)
	void PassTurnToTheNextPlayer();

	UFUNCTION(BlueprintCallable)
	void RequestToStartPlayerTurnTimer(int32 PlayerID);

	UFUNCTION(BlueprintCallable)
	ABM_PlayerControllerBase* GetPlayerController(int32 PlayerID);

	UFUNCTION()
	void InitGameState();
	
	UFUNCTION(BlueprintCallable)
	void OpenQuestion(EQuestionType QuestionType);

	UFUNCTION()
	void TransferDefendingPlayerTerritoryToAttacker();
	
	FunctionVoidPtr OpenChooseQuestionPtr;
	FunctionVoidPtr StartSiegePtr;
	FunctionVoidPtr PassTurnToNextPlayerPtr;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Game flow")
	EGameRound Round;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle QuestionTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	int32 QuestionsCount = 0;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="Game flow", meta=(BaseStruct = "Question"))
	FInstancedStruct LastQuestion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players info")
	int32 CurrentPlayerCounter = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle PlayerTurnHandle;

	/* Player choices sent (or auto generated) to the LastQuestion */
	UPROPERTY(BlueprintReadWrite, Category="Players info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> PlayersCurrentChoices;

	UPROPERTY(BlueprintReadWrite, Category = "Players info")
	int32 DefendingPlayerID;

	UPROPERTY(BlueprintReadWrite, Category = "Game flow", meta=(BaseStruct = "Question"))
	TArray<FInstancedStruct> UsedQuestions;
	
	/* Order of Player's turn in IDs*/
	UPROPERTY(BlueprintReadOnly, Category = "Players info")
	TArray<int32> PlayerTurns;

	UPROPERTY()
	TArray<int32> AnsweringPlayers;
	
	FTimerHandle PauseHandle;
	
	FTimerDelegate QuestionDelegate;

	UPROPERTY()
	float CurrentTurnTimer;
	
	bool bShotQuestionIsNeeded = false;
	
	int32 NumberOfSentAnswers = 0;

	UPROPERTY(BlueprintReadWrite)
	const ABM_GameModeBase* BMGameMode = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalSetTerritoryTurns = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalFightForTerritoryTurns = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentSiegeTileQuestionCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalSiegeTileQuestionCount = 0;

	UPROPERTY(BlueprintReadWrite)
	ABM_TileManager* TileManager = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayersCycle> PlayerTurnsCycles;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayerTurnsCycle = 0;

	UPROPERTY(BlueprintReadOnly)
	TMap<EGameRound, int32> MaxCyclesPerRound;

	virtual void BeginPlay() override;

	UFUNCTION()
	void ConstructPlayerTurnsCycles();

	UFUNCTION()
	void DisableTileEdgesHighlight();

	UFUNCTION()
	void UnbindAllOnBannerSpawnedTiles();

	UFUNCTION(BlueprintPure)
	int32 GetNextPlayerArrayIndex() const;
	
	UFUNCTION(BlueprintPure)
	int32 GetPreviousPlayerArrayIndex() const;

	UFUNCTION()
	void StopPlayerTurnTimer();
	
	// Move Players Cameras to Question Location
	UFUNCTION()
	void SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const;
	
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	
	UFUNCTION()
	void OnPlayerAnswerSent(int32 LastSentPlayer);

	UFUNCTION()
	void GenerateAutoPlayerChoice(ABM_PlayerState* PlayerState) const;

	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();

	UFUNCTION(BlueprintCallable)
	void ConstructQuestionResult(ABM_PlayerState* InPlayerState, int32 InQuestionNumber, FInstancedStruct InQuestion, TArray<FInstancedStruct> InPlayerChoices, int32 InReceivedPoints, bool InWasAnswered);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerChoicesAndCorrectAnswer();

	UFUNCTION()
	void PrepareNextTurn();
	
	UFUNCTION(BlueprintCallable)
	void VerifyAnswers();
	
	UFUNCTION()
	void VerifyChooseAnswers();
	
	UFUNCTION()
	void VerifyShotAnswers();
	
	UFUNCTION()
	EGameRound GetNextGameRound() const;

	UFUNCTION()
	void SetNextGameRound(EGameRound NewRound);

	UFUNCTION()
	int32 CountNotOwnedTiles();

	UFUNCTION()
	void WrapUpCurrentPlayersCycle();

	UFUNCTION()
	void UpdatePlayersTurnTimerAndNickname(int32 PlayerID);

	UFUNCTION()
	void HighlightAvailableTiles(int32 PlayerArrayIndex);

	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer(int32 PlayerArrayIndex);
	
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableTileForPlayer(int32 PlayerID);

	UFUNCTION()
	void ForceChooseAvailableTile();
	
	UFUNCTION(BlueprintCallable)
	void CountResults();

	// Find row in the corresponding Question table
	UFUNCTION()
	int32 FindNextQuestion(EQuestionType Question, TArray<FName> Array, FString& ContextString);
	
	UFUNCTION()
	void AssignAnsweringPlayers();

	UFUNCTION()
	void StopAllTimers();

	UFUNCTION()
	void HandleSiegedTile(ABM_PlayerControllerBase* AttackingPlayerController, ABM_PlayerControllerBase* DefendingPlayerController,
		bool QuestionWasAnsweredByAttacker);

	UFUNCTION(NetMulticast, Unreliable)
	void MC_UpdatePoints(int32 PlayerID, float NewScore);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CalculateAndSetMaxCyclesPerRound();

};
