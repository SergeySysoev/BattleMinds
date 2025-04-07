// Battle Minds, 2022. All rights reserved.


#include "BM_TileManager.h"
#include "BM_TileBase.h"
#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"

ABM_TileManager::ABM_TileManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABM_TileManager::GenerateMap_Implementation(int32 NumberOfPlayers)
{
	OnMapGeneratedNative.Broadcast();
}

void ABM_TileManager::HandleClickedTile(int32 PlayerID, FIntPoint ClickedTileAxials)
{
	if (Tiles.Contains(ClickedTileAxials))
	{
		ClickedTiles.Add(PlayerID, ClickedTileAxials);
	}
}

int32 ABM_TileManager::GetTileOwnerID(FIntPoint TileAxials) const
{
	if (Tiles.Contains(TileAxials))
	{
		return Tiles.FindRef(TileAxials)->GetOwningPlayerIndex();
	}
	return -1;
}

void ABM_TileManager::SC_AddClickedTileToTheTerritory_Implementation(int32 PlayerID, ETileStatus NewStatus, EColor NewColor, EGameRound CurrentGameRound)
{
	TilesToSwitchMaterial.Add(ClickedTiles.FindRef(PlayerID));
	SC_SetTileOwner(ClickedTiles.FindRef(PlayerID),NewStatus, PlayerID, NewColor, CurrentGameRound);
}

void ABM_TileManager::SC_CancelAttackOnClickedTile_Implementation(int32 PlayerID)
{
	FIntPoint PlayerTileAxials = ClickedTiles.FindRef(PlayerID);
	ABM_TileBase* PlayerTile = Tiles.FindRef(PlayerTileAxials);
	if (IsValid(PlayerTile))
	{
		PlayerTile->SC_CancelAttack();
	}
}

void ABM_TileManager::BindPassTurnToTileCastleMeshSpawned(FIntPoint TileAxials)
{
	ABM_TileBase* LTile = Tiles.FindRef(TileAxials);
	if (IsValid(LTile))
	{
		ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
		LTile->OnCastleMeshSpawned.AddUniqueDynamic(LGameState, &ABM_GameStateBase::PassTurnToTheNextPlayer);
	}
}

void ABM_TileManager::BindGameStateToTileBannerMeshSpawned(FIntPoint TileAxials, FunctionVoidPtr FunctionPointer)
{
	ABM_TileBase* LTile = Tiles.FindRef(TileAxials);
	ABM_GameStateBase* LGameState = GetWorld()->GetGameState<ABM_GameStateBase>();
	LTile->OnBannerMeshSpawnedNative.AddUObject(LGameState, FunctionPointer);
	
}

void ABM_TileManager::UnbindAllOnBannerSpawnedDelegates()
{
	ABM_GameStateBase* LGameState = GetWorld()->GetGameState<ABM_GameStateBase>();
	for (const auto LTile : Tiles)
	{
		LTile.Value->OnBannerMeshSpawned.RemoveAll(LGameState);
		LTile.Value->OnBannerMeshSpawnedNative.RemoveAll(LGameState);
	}
}

void ABM_TileManager::AutoAssignTerritory_Implementation(const bool bAssignCastles, const TMap<int32, EColor>& PlayerColors)
{
	
}

void ABM_TileManager::AutoAssignTerritoryWithEmptyTiles_Implementation(const bool bAssignCastles, const TMap<int32, EColor>& PlayerColors, const int32 AmountOfEmptyTiles)
{
	
}

EGameRound ABM_TileManager::GetTileAnnexedRound(FIntPoint TileAxial) const
{
	if (Tiles.Contains(TileAxial))
	{
		return Tiles.FindRef(TileAxial)->GetAnnexedRound();	
	}
	return EGameRound::End;
}

PRAGMA_DISABLE_OPTIMIZATION
int32 ABM_TileManager::GetTileQuestionsCount(FIntPoint TileAxials) const
{
	if (Tiles.Contains(TileAxials))
	{
		int32 LCount = Tiles.FindRef(TileAxials)->GetTileQuestionCount();
		return LCount;
	}
	return 1;
}
PRAGMA_ENABLE_OPTIMIZATION
int32 ABM_TileManager::GetPointsOfTile(FIntPoint TileAxials) const
{
	ABM_GameModeBase* LGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(LGameMode) && Tiles.Contains(TileAxials))
	{
		EGameRound LRound = Tiles.FindRef(TileAxials)->GetAnnexedRound();
		return LGameMode->GetPointsOfTile(LRound);
	}
	return 200;
}

int32 ABM_TileManager::GetPointsOfCurrentClickedTile(int32 PlayerIndex)
{
	if (ClickedTiles.Contains(PlayerIndex))
	{
		return GetPointsOfTile(ClickedTiles.FindRef(PlayerIndex));
	}
	return 200;
}

void ABM_TileManager::SwitchToNextRound(EGameRound NewRound)
{
	switch (NewRound)
	{
		case EGameRound::SetTerritory:
			TilesToSwitchMaterial.Empty();
			ClickedTiles.Empty();
			break;
		default:
			TilesToSwitchMaterial.Empty();
			ClickedTiles.Empty();
			break;
	}
}

void ABM_TileManager::BeginPlay()
{
	Super::BeginPlay();
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->OnQuestionCompleted.AddUniqueDynamic(this, &ThisClass::SwitchTileMaterial);
	}
}

EQuestionType ABM_TileManager::GetTileNextQuestionType(FIntPoint TileAxials)
{
	ABM_TileBase* LTileToChange = Tiles.FindRef(TileAxials);
	if (IsValid(LTileToChange))
	{
		return LTileToChange->GetTileNextQuestionType();
	}
	return EQuestionType::Undefined;
}

void ABM_TileManager::HighlightTilesForPlayer(TArray<FIntPoint> TilesToHighlight, EColor InPlayerColor, EGameRound CurrentRound)
{
	for (FIntPoint LTileAxials : TilesToHighlight)
	{
		ABM_TileBase* LTileToHighlight = Tiles.FindRef(LTileAxials);
		if (IsValid(LTileToHighlight))
		{
			LTileToHighlight->SetTileEdgesColor(InPlayerColor);
			LTileToHighlight->MC_SetBorderVisibility(true);
			if (LTileToHighlight->GetStatus() == ETileStatus::NotOwned)
			{
				ABM_GameModeBase* LGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
				if (IsValid(LGameMode))
				{
					LTileToHighlight->SetPointsWidgetValue(LGameMode->GetPointsOfTile(CurrentRound));	
				}
				else
				{
					LTileToHighlight->SetPointsWidgetValue(200);
				}
			}
			else
			{
				LTileToHighlight->SetPointsWidgetValue(GetPointsOfTile(LTileAxials));
			}
			LTileToHighlight->MC_SetPointsWidgetVisibility(true);
		}
	}
}

void ABM_TileManager::UnhighlightTiles()
{
	for (auto LTile : Tiles)
	{
		if (IsValid(LTile.Value))
		{
			LTile.Value->MC_SetBorderVisibility(false);
			LTile.Value->MC_SetPointsWidgetVisibility(false);
		}
	}
}

void ABM_TileManager::SC_HighlightAvailableTiles_Implementation(EGameRound GameRound, int32 PlayerID, EColor InPlayerColor)
{
	switch (GameRound)
	{
		case EGameRound::ChooseCastle:
			/*
			 * 1) взять границу текущей карты
			 * 2) Найти занятые клетки в этой границе
			 * 3) Найти множество клеток на этой границе без занятых клеток, находищихся на расстоянии 2 от занятых клеток,
			 *  если клеток нет, то уменьшить радиус на 1
			 *  если радиус == 0, то дать любую клетку из границы
			 */
			CurrentPlayerAvailableTiles = GetTilesAvailableForCastle();
			break;
		case EGameRound::SetTerritory:
			/*
			 * 1) Найти все клетки, у которых OwnerID == PlayerID
			 * 2) Найти всех соседей этих клеток в пределах карты
			 */
			CurrentPlayerAvailableTiles = GetTilesAvailableForExpansion(PlayerID);
			break;
		case EGameRound::FightForTheRestTiles:
			CurrentPlayerAvailableTiles = GetUncontrolledTiles();
			break;
		case EGameRound::FightForTerritory:
			CurrentPlayerAvailableTiles = GetTilesAvailableForAttack(PlayerID);
			break;
		default: break;
	}
	HighlightTilesForPlayer(CurrentPlayerAvailableTiles, InPlayerColor, GameRound);
}

TArray<ABM_TileBase*> ABM_TileManager::GetCurrentPlayerAvailableTiles(int32 PlayerID)
{
	TArray<ABM_TileBase*> AvailableTiles;
	for (const auto LTileAxials : CurrentPlayerAvailableTiles)
	{
		AvailableTiles.Add(Tiles.FindRef(LTileAxials));
	}
	return AvailableTiles;
}

TArray<FIntPoint> ABM_TileManager::GetCurrentPlayerAvailableTilesAxials(EGameRound CurrentRound, int32 PlayerIndex)
{
	switch (CurrentRound)
	{
		case EGameRound::ChooseCastle:
			/*
			 * 1) взять границу текущей карты
			 * 2) Найти занятые клетки в этой границе
			 * 3) Найти множество клеток на этой границе без занятых клеток, находищихся на расстоянии 2 от занятых клеток,
			 *  если клеток нет, то уменьшить радиус на 1
			 *  если радиус == 0, то дать любую клетку из границы
			 */
				CurrentPlayerAvailableTiles = GetTilesAvailableForCastle();
		break;
		case EGameRound::SetTerritory:
			/*
			 * 1) Найти все клетки, у которых OwnerID == PlayerID
			 * 2) Найти всех соседей этих клеток в пределах карты
			 */
				CurrentPlayerAvailableTiles = GetTilesAvailableForExpansion(PlayerIndex);
		break;
		case EGameRound::FightForTheRestTiles:
			CurrentPlayerAvailableTiles = GetUncontrolledTiles();
		break;
		case EGameRound::FightForTerritory:
			CurrentPlayerAvailableTiles = GetTilesAvailableForAttack(PlayerIndex);
		break;
		default: break;
	}
	return CurrentPlayerAvailableTiles;
}

TArray<FIntPoint> ABM_TileManager::GetShapeBorder(TArray<FIntPoint> Shape)
{
	TArray<FIntPoint> AvailableTiles;
	return AvailableTiles;
}

void ABM_TileManager::SwitchTileMaterial()
{
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		ExpectedTilesWithSwitchedMaterial = TilesToSwitchMaterial.Num();
		for (const auto LTileAxials : TilesToSwitchMaterial)
		{
			const auto LTileBase = Tiles.FindRef(LTileAxials);
			LTileBase->SwitchTileMeshMaterialColor();
			LTileBase->OnTileMaterialSwitched.AddUniqueDynamic(this, &ThisClass::CheckForTileMaterialsSwitched);
		}
	}
}

void ABM_TileManager::CheckForTileMaterialsSwitched()
{
	++CurrentTilesWithSwitchedMaterial;
	if (CurrentTilesWithSwitchedMaterial >= ExpectedTilesWithSwitchedMaterial)
	{
		TilesToSwitchMaterial.Empty();
		ExpectedTilesWithSwitchedMaterial = 0;
		CurrentTilesWithSwitchedMaterial = 0;
		ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
		if (IsValid(LGameState))
		{
			LGameState->NotifyPostQuestionPhaseReady();
		}
	}
}

void ABM_TileManager::SC_SetTileOwner_Implementation(FIntPoint TileAxials, ETileStatus NewStatus, int32 NewOwnerID, EColor NewOwnerColor, EGameRound CurrentGameRound)
{
	ABM_TileBase* LTileToChange = Tiles.FindRef(TileAxials);
	if (IsValid(LTileToChange))
	{
		LTileToChange->SC_AddTileToPlayerTerritory(NewStatus, NewOwnerID, NewOwnerColor, CurrentGameRound);
	}
}

void ABM_TileManager::SC_AttackTile_Implementation(FIntPoint TileAxials, EColor InPlayerColor)
{
	ABM_TileBase* LTileToChange = Tiles.FindRef(TileAxials);
	if (IsValid(LTileToChange))
	{
		LTileToChange->SC_AttackTile(InPlayerColor);
	}
}

void ABM_TileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

