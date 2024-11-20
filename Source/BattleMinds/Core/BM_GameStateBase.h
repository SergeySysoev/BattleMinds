// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BM_Types.h"
#include "Camera/CameraActor.h"
#include "BM_GameStateBase.generated.h"

class ABM_PlayerControllerBase;
class ABM_TileBase;
class ABM_GameModeBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnswerSentSignature, int32, PlayerID);

class ABM_PlayerState;

UCLASS()
class BATTLEMINDS_API ABM_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, BlueprintCallable, Category = "Players info")
	FAnswerSentSignature OnAnswerSent;

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetCurrentPlayerID() const { return CurrentPlayerID; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE EGameRound GetCurrentRound() const { return Round; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<ABM_TileBase*> GetCurrentPlayerAvailableTiles() const { return CurrentPlayerAvailableTiles; }

	UFUNCTION(BlueprintCallable)
	void UpdatePlayersHUD();

	UFUNCTION(BlueprintCallable)
	void SetDefendingPlayerID(int32 InID);

	UFUNCTION(BlueprintCallable)
	void RequestToClearPlayerTurnTimer();

	UFUNCTION(BlueprintCallable)
	void RequestToOpenQuestion(EQuestionType QuestionType);

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
	int32 CurrentPlayerID = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game flow")
	FTimerHandle PlayerTurnHandle;

	/* Player choices sent (or auto generated) to the LastQuestion */
	UPROPERTY(BlueprintReadWrite, Category="Players info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> PlayersCurrentChoices;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Players info")
	TArray<ABM_TileBase*> CurrentPlayerAvailableTiles;

	UPROPERTY(BlueprintReadWrite, Category = "Players info")
	int32 DefendingPlayerID;

	UPROPERTY(BlueprintReadWrite, Category = "Game flow", meta=(BaseStruct = "Question"))
	TArray<FInstancedStruct> UsedQuestions;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<AActor*> Tiles;

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
	int32 NumberOfTotalTurns;

	virtual void BeginPlay() override;

	// Move Players Cameras to Question Location
	UFUNCTION()
	void SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const;
	
	UFUNCTION(BlueprintCallable)
	void StartQuestionTimer();
	
	UFUNCTION()
	void ResetQuestionTimer(int32 LastSentPlayer);

	UFUNCTION()
	void GenerateAutoPlayerChoice(ABM_PlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	void GatherPlayersAnswers();

	UFUNCTION(BlueprintCallable)
	void ConstructQuestionResult(ABM_PlayerState* InPlayerState, int32 InQuestionNumber, FInstancedStruct InQuestion, TArray<FInstancedStruct> InPlayerChoices, int32 InReceivedPoints, bool InWasAnswered);
	
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

	UFUNCTION()
	void ResetPlayersTurns(int32 PlayerID);

	UFUNCTION(BlueprintCallable)
	void StartPlayerTurnTimer(int32 PlayerID);
	
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableTileForPlayer(int32 PlayerID);
	
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerTurnTimers();
	
	UFUNCTION(BlueprintCallable)
	void CountResults();

	// Find row in the corresponding Question table
	UFUNCTION()
	int32 FindNextQuestion(EQuestionType Question, TArray<FName> Array, FString& ContextString);
	
	UFUNCTION()
	void AssignAnsweringPlayers();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
