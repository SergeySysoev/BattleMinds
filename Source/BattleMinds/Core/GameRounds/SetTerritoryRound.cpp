// Battle Minds, 2022. All rights reserved.

#include "SetTerritoryRound.h"

#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerState.h"

void USetTerritoryRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	Super::Enter(InGameState, InTileManager);
	PassTurnToTheNextPlayer();
}

void USetTerritoryRound::HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState)
{
	TileManager->SC_AttackTile(InClickedTile, CurrentPlayerState->GetPlayerColor());
	TileManager->BindGameStateToTileBannerMeshSpawned(InClickedTile, OwnerGameState->PassTurnToNextPlayerPtr);
}

TMap<int32, EQuestionResult> USetTerritoryRound::VerifyChooseAnswers(FInstancedStruct& LastQuestion, TArray<FInstancedStruct>& PlayerCurrentChoices, int32 QuestionNumber)
{
	const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	TMap<int32, EQuestionResult> LQuestionResults;
	for (const auto LPlayerChoice : PlayerCurrentChoices)
	{
		const auto LPlayerID =  LPlayerChoice.GetPtr<FPlayerChoice>()->PlayerID;
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[LPlayerID]);
		if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
		{
			// Wrong answer was given
			OwnerGameState->ConstructQuestionResult(LCurrentPlayerState, QuestionNumber, LastQuestion, PlayerCurrentChoices, 0, false);
			LQuestionResults.Add(LPlayerID, EQuestionResult::TileDefended);
		}
		else
		{
			// Correct answer was given
			int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(OwnerGameState->GetCurrentPlayerIndex());
			OwnerGameState->ConstructQuestionResult(LCurrentPlayerState, QuestionNumber, LastQuestion, PlayerCurrentChoices, LPoints, true);
			LCurrentPlayerState->SC_ChangePoints(LPoints);
			LQuestionResults.Add(LPlayerID, EQuestionResult::TileCaptured);
		}
	}
	return LQuestionResults;
}

void USetTerritoryRound::HandleQuestionResults(EAnsweredPlayer AnsweredPlayer)
{
}

void USetTerritoryRound::WrapUpCurrentPlayersCycle()
{
	Super::WrapUpCurrentPlayersCycle();
	OwnerGameState->IncrementCurrentPlayerCycle();
	OwnerGameState->RequestToOpenQuestion(EQuestionType::Choose);
}

void USetTerritoryRound::PrepareNextTurn()
{
	Super::PrepareNextTurn();
	/*
	 * Count NotOwned tiles.
	 * If there are more than 0
	 *		If there are more than Players, continue SetTerritoryRound
	 *		Else start FightForTheRestTiles special round
	 * Else start FightForTerritory round
	 */
	int32 NotOwnedTiles = OwnerGameState->GetNotOwnedTilesCount();	
	if (NotOwnedTiles > 0)
	{
		if(NotOwnedTiles >= OwnerGameState->GetRemainingPlayersCount())
		{
			if (!OwnerGameState->CheckCurrentCycleCounter())
			{
				Exit(EGameRound::FightForTheRemains);
			}
			else
			{
				PassTurnToTheNextPlayer();	
			}
		}
		else
		{
			Exit(EGameRound::FightForTheRemains);
		}
	}
	else
	{
		Exit(EGameRound::FightForTerritory);
	}
}

void USetTerritoryRound::Exit(EGameRound NextRound)
{
	Super::Exit(NextRound);
}

void USetTerritoryRound::ConstructPlayerTurnsCycles()
{
	TArray<int32> LElements;
	TArray<FPlayersCycle> LPlayersCycles;
	for (int32 i = 0; i < OwnerGameState->PlayerArray. Num(); i++)
	{
		LElements.Add(i);
	}
	TArray<FPermutation> LPermutations =
		UBM_Types::GenerateNumberOfPermutations(LElements, OwnerGameState->GetMaxCyclesForRound(EGameRound::SetTerritory));
	for (int32 i = 0; i < LPermutations.Num(); i++)
	{
		LPlayersCycles.Add(FPlayersCycle(i, LPermutations[i], false));
	}
	OwnerGameState->SetCurrentPlayerCycles(LPlayersCycles);
	Super::ConstructPlayerTurnsCycles();
}