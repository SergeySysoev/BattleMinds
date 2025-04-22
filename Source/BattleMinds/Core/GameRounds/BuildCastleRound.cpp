// Battle Minds, 2022. All rights reserved.

#include "BuildCastleRound.h"
#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerState.h"

void UBuildCastleRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	Super::Enter(InGameState, InTileManager);
	PassTurnToTheNextPlayer();
}

void UBuildCastleRound::ConstructPlayerTurnsCycles()
{
	TArray<FPlayersCycle> LPlayersCycles;
	FPermutation LPermutation;
	for (int32 i = 0; i < OwnerGameState->PlayerArray.Num(); i++)
	{
		LPermutation.Values.Add(i);
	}
	LPlayersCycles.Add(FPlayersCycle(0, LPermutation, false));
	OwnerGameState->SetCurrentPlayerCycles(LPlayersCycles);
	Super::ConstructPlayerTurnsCycles(); // Call Update of the PlayerCycles widget
}

void UBuildCastleRound::HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState)
{
	/* Player chose their Castle tile
	* so add it to their territory
	and pass the turn to the nex Player when Castle mesh is spawned*/
	TileManager->SC_AddClickedTileToTheTerritory(OwnerGameState->GetCurrentPlayerIndex(), ETileStatus::Castle, CurrentPlayerState->GetPlayerColor(), EGameRound::BuildCastle);
	TileManager->SC_ResetFirstAvailableTile();
	TMap<int32, int32> LPointsMap;
	LPointsMap.Add(OwnerGameState->GetCurrentPlayerIndex(), TileManager->GetPointsOfTile(InClickedTile));
	OwnerGameState->ChangePlayerPoints(LPointsMap);
	TileManager->BindPassTurnToTileCastleMeshSpawned(InClickedTile);
}

void UBuildCastleRound::WrapUpCurrentPlayersCycle()
{
	Super::WrapUpCurrentPlayersCycle();
	Exit(EGameRound::SetTerritory);
}

void UBuildCastleRound::PrepareNextTurn()
{
	PassTurnToTheNextPlayer();
}

void UBuildCastleRound::Exit(EGameRound NextRound)
{
	Super::Exit(EGameRound::SetTerritory);
}
