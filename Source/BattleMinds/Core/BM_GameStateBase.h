// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BM_Types.h"
#include "GameRounds/GameRound.h"
#include "BM_GameStateBase.generated.h"

class UGameRound;
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
	bool IsPlayerTurn(int32 PlayerIndex) const;
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE EGameRound GetCurrentRound() const { return Round; }

	UFUNCTION()
	FORCEINLINE UGameRound* GetCurrentRoundObject() {return CurrentGameRoundObject;}

	UFUNCTION(BlueprintPure)
	TArray<FIntPoint> GetPlayerAvailableTiles(EGameRound CurrentRound, int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure)
	FLinearColor GetPlayerLinearColorByIndex(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure)
	EColor GetPlayerColorByIndex(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentPlayerIndex();

	UFUNCTION()
	void HandleClickedTile(FIntPoint InClickedTile);

	UFUNCTION()
	void EliminatePlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable)
	void RequestToOpenQuestion(EQuestionType QuestionType, float PauseBeforeOpeningQuestion);

	UFUNCTION(BlueprintCallable)
	void StartPrePlayerTurnPhase();

	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer();

	UFUNCTION(BlueprintCallable)
	ABM_PlayerControllerBase* GetPlayerController(int32 PlayerIndex);

	UFUNCTION()
	void InitGameState();

	UFUNCTION()
	int32 GetPointsOfTile(EGameRound GameRound) const;

	UFUNCTION()
	void PrepareNextRound(EGameRound NextRound);

	UFUNCTION()
	void StartPostCastleChosenPhase();

	UFUNCTION(BlueprintCallable)
	void OpenNextQuestion();
	
	UFUNCTION(BlueprintCallable)
	void OpenQuestion(EQuestionType QuestionType);

	UFUNCTION()
	void GenerateAutoPlayerChoice(ABM_PlayerState* PlayerState) const;
	
	UFUNCTION(BlueprintCallable)
	void ConstructQuestionResult(ABM_PlayerState* InPlayerState, int32 InQuestionNumber, FInstancedStruct InQuestion, TArray<FInstancedStruct> InPlayerChoices, int32 InReceivedPoints, bool InWasAnswered);
	
	UFUNCTION(BlueprintCallable)
	void NotifyPostQuestionPhaseReady(UObject* PostQuestionListener);

	void SetPlayersCamerasToDefault();

	UFUNCTION(BlueprintCallable)
	void NotifyPrePlayerTurnPhaseReady();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_ChangePointsOfPlayer(int32 PlayerIndex, int32 PointsIncrement);

	UFUNCTION()
	void ChangePlayerPoints(TMap<int32, int32>& NewPlayersPoints);

	UFUNCTION()
	void UpdatePlayersCyclesWidget();

	UFUNCTION()
	void StopPlayerTurnTimer();

	UFUNCTION()
	void UpdatePlayerTurn();

	UFUNCTION()
	FORCEINLINE int32 GetMaxCyclesForRound(EGameRound InRound) const { return MaxCyclesPerRound.FindRef(InRound);};

	UFUNCTION()
	int32 GetNotOwnedTilesCount();

	UFUNCTION()
	FORCEINLINE int32 GetRemainingPlayersCount() const { return RemainingPlayers;}

	UFUNCTION()
	FORCEINLINE bool IsRemainingPlayer(const int32 IndexToCheck) const { return RemainingPlayerIndices.IsValidIndex(IndexToCheck); }
	
	FunctionVoidPtr OpenNextQuestionPtr;
	FunctionVoidPtr StartSiegePtr;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Game flow")
	EGameRound Round;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle QuestionTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	int32 QuestionsCount = 0;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="Game flow", meta=(BaseStruct = "Question"))
	FInstancedStruct LastQuestion;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle PlayerTurnHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Game flow", meta=(BaseStruct = "Question"))
	TArray<FInstancedStruct> UsedQuestions;

	UPROPERTY()
	TArray<int32> AnsweringPlayers;
	
	FTimerHandle PauseHandle;
	
	FTimerDelegate QuestionDelegate;

	UPROPERTY()
	float CurrentTurnTimer;
	
	int32 NumberOfSentAnswers = 0;

	UPROPERTY(BlueprintReadOnly, Category="Game flow")
	int32 RemainingPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category="Game flow")
	TArray<int32> RemainingPlayerIndices;

	UPROPERTY(BlueprintReadWrite)
	const ABM_GameModeBase* BMGameMode = nullptr;

	UPROPERTY(BlueprintReadWrite)
	ABM_TileManager* TileManager = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TMap<EGameRound, int32> MaxCyclesPerRound;

	virtual void BeginPlay() override;

	template <typename T>
	void SwitchGameRound()
	{
		CurrentGameRoundObject = NewObject<T>(this);
		CurrentGameRoundObject->Enter(this, TileManager);
	}

	UFUNCTION(BlueprintCallable)
	void StartPostQuestionPhase(bool bSkipToPostQuestionComplete = false);

	UFUNCTION()
	void CheckPostQuestionPhaseComplete();

	UFUNCTION()
	void CheckPrePlayerTurnPhaseCompleted();

	UFUNCTION()
	void DisableTileEdgesHighlight();

	UFUNCTION()
	void UnbindAllOnBannerSpawnedTiles();
	
	UFUNCTION(BlueprintCallable)
	void TogglePlayerTurnTimer(bool ShouldPause);
	
	// Move Players Cameras to Question Location
	UFUNCTION()
	void SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const;
	
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	
	UFUNCTION()
	void OnPlayerAnswerSent(int32 LastSentPlayer);

	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerChoicesAndCorrectAnswer(TArray<FInstancedStruct>& CurrentPlayersChoices);

	UFUNCTION()
	void PrepareNextTurn();
	
	UFUNCTION(BlueprintCallable)
	void VerifyAnswers();
	
	UFUNCTION()
	EGameRound GetNextGameRound() const;

	UFUNCTION()
	void SetNextGameRound(EGameRound NewRound);

	UFUNCTION()
	void ConvertPlayerCyclesToPlayerCyclesUI(TArray<FPlayersCycleUI>& OutPlayersCyclesUI, TArray<FPlayersCycle> InPlayersCycles);

	UFUNCTION()
	void StopHUDAnimations();

	UFUNCTION()
	void HighlightAvailableTiles(int32 PlayerArrayIndex);
	
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
	
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void MC_UpdatePoints(int32 PlayerID, float NewScore);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY()
	TObjectPtr<UGameRound> CurrentGameRoundObject;
};
