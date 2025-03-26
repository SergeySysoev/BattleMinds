// Battle Minds, 2022. All rights reserved.

#include "Core/Classic/BM_GameStateClassic.h"

#include "Core/BM_GameModeBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"
#include "Tiles/BM_TileManager.h"

void ABM_GameStateClassic::ForceSetGameRound(EGameRound NewRound)
{
	SetNextGameRound(NewRound);
	ConstructPlayerTurnsCycles();
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			StopAllTimers();
			CurrentPlayerCounter = 0;
			PassTurnToTheNextPlayer();
			break;
		case EGameRound::SetTerritory:
			break;
		case EGameRound::FightForTheRestTiles:
			StopAllTimers();
			if (IsValid(TileManager))
			{
				TMap<int32, EColor> PlayerColors;
				for (const auto LPlayerState : PlayerArray)
				{
					ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
					PlayerColors.Add(LBMPlayerState->GetPlayerIndex(), LBMPlayerState->GetPlayerColor());
				}
				TileManager->AutoAssignTerritoryWithEmptyTiles(true, PlayerColors, 1);
			}
			CurrentPlayerCounter = 0;
			CurrentPlayerIndex = 0;
			WrapUpCurrentPlayersCycle();
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
			CurrentPlayerCounter = 0;
			PassTurnToTheNextPlayer();
			break;
		default: break;
	}

}