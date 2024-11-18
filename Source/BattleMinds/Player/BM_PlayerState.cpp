// Battle Minds, 2022. All rights reserved.


#include "BattleMinds/Player/BM_PlayerState.h"

#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"

void ABM_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_PlayerState, BMPlayerID);
	DOREPLIFETIME(ABM_PlayerState, Nickname);
	DOREPLIFETIME(ABM_PlayerState, Points);
	DOREPLIFETIME(ABM_PlayerState, MaterialTile);
	DOREPLIFETIME(ABM_PlayerState, bHasArtillery);
	DOREPLIFETIME(ABM_PlayerState, bHasTurn);
	DOREPLIFETIME(ABM_PlayerState, AnsweredQuestions);
	DOREPLIFETIME(ABM_PlayerState, MaterialCastle);
	DOREPLIFETIME(ABM_PlayerState, MaterialAttack);
	DOREPLIFETIME(ABM_PlayerState, MaterialNeighbour);
	DOREPLIFETIME(ABM_PlayerState, PlayerColor);
	DOREPLIFETIME(ABM_PlayerState, CurrentQuestionAnswerSent);
	DOREPLIFETIME(ABM_PlayerState, NumberOfTurns);
	DOREPLIFETIME(ABM_PlayerState, WinnerPosition);
}

TSet<ABM_TileBase*> ABM_PlayerState::GetAllCurrentNeighbours()
{
	TSet<ABM_TileBase*> AllNeighbours;
	for (const auto Tile: OwnedTiles)
	{
		for (const auto Neighbour: Tile->NeighbourTiles)
		{
			AllNeighbours.Add(Neighbour);
		}
	}
	return AllNeighbours;
}

float ABM_PlayerState::GetPoints()
{
	return Points;
}

void ABM_PlayerState::AddPoints_Implementation(int32 inPoints)
{
	Points += inPoints;
}

bool ABM_PlayerState::IsPlayerTurn()
{
	return bHasTurn;
}

void ABM_PlayerState::SetPlayerTurn(bool inTurn)
{
	bHasTurn = inTurn;
}

bool ABM_PlayerState::HasArtillery()
{
	return bHasArtillery;
}
int32 ABM_PlayerState::GetCorrectAnswersNumber()
{
	int32 Count = 0;
	/*for (const auto Questions : AnsweredQuestions)
	{
		if (Questions.bWasAnswered)
		{
			Count++;
		}
	}*/
	return Count;
}

int32 ABM_PlayerState::GetWrongAnswersNumber()
{
	int32 Count = 0;
	/*for (const auto Questions : AnsweredQuestions)
	{
		if (!Questions.bWasAnswered)
		{
			Count++;
		}
	}*/
	return  Count;
}

void ABM_PlayerState::SetPointsInWidget_Implementation()
{
	ABM_PlayerControllerBase* LOwningPC = Cast<ABM_PlayerControllerBase>(GetOwningController());
	if (IsValid(LOwningPC))
	{
		LOwningPC->SC_RequestToUpdateHUD();
	}
}

TSet<ABM_TileBase*> ABM_PlayerState::GetNeighbors()
{
	TSet<ABM_TileBase*> Neighbors;
	for(ABM_TileBase* Tile : OwnedTiles)
	{
		for(ABM_TileBase* Neighbor : Tile->NeighbourTiles)
			Neighbors.Add(Neighbor);
	}
	return Neighbors;
}
