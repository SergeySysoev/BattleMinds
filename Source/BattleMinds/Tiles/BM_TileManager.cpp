// Battle Minds, 2022. All rights reserved.


#include "BM_TileManager.h"
#include "BM_TileBase.h"
#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"

ABM_TileManager::ABM_TileManager()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ABM_TileManager::GenerateMap_Implementation(int32 NumberOfPlayers)
{
	for (const auto LTile: Tiles)
	{
		LTile.Value->OnCastleDestroyed.AddDynamic(this, &ABM_TileManager::OnCastleDestroyed);
	}
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

void ABM_TileManager::SC_ApplyDamageToTile_Implementation(int32 PlayerIndex, int32 InDamage)
{
	ABM_TileBase* LTileToDamage = Tiles.FindRef(ClickedTiles.FindRef(PlayerIndex));
	if (IsValid(LTileToDamage))
	{
		LTileToDamage->SC_ApplyDamage(InDamage);
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

EQuestionType ABM_TileManager::GetNextQuestionTypeOfClickedTile() const
{
	if (IsValid(CurrentClickedTile))
	{
		return CurrentClickedTile->GetTileNextQuestionType();
	}
	return EQuestionType::Choose;
}

int32 ABM_TileManager::GetTileQuestionsCount(FIntPoint TileAxials) const
{
	if (Tiles.Contains(TileAxials))
	{
		int32 LCount = Tiles.FindRef(TileAxials)->GetTileQuestionCount();
		return LCount;
	}
	return 1;
}

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
		LGameState->OnQuestionCompleted.AddUniqueDynamic(this, &ThisClass::HandlePostQuestionPhase);
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

bool ABM_TileManager::IsTileAvailable(FIntPoint TileAxials) const
{
	return CurrentPlayerAvailableTilesAxials.Contains(TileAxials);
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
	GetCurrentPlayerAvailableTilesAxials(GameRound, PlayerID);
	HighlightTilesForPlayer(CurrentPlayerAvailableTilesAxials, InPlayerColor, GameRound);
}

TArray<ABM_TileBase*> ABM_TileManager::GetCurrentPlayerAvailableTiles()
{
	TArray<ABM_TileBase*> AvailableTiles;
	for (const auto LTileAxials : CurrentPlayerAvailableTilesAxials)
	{
		AvailableTiles.Add(Tiles.FindRef(LTileAxials));
	}
	return AvailableTiles;
}

TArray<FIntPoint> ABM_TileManager::GetCurrentPlayerAvailableTilesAxials(EGameRound CurrentRound, int32 PlayerIndex)
{
	CurrentPlayerAvailableTilesAxials.Empty();
	
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
				CurrentPlayerAvailableTilesAxials = GetTilesAvailableForCastle();
		break;
		case EGameRound::SetTerritory:
			/*
			 * 1) Найти все клетки, у которых OwnerID == PlayerID
			 * 2) Найти всех соседей этих клеток в пределах карты
			 */
				CurrentPlayerAvailableTilesAxials = GetTilesAvailableForExpansion(PlayerIndex);
		break;
		case EGameRound::FightForTheRestTiles:
			CurrentPlayerAvailableTilesAxials = GetUncontrolledTiles();
		break;
		case EGameRound::FightForTerritory:
			CurrentPlayerAvailableTilesAxials = GetTilesAvailableForAttack(PlayerIndex);
		break;
		default: break;
	}
	return CurrentPlayerAvailableTilesAxials;
}

TArray<ABM_TileBase*> ABM_TileManager::GetPlayerOwnedTiles_Implementation(int32 PlayerID)
{
	TArray<ABM_TileBase*> PlayerOwnedTiles;
	for (const auto LTileAxials : Tiles)
	{
		if (LTileAxials.Value->GetOwnerPlayerIndex() == PlayerID)
		{
			PlayerOwnedTiles.Add(LTileAxials.Value);
		}
	}
	return PlayerOwnedTiles;
}

TArray<FIntPoint> ABM_TileManager::GetPlayerOwnedTilesAxials_Implementation(int32 PlayerID)
{
	TArray<FIntPoint> PlayerOwnedTiles;
	for (const auto LTileAxials : Tiles)
	{
		if (LTileAxials.Value->GetOwnerPlayerIndex() == PlayerID)
		{
			PlayerOwnedTiles.Add(LTileAxials.Key);
		}
	}
	return PlayerOwnedTiles;
}

TArray<FIntPoint> ABM_TileManager::GetShapeBorder(TArray<FIntPoint> Shape)
{
	TArray<FIntPoint> AvailableTiles;
	return AvailableTiles;
}

void ABM_TileManager::HandlePostQuestionPhase(FPostQuestionPhaseInfo PostQuestionPhaseInfo)
{
	if (HasAuthority())
	{
		if (PostQuestionPhaseInfo.ContainsResultType(EQuestionResult::TileDamaged))
		{
			// Play Castle Damage animation
			if (IsValid(CurrentClickedTile))
			{
				CurrentClickedTile->MC_DamageTile();
				CurrentTilesToHandlePostQuestion = 0;
				ExpectedTilesToHandlePostQuestion = 1;
				if (!CurrentClickedTile->OnCastleTowerDamaged.IsAlreadyBound(this, &ThisClass::CheckForTilePostQuestionHandled))
				{
					CurrentClickedTile->OnCastleTowerDamaged.AddDynamic(this, &ThisClass::CheckForTilePostQuestionHandled);	
				}
			}
			return;
		}
		if (PostQuestionPhaseInfo.ContainsResultType(EQuestionResult::TileDefended))
		{
			// Play Successful Defence animation
			CurrentTilesToHandlePostQuestion = ExpectedTilesToHandlePostQuestion = 0; 
			CheckForTilePostQuestionHandled();
			return;
		}
		if (PostQuestionPhaseInfo.ContainsResultType(EQuestionResult::TileCaptured))
		{
			CurrentClickedTile->MC_DamageTile();
			if (bCastleCaptured)
			{
				/*if (!CurrentClickedTile->OnCastleTowerDamaged.IsAlreadyBound(this, &ThisClass::SC_TransferTerritory))
				{
					CurrentClickedTile->OnCastleTowerDamaged.AddDynamic(this, &ThisClass::SC_TransferTerritory);	
				}*/
				SC_TransferTerritory(CastlePreviousOwnerIndex);
			}
			else
			{
				SwitchTileMaterial();
			}
			return;
		}
		CurrentTilesToHandlePostQuestion = ExpectedTilesToHandlePostQuestion = 0; 
		CheckForTilePostQuestionHandled();
	}
}

void ABM_TileManager::SwitchTileMaterial()
{
	ExpectedTilesToHandlePostQuestion = TilesToSwitchMaterial.Num();
	for (const auto LTileAxials : TilesToSwitchMaterial)
	{
		const auto LTileBase = Tiles.FindRef(LTileAxials);
		LTileBase->SwitchTileMeshMaterialColor();
		if (!LTileBase->OnTileMaterialSwitched.IsAlreadyBound(this, &ThisClass::CheckForTilePostQuestionHandled))
		{
			LTileBase->OnTileMaterialSwitched.AddDynamic(this, &ThisClass::CheckForTilePostQuestionHandled);
		}
	}
}

void ABM_TileManager::SC_TransferTerritory(int32 OldOwnerIndex)
{
	TArray<FIntPoint> LTilesToTransfer = GetPlayerOwnedTilesAxials(OldOwnerIndex);
	TPair<int32, EColor> LPlayerInfo = GetCurrentPlayerInfo();
	int32 LAmountOfPoints = 0;
	for (FIntPoint LTileAxials : LTilesToTransfer)
	{
		if (Tiles.Contains(LTileAxials))
		{
			LAmountOfPoints += GetPointsOfTile(LTileAxials);
			TilesToSwitchMaterial.Add(LTileAxials);
			SC_SetTileOwner(LTileAxials, ETileStatus::Controlled, LPlayerInfo.Key, LPlayerInfo.Value, EGameRound::FightForTerritory);
		}
	}
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->SC_ChangePointsOfPlayer(LPlayerInfo.Key, LAmountOfPoints);
		LGameState->SC_ChangePointsOfPlayer(OldOwnerIndex, -LAmountOfPoints);
	}
	SwitchTileMaterial();
}

void ABM_TileManager::OnCastleDestroyed_Implementation(int32 OwnerPlayerIndex)
{
	bCastleCaptured = true;
	CastlePreviousOwnerIndex = OwnerPlayerIndex;;
}

void ABM_TileManager::CheckForTilePostQuestionHandled()
{
	++CurrentTilesToHandlePostQuestion;
	if (CurrentTilesToHandlePostQuestion >= ExpectedTilesToHandlePostQuestion)
	{
		TilesToSwitchMaterial.Empty();
		ExpectedTilesToHandlePostQuestion = 0;
		CurrentTilesToHandlePostQuestion = 0;
		if (bCastleCaptured)
		{
			ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
			if (IsValid(LGameState))
			{
				LGameState->EliminatePlayer(CastlePreviousOwnerIndex);
			}
			bCastleCaptured = false;
			CastlePreviousOwnerIndex = -1;
		}
		ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
		if (IsValid(LGameState))
		{
			LGameState->NotifyPostQuestionPhaseReady();
		}
	}
}

TPair<int32, EColor> ABM_TileManager::GetCurrentPlayerInfo()
{
	TPair<int32, EColor> LPlayerInfo;
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LPlayerInfo.Key = LGameState->GetCurrentPlayerIndex();
		LPlayerInfo.Value = LGameState->GetPlayerColorByIndex(LPlayerInfo.Key);
	}
	return LPlayerInfo;
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
		CurrentClickedTile = LTileToChange;
		LTileToChange->SC_AttackTile(InPlayerColor);
	}
}

void ABM_TileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
