// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameRound.h"
#include "SetTerritoryRound.generated.h"

UCLASS()
class BATTLEMINDS_API USetTerritoryRound : public UGameRound
{
	GENERATED_BODY()

	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager) override;
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState) override;
	virtual void GatherPlayerAnswers() override;
	virtual TMap<int32, EQuestionResult> VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber) override;
	virtual void ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults) override;
	virtual void WrapUpCurrentPlayersCycle() override;
	virtual bool HasMoreTurns() const override;
	virtual void PrepareNextTurn() override;
	virtual void Exit(EGameRound NextRound) override;
	/*~Base Game flow methods */

	virtual void ConstructPlayerTurnsCycles() override;
};
