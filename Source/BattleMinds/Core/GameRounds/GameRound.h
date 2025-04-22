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
class ABM_PlayerControllerBase;

UCLASS(Abstract, Blueprintable)
class BATTLEMINDS_API UGameRound : public UObject, public IBMPostQuestionPhase,
public IBMPrePlayerTurnInterface
{
	GENERATED_BODY()

public:

	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager);
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState);
	virtual void PassTurnToTheNextPlayer();
	virtual TMap<int32, EQuestionResult> VerifyChooseAnswers(FInstancedStruct& LastQuestion, TArray<FInstancedStruct>& PlayerCurrentChoices, int32 QuestionNumber);
	virtual TMap<int32, EQuestionResult> VerifyShotAnswers();
	virtual void HandleQuestionResults(EAnsweredPlayer AnsweredPlayer);
	virtual void WrapUpCurrentPlayersCycle();
	virtual void PrepareNextTurn();
	virtual void Exit(EGameRound NextRound);
	/*~Base Game flow methods */

	/* Base auxiliary methods for variables initialization*/
	virtual void ConstructPlayerTurnsCycles();

	/* Post Question Phase interface */
	/*~Post Question Phase interface */

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Round")
	ABM_GameStateBase* OwnerGameState = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Round")
	ABM_TileManager* TileManager = nullptr;

	UFUNCTION()
	bool IsValidPlayerIndex(int32 IndexToCheck) const;
};
