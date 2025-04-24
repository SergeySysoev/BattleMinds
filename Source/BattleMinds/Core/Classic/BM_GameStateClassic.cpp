// Battle Minds, 2022. All rights reserved.

#include "Core/Classic/BM_GameStateClassic.h"

#include "Core/BM_GameModeBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileManager.h"

void ABM_GameStateClassic::ForceSetGameRound(EGameRound NewRound)
{
	switch (NewRound)
	{
		case EGameRound::BuildCastle:
			StopAllTimers();
			PrepareNextRound(EGameRound::BuildCastle);
			break;
		case EGameRound::SetTerritory:
			break;
		case EGameRound::FightForTheRemains:
			StopAllTimers();
			if (IsValid(TileManager))
			{
				TMap<int32, EColor> PlayerColors;
				for (const auto LPlayerState : PlayerArray)
				{
					ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
					PlayerColors.Add(LBMPlayerState->GetPlayerIndex(), LBMPlayerState->GetPlayerColor());
				}
				TileManager->AutoAssignTerritoryWithEmptyTiles(true, PlayerColors, 4);
			}
			SetPlayersCamerasToDefault();
			PrepareNextRound(EGameRound::FightForTheRemains);
			break;
		case EGameRound::FightForTerritory:
			StopAllTimers();
			if (IsValid(TileManager))
			{
				TMap<int32, EColor> PlayerColors;
				for (const auto LPlayerState : PlayerArray)
				{
					ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
					PlayerColors.Add(LBMPlayerState->GetPlayerIndex(), LBMPlayerState->GetPlayerColor());
				}
				TileManager->AutoAssignTerritory(true, PlayerColors);
			}
			SetPlayersCamerasToDefault();
			PrepareNextRound(EGameRound::FightForTerritory);
			break;
		default: break;
	}

}