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
	TileManager->BindRoundToTileBannerMeshSpawned(InClickedTile, PassTurnToNextPlayerPtr);
}

void USetTerritoryRound::GatherPlayerAnswers()
{
	Super::GatherPlayerAnswers();
	
	for (const auto PlayerState : OwnerGameState->PlayerArray)
	{
		const auto LPlayerState = Cast<ABM_PlayerState>(PlayerState);
		// Answers may not be pushed by the Player manually
		if (!IsValid(LPlayerState))
		{
			continue;
		}
		if(LPlayerState->CurrentQuestionAnswerSent == false)	// TODO: not set properly : Testcase - FightForTheRestTiles round with Shot question
		{
			OwnerGameState->GenerateAutoPlayerChoice(LPlayerState);
		}
		PlayersCurrentChoices.Add(LPlayerState->QuestionChoices.Last());
	}
}

TMap<int32, EQuestionResult> USetTerritoryRound::VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	TMap<int32, EQuestionResult> LQuestionResults;
	for (const auto LPlayerChoice : PlayersCurrentChoices)
	{
		const auto LPlayerID =  LPlayerChoice.GetPtr<FPlayerChoice>()->PlayerID;
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[LPlayerID]);
		if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
		{
			// Wrong answer was given
			OwnerGameState->ConstructQuestionResult(LCurrentPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
			LQuestionResults.Add(LPlayerID, EQuestionResult::WrongAnswer);
		}
		else
		{
			// Correct answer was given
			int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
			OwnerGameState->ConstructQuestionResult(LCurrentPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, LPoints, true);
			LQuestionResults.Add(LPlayerID, EQuestionResult::TileCaptured);
			PlayerQuestionPoints.Add(LPlayerID, LPoints);
		}
	}
	return LQuestionResults;
}

void USetTerritoryRound::ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults)
{
	/*TMap<int32, int32> LPlayerPointsUpdate;
	for (const auto LQuestionResult : QuestionResults)
	{
		switch (LQuestionResult.Value)
		{
			case EQuestionResult::TileCaptured:
			{
				int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
				LPlayerPointsUpdate.Add(LQuestionResult.Key, LPoints);	
			}
			break;
			default: break;
		}
	}*/
	//OwnerGameState->ChangePlayerPoints(PlayerQuestionPoints);
}

void USetTerritoryRound::WrapUpCurrentPlayersCycle()
{
	Super::WrapUpCurrentPlayersCycle();
	CurrentPlayerCounter = -1;
	CurrentPlayerTurnsCycle++;
	OwnerGameState->RequestToOpenQuestion(EQuestionType::Choose, 2.0f);
}

bool USetTerritoryRound::HasMoreTurns() const
{
	return PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle);
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
			if (!HasMoreTurns())
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
	PlayerTurnsCycles.Empty();
	for (int32 i = 0; i < OwnerGameState->PlayerArray. Num(); i++)
	{
		LElements.Add(i);
	}
	TArray<FPermutation> LPermutations =
		UBM_Types::GenerateNumberOfPermutations(LElements, OwnerGameState->GetMaxCyclesForRound(EGameRound::SetTerritory));
	for (int32 i = 0; i < LPermutations.Num(); i++)
	{
		PlayerTurnsCycles.Add(FPlayersCycle(i, LPermutations[i], false));
	}
	CurrentPlayerTurnsCycle = 0;
	CurrentPlayerCounter = -1;
	Super::ConstructPlayerTurnsCycles();
}