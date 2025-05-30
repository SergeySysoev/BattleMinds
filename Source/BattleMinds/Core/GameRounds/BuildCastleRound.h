﻿// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameRound.h"
#include "BuildCastleRound.generated.h"


UCLASS()
class BATTLEMINDS_API UBuildCastleRound : public UGameRound
{
	GENERATED_BODY()

public:

	/* Base Game flow methods */
	virtual void Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager) override;
	virtual void ConstructPlayerTurnsCycles() override;
	virtual void HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState) override;
	virtual void WrapUpCurrentPlayersCycle() override;
	virtual void PrepareNextTurn() override;
	virtual void Exit(EGameRound NextRound) override;
	/*~Base Game flow methods */
};
