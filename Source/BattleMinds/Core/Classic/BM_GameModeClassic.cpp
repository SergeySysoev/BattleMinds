// Battle Minds, 2022. All rights reserved.


#include "Core/Classic/BM_GameModeClassic.h"

#include "Core/BM_GameInstance.h"
#include "Core/BM_GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

void ABM_GameModeClassic::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (NumberOfActivePlayers == Cast<UBM_GameInstance>(GetWorld()->GetGameInstance())->NumberOfPlayers)
	{
		Round = EGameRound::ChooseCastle;
		StartChooseCastleRound();
	}
}

void ABM_GameModeClassic::StartChooseCastleRound()
{
	StartChooseCastleTimer();
}

void ABM_GameModeClassic::StartChooseCastleTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Current PlayerID %d"), CurrentPlayerID);
	CurrentTurnTimer = TurnTimer;
	if(CastleTurnTimer.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(CastleTurnTimer);
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer();
		}
	}
	if (CurrentPlayerID < NumberOfActivePlayers)
		GetWorld()->GetTimerManager().SetTimer(CastleTurnTimer, this, &ABM_GameModeClassic::UpdateChooseCastleTimer, 1.0f, true, 1.0f);
	else
	{
		CurrentPlayerID = 0;
		StartSetTerritoryRound();
	}
}

void ABM_GameModeClassic::StartSetTerritoryRound()
{
	Round = EGameRound::SetTerritory;
	CurrentPlayerID = 0;
	StartSetTerritoryTimer();
}

void ABM_GameModeClassic::StartSetTerritoryTimer()
{
	CurrentTurnTimer = TurnTimer;
	GetWorld()->GetTimerManager().SetTimer(TerritoryTurnTimer, this, &ABM_GameModeClassic::UpdateSetTerritoryTimer, 1.0f, true, 1.0f);
}

void ABM_GameModeClassic::UpdateSetTerritoryTimer()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdateTurnTimer();
		}
	}
	CurrentTurnTimer--;
	if(CurrentTurnTimer == 0)
	{
		OpenQuestion();
	}
}

void ABM_GameModeClassic::ChooseFirstAvailableTile()
{
	
}

void ABM_GameModeClassic::UpdateChooseCastleTimer()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdateTurnTimer();
		}
	}
	CurrentTurnTimer--;
	if(CurrentTurnTimer == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CastleTurnTimer);
		ChooseFirstAvailableCastle();
		for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
			{
				PlayerController->ResetTurnTimer();
			}
		}
	}
}

void ABM_GameModeClassic::ChooseFirstAvailableCastle()
{
	TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, Tiles);
	for (auto Tile: Tiles)
	{
		bool NeigboursFree = true;
		auto FoundTile = Cast<ABM_TileBase>(Tile);
		//TODO additional check if this tile is 1-tile close to the current one
		// TArray<ABM_Tiles> NearestTiles = FoundTile->GetNearestTiles();
		// for (Tile : NearestTiles)
		//{ the code below}
		if(FoundTile->GetStatus() == ETileStatus::NotOwned)
		{
			for (const auto NeighbourTile : FoundTile->NeighbourTiles)
			{
				if(NeighbourTile->GetStatus() != ETileStatus::NotOwned)
				{
					NeigboursFree = false;
					break;
				}
			}
			if(NeigboursFree)
			{
				if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]->GetPlayerController()))
				{
					PlayerController->SC_TryClickTheTile(FoundTile);
				}
				if (ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]))
				{
					PlayerState->OwnedTiles.Add(FoundTile);
				}
				break;
			}
		}
			/*if (NeighbourTile->GetStatus() == ETileStatus::NotOwned)
			{
				if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]->GetPlayerController()))
				{
					PlayerController->SC_TryClickTheTile(FoundTile);
				}
				if (ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]))
				{
					PlayerState->OwnedTiles.Add(FoundTile);
				}
				break;
			}*/
	}
}
