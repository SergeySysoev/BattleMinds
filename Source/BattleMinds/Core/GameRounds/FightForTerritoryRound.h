// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameRound.h"
#include "FightForTerritoryRound.generated.h"

UCLASS()
class BATTLEMINDS_API UFightForTerritoryRound : public UGameRound
{
	GENERATED_BODY()

public:
	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager) override;
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState) override;
	virtual void AssignAnsweringPlayers(TArray<int32>& AnsweringPlayers) override;
	virtual void GatherPlayerAnswers() override;
	virtual TMap<int32, EQuestionResult> VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber) override;
	virtual TMap<int32, EQuestionResult> VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber) override;
	virtual void ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults) override;
	virtual void WrapUpCurrentPlayersCycle() override;
	virtual bool HasMoreTurns() const override;
	virtual void PrepareNextTurn() override;
	/*~Base Game flow methods */

	virtual void ConstructPlayerTurnsCycles() override;
	virtual bool IsShotQuestionNeeded() const override;

	/* Post Question Phase interface */
	virtual void OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults) override;
	virtual bool ShouldSkipToPostQuestionComplete() const override;
	/*~Post Question Phase interface */
protected:
	UPROPERTY()
	int32 CurrentSiegeTileQuestionCount;

	/*
	 * index of player in PlayerArray used when one player attacks another player tile
	 * is set from Tile->OwningPlayerIndex property, equivalent of CurrentPlayerIndex
	 */
	UPROPERTY()
	int32 DefendingPlayerIndex = -1;

	UPROPERTY()
	bool bShotQuestionIsNeeded = false;

	UFUNCTION()
	void SetDefendingPlayer(FIntPoint InClickedTile);
	
};
