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

DECLARE_LOG_CATEGORY_EXTERN(LogBM_GameStateBase, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnswerSentSignature, int32, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPointsChanged, int32, PlayerID, float, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestionCompleted, FPostQuestionPhaseInfo, PostQuestionPhaseInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrePlayerTurnPhaseStarted, FPrePlayerTurnPhaseInfo, PrePlayerTurnPhaseInfo);

class ABM_PlayerState;

UCLASS()
class BATTLEMINDS_API ABM_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Players info")
	FAnswerSentSignature OnAnswerSent;
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Game flow")
	FOnQuestionCompleted OnQuestionCompleted;
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Players info")
	FOnPlayerPointsChanged OnPlayerPointsChanged;

	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Game flow")
	FOnPrePlayerTurnPhaseStarted OnPrePlayerTurnPhaseStarted;
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetCurrentPlayerCounter() const { return CurrentPlayerCounter; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetCurrentPlayerIndex() const { return CurrentPlayerIndex; }

	UFUNCTION(BlueprintPure)
	bool IsPlayerTurn(int32 PlayerIndex) const;
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE EGameRound GetCurrentRound() const { return Round; }

	UFUNCTION(BlueprintPure)
	TArray<FIntPoint> GetPlayerAvailableTiles(EGameRound CurrentRound, int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure)
	FLinearColor GetPlayerLinearColorByIndex(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure)
	EColor GetPlayerColorByIndex(int32 PlayerIndex) const;

	UFUNCTION()
	void HandleClickedTile(FIntPoint InClickedTile);

	UFUNCTION(BlueprintCallable)
	void SetDefendingPlayer(FIntPoint InClickedTile);

	UFUNCTION(BlueprintCallable)
	void ClearPlayerTurnTimer();

	UFUNCTION()
	void EliminatePlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable)
	void RequestToOpenQuestion(EQuestionType QuestionType);

	UFUNCTION(BlueprintCallable)
	void StartSiege();

	UFUNCTION(BlueprintCallable)
	void OpenChooseQuestion();

	UFUNCTION(BlueprintCallable)
	void StartPrePlayerTurnPhase();
	
	UFUNCTION(BlueprintCallable)
	void PassTurnToTheNextPlayer();

	UFUNCTION(BlueprintCallable)
	void PassTurnToTheShotQuestionWinner();

	UFUNCTION(BlueprintCallable)
	ABM_PlayerControllerBase* GetPlayerController(int32 PlayerIndex);

	UFUNCTION()
	void InitGameState();

	UFUNCTION()
	void StartPostCastleChosenPhase();
	
	UFUNCTION(BlueprintCallable)
	void OpenQuestion(EQuestionType QuestionType);

	UFUNCTION()
	void TransferDefendingPlayerTerritoryToAttacker();

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<FPlayersCycle> GetPlayerCycles() const { return PlayerTurnsCycles;}
	
	UFUNCTION(BlueprintCallable)
	void NotifyPostQuestionPhaseReady();

	UFUNCTION(BlueprintCallable)
	void NotifyPrePlayerTurnPhaseReady();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_ChangePointsOfPlayer(int32 PlayerIndex, int32 PointsIncrement);
	
	FunctionVoidPtr OpenNextQuestionPtr;
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

	/*
	 * integer to iterate through PlayerCycles[i].Permutations array, always go from 0 to Number of players
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players info")
	int32 CurrentPlayerCounter = -1;

	/*
	 * index of player in PlayerArray, is set from PlayerCycles[i].Permutations array using CurrentPlayerCounter
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players info")
	int32 CurrentPlayerIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle PlayerTurnHandle;

	/* Player choices sent (or auto generated) to the LastQuestion */
	UPROPERTY(BlueprintReadWrite, Category="Players info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> PlayersCurrentChoices;

	/*
	 * index of player in PlayerArray used when one player attacks another player tile
	 * is set from Tile->OwningPlayerIndex property, equivalent of CurrentPlayerIndex
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Players info")
	int32 DefendingPlayerIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Game flow", meta=(BaseStruct = "Question"))
	TArray<FInstancedStruct> UsedQuestions;

	UPROPERTY()
	TArray<int32> AnsweringPlayers;
	
	FTimerHandle PauseHandle;
	
	FTimerDelegate QuestionDelegate;

	UPROPERTY()
	float CurrentTurnTimer;
	
	bool bShotQuestionIsNeeded = false;
	
	int32 NumberOfSentAnswers = 0;

	UPROPERTY(BlueprintReadOnly, Category="Game flow")
	int32 RemainingPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category="Game flow")
	TArray<int32> RemainingPlayerIndices;

	UPROPERTY(BlueprintReadWrite)
	const ABM_GameModeBase* BMGameMode = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentSiegeTileQuestionCount = 0;

	UPROPERTY(BlueprintReadWrite)
	ABM_TileManager* TileManager = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayersCycle> PlayerTurnsCycles;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayerTurnsCycle = 0;

	UPROPERTY(BlueprintReadOnly)
	TMap<EGameRound, int32> MaxCyclesPerRound;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void StartPostQuestionPhase(bool bSkipToPostQuestionComplete = false);

	UFUNCTION()
	void CheckPostQuestionPhaseComplete();

	UFUNCTION()
	void CheckPrePlayerTurnPhaseCompleted();

	UFUNCTION()
	void ConstructPlayerTurnsCycles();

	UFUNCTION()
	void DisableTileEdgesHighlight();

	UFUNCTION()
	void UnbindAllOnBannerSpawnedTiles();

	UFUNCTION(BlueprintPure)
	int32 GetNextPlayerArrayIndex();
	
	UFUNCTION(BlueprintPure)
	int32 GetPreviousPlayerArrayIndex();

	UFUNCTION(BlueprintCallable)
	void TogglePlayerTurnTimer(bool ShouldPause);
	
	UFUNCTION()
	void StopPlayerTurnTimer();

	UFUNCTION(BlueprintCallable)
	void OpenNextQuestion();
	
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
	TMap<int32, EQuestionResult> VerifyChooseAnswers();
	
	UFUNCTION()
	TMap<int32, EQuestionResult> VerifyShotAnswers();
	
	UFUNCTION()
	EGameRound GetNextGameRound() const;

	UFUNCTION()
	void SetNextGameRound(EGameRound NewRound);

	UFUNCTION()
	int32 CountNotOwnedTiles();

	UFUNCTION()
	void UpdatePlayersTurnsWidget();

	UFUNCTION()
	void ConvertPlayerCyclesToPlayerCyclesUI(TArray<FPlayersCycleUI>& OutPlayersCyclesUI);
	
	UFUNCTION()
	void WrapUpCurrentPlayersCycle();

	UFUNCTION()
	void UpdatePlayerTurn();

	UFUNCTION()
	void StopHUDAnimations();

	UFUNCTION()
	void UpdateGameMap();

	UFUNCTION()
	void HighlightAvailableTiles(int32 PlayerArrayIndex);

	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer();
	
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableTileForPlayer(int32 PlayerIndex);

	UFUNCTION()
	void ForceChooseAvailableTile(int32 PlayerArrayIndex);
	
	UFUNCTION(BlueprintCallable)
	void CountResults();

	// Find row in the corresponding Question table
	UFUNCTION()
	int32 FindNextQuestion(EQuestionType Question, TArray<FName> Array, FString& ContextString);
	
	UFUNCTION()
	void AssignAnsweringPlayers();

	UFUNCTION()
	void StopAllTimers();

	/*
	 * AnsweredPlayer = 0 - no one gave the right answer
	 * AnsweredPlayer = 1 - Attacker was right
	 * AnsweredPlayer = 2 - Defender was right
	 */
	UFUNCTION()
	void HandleSiegedTile(
		uint8 AnsweredPlayer);

	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void MC_UpdatePoints(int32 PlayerID, float NewScore);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CalculateAndSetMaxCyclesPerRound();

private:

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	int32 CurrentPostQuestionReadyActors;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	int32 ExpectedPostQuestionReadyActors;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	int32 CurrentPrePlayerTurnReadyActors = 0;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	int32 ExpectedPrePlayerTurnReadyActors = 0;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TMap<int32, EQuestionResult> QuestionResults;

	UPROPERTY()
	int32 FightForTheRestTileWinnerIndex = -1;

	UPROPERTY()
	TArray<int32> PlayersToUpdatePoints;
};
