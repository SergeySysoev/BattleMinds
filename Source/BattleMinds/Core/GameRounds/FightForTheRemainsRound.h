// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameRound.h"
#include "FightForTheRemainsRound.generated.h"

UCLASS()
class BATTLEMINDS_API UFightForTheRemainsRound : public UGameRound
{
	GENERATED_BODY()

	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager) override;
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState) override;
	virtual void PassTurnToTheNextPlayer() override;
	virtual void GatherPlayerAnswers() override;
	virtual TMap<int32, EQuestionResult> VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber) override;
	virtual void OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults) override;
	virtual void WrapUpCurrentPlayersCycle() override;
	virtual void PrepareNextTurn() override;
	/*~Base Game flow methods */

	/* Base auxiliary methods for variables initialization*/
	virtual void ConstructPlayerTurnsCycles() override;
	virtual bool HasMoreTurns() const override;

	virtual bool ShouldSkipToPostQuestionComplete() const override;

protected:
	UPROPERTY()
	int32 FightForTheRestTileWinnerIndex;
};
