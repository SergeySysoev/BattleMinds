// Battle Minds, 2022. All rights reserved.

#include "Core/Classic/BM_GameStateClassic.h"

#include "Core/BM_GameModeBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

void ABM_GameStateClassic::AutoAssignTiles()
{
	/*if(HasAuthority())
	{
		int32 LTempPlayerCounter = 0;
		int32 i = 0, j = Tiles.Num()-1;
		for (int32 q = 0; i < j; q++)
		{
			int32 LIndex = -1;
			if (q%2 == 0)
			{
				LIndex = i;
				i++;
			}
			else
			{
				LIndex = j;
				j--;
			}
			
			TArray<ABM_TileBase*> LCurrentAvailableTiles;
			if (!Tiles.IsValidIndex(LIndex))
			{
				break;
			}
			ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tiles[LIndex]);
			if (BMTile->GetStatus() == ETileStatus::NotOwned)
			{
				bool NeighborsAvailable = true;
				for (const auto NeighborTile: BMTile->NeighbourTiles)
				{
					if(NeighborTile->GetStatus() == ETileStatus::NotOwned && !NeighborTile->bIsAttacked)
					{
						NeighborsAvailable = false;
					}
				}
				if(NeighborsAvailable)
				{
					LCurrentAvailableTiles.Add(BMTile);
				}
			}
			int32 LRandAvailableIndex = FMath::RandRange(0, LCurrentAvailableTiles.Num()-1);
			if (!LCurrentAvailableTiles.IsValidIndex(LRandAvailableIndex))
			{
				break;
			}
			LCurrentAvailableTiles[LRandAvailableIndex]->AddTileToPlayerTerritory(Cast<ABM_PlayerState>(PlayerArray[LTempPlayerCounter]), TODO, TODO, TODO);
			LTempPlayerCounter++;
			if (LTempPlayerCounter >= BMGameMode->GetNumberOfActivePlayers())
			{
				LTempPlayerCounter = 0;
			}
		}
	}
	*/
	
}

void ABM_GameStateClassic::ForceSetGameRound(EGameRound NewRound)
{
	Round = NewRound;
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			StopAllTimers();
			CurrentPlayerID = 0;
			PassTurnToTheNextPlayer();
			break;
		case EGameRound::SetTerritory:
			break;
		case EGameRound::FightForTerritory:
			StopAllTimers();
			AutoAssignTiles();
			CurrentPlayerID = 0;
			PassTurnToTheNextPlayer();
			break;
		default: break;
	}

}