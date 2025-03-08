// Battle Minds, 2022. All rights reserved.

#include "BattleMinds/Player/BM_PlayerState.h"
#include "BM_PlayerControllerBase.h"
#include "Core/BM_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BM_GameStateBase.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerState);

void ABM_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_PlayerState, BMPlayerID);
	DOREPLIFETIME(ABM_PlayerState, Nickname);
	DOREPLIFETIME(ABM_PlayerState, Points);
	DOREPLIFETIME(ABM_PlayerState, QuestionChoices);
	DOREPLIFETIME(ABM_PlayerState, CurrentQuestionAnswerSent);
	DOREPLIFETIME(ABM_PlayerState, NumberOfTurns);
	DOREPLIFETIME(ABM_PlayerState, TotalPlace);
	DOREPLIFETIME(ABM_PlayerState, QuestionResults);
}

float ABM_PlayerState::GetPoints() const
{
	return Points;
}

void ABM_PlayerState::SC_AddPoints_Implementation(int32 inPoints)
{
	Points += inPoints;
	if (HasAuthority() && GetNetMode() == NM_ListenServer)
	{
		OnRep_Points();
	}
}

void ABM_PlayerState::SC_AddTileToTerritory_Implementation(ABM_TileBase* InTile, ETileStatus InTileStatus)
{
	if (IsValid(InTile))
	{
		InTile->AddTileToPlayerTerritory(InTileStatus, BMPlayerID, Nickname, GetPlayerColor());
		SC_AddPoints(InTile->GetPoints());
		OwnedTiles.Add(InTile);
	}
}

void ABM_PlayerState::SC_RemoveTileFromTerritory_Implementation(ABM_TileBase* InTile)
{
	if (IsValid(InTile))
	{
		InTile->RemoveTileFromPlayerTerritory();
		if (InTile->GetStatus() == ETileStatus::Castle)
		{
			OnCastleConquered.Broadcast();
		}
		else
		{
			SC_AddPoints(-1 * Points);
		}
		OwnedTiles.RemoveSwap(InTile, true);
	}
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
	int32 LCount = 0;
	for (const FQuestionResult LQuestionResult : QuestionResults)
	{
		if (LQuestionResult.bWasAnswered)
		{
			LCount++;
		}
	}
	return LCount;
}

int32 ABM_PlayerState::GetWrongAnswersNumber()
{	
	int32 LCount = 0;
	for (const FQuestionResult LQuestionResult : QuestionResults)
	{
		if (!LQuestionResult.bWasAnswered)
		{
			LCount++;
		}
	}
	return LCount;
}

void ABM_PlayerState::OnRep_Points()
{
	OnPointsChanged.Broadcast(BMPlayerID, Points);
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

void ABM_PlayerState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("ABMPlayerState BeginPlay()"));
}
