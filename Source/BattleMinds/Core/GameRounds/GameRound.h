// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "Interfaces/BMPostQuestionPhase.h"
#include "Interfaces/BMPrePlayerTurnInterface.h"
#include "Tiles/BM_TileManager.h"
#include "UObject/Object.h"
#include "GameRound.generated.h"

class ABM_PlayerState;
class ABM_GameStateBase;
class ABM_CharacterBase;
class ABMCharacterSpawnSlot;
class ABM_PlayerControllerBase;

UCLASS(Abstract, Blueprintable)
class BATTLEMINDS_API UGameRound : public UObject, public IBMPostQuestionPhase,
public IBMPrePlayerTurnInterface
{
	GENERATED_BODY()

public:

	RoundFunctionVoidPtr PassTurnToNextPlayerPtr;
	
	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager);
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState);
	virtual void PassTurnToTheNextPlayer();
	virtual void AssignAnsweringPlayers(TArray<int32>& AnsweringPlayers);
	virtual void GatherPlayerAnswers();
	virtual TMap<int32, EQuestionResult> VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber);
	virtual TMap<int32, EQuestionResult> VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber);
	virtual void ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults);
	virtual void WrapUpCurrentPlayersCycle();
	virtual void PrepareNextTurn();
	virtual void Exit(EGameRound NextRound);
	/*~Base Game flow methods */

	/* Base auxiliary methods for variables initialization or verification*/
	virtual void ConstructPlayerTurnsCycles();
	UFUNCTION()
	void SetCharacterSpawnSlots();
	virtual bool HasMoreTurns() const;
	virtual bool IsShotQuestionNeeded() const;
	int32 GetCurrentPlayerIndex() const; 
	int32 GetCurrentCycle() const; 
	int32 GetCurrentPlayerCounter() const;
	TArray<FPlayersCycle> GetPlayersCycles() const;
	
	UFUNCTION()
	void PlayAnimationOnSpawnedCharacters(TMap<int32, EQuestionResult> QuestionResults);
	
	/* Post Question Phase interface */
	virtual void OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults);
	virtual bool ShouldSkipToPostQuestionComplete() const;
	/*~Post Question Phase interface */

	UFUNCTION()
	FORCEINLINE TArray<FInstancedStruct>& GetPlayerChoices() { return PlayersCurrentChoices; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Round")
	ABM_GameStateBase* OwnerGameState = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Round")
	ABM_TileManager* TileManager = nullptr;

	UFUNCTION()
	bool IsValidPlayerIndex(int32 IndexToCheck) const;

	/* Player choices sent (or auto generated) to the LastQuestion */
	UPROPERTY(BlueprintReadWrite, Category="Players info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> PlayersCurrentChoices;

	/* Points each player received on Question*/
	UPROPERTY()
	TMap<int32, int32> PlayerQuestionPoints;

	/* Player turns order related variables */

	/* integer to iterate through PlayerCycles[i].Permutations array, always go from 0 to Number of players */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players info")
	int32 CurrentPlayerCounter = -1;

	/* index of player in PlayerArray, is set from PlayerCycles[i].Permutations array using CurrentPlayerCounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Players info")
	int32 CurrentPlayerIndex = 0;

	/* Array of Players cycles, each round has its own array constructed based on MaxCycles setting in GameState and GameMode*/
	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayersCycle> PlayerTurnsCycles;

	/* Index of current Cycle from PlayerTurnsCycles array */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayerTurnsCycle = 0;

	UPROPERTY()
	TMap<int32, ABMCharacterSpawnSlot*> CurrentRoundSpawnSlots;

	UPROPERTY()
	TMap<int32, ABM_CharacterBase*> CurrentSpawnedCharacters;

	/*~ Player turns order related variables */

	UFUNCTION()
	int32 GetNextPlayerIndex();

	UFUNCTION()
	void DestroySpawnedCharacters();
};