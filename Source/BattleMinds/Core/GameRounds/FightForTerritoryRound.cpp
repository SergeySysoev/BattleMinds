// Battle Minds, 2022. All rights reserved.


#include "FightForTerritoryRound.h"

#include "Characters/BMCharacterSpawnSlot.h"
#include "Characters/BM_CharacterBase.h"
#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerState.h"

void UFightForTerritoryRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	Super::Enter(InGameState, InTileManager);
	PassTurnToTheNextPlayer();
}

void UFightForTerritoryRound::HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState)
{
	TileManager->SC_AttackTile(InClickedTile, CurrentPlayerState->GetPlayerColor());
	CurrentSiegeTileQuestionCount = TileManager->GetTileQuestionsCount(InClickedTile);
	SetDefendingPlayer(InClickedTile);
	TileManager->BindGameStateToTileBannerMeshSpawned(InClickedTile, OwnerGameState->OpenNextQuestionPtr);
}

void UFightForTerritoryRound::AssignAnsweringPlayers(TArray<int32>& AnsweringPlayers)
{
	AnsweringPlayers.Add(CurrentPlayerIndex);
	AnsweringPlayers.Add(DefendingPlayerIndex);
	for (int i = 0; i < AnsweringPlayers.Num(); i++)
	{
		ABMCharacterSpawnSlot* LSpawnSlot = CurrentRoundSpawnSlots.FindRef(i);
		if (IsValid(LSpawnSlot))
		{
			ABM_CharacterBase* LSpawnedCharacter = LSpawnSlot->SpawnCharacter();
			if (IsValid(LSpawnedCharacter))
			{
				LSpawnedCharacter->SC_SetColor(OwnerGameState->GetPlayerColorByIndex(AnsweringPlayers[i]));
				LSpawnedCharacter->SC_SetAttacker(AnsweringPlayers[i] == CurrentPlayerIndex);
				CurrentSpawnedCharacters.Add(AnsweringPlayers[i], LSpawnedCharacter);
			}
		}
	}
}

void UFightForTerritoryRound::GatherPlayerAnswers()
{
	Super::GatherPlayerAnswers();
	
	ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[DefendingPlayerIndex]);

	if (IsValid(AttackingPlayerState) && IsValid(DefendingPlayerState))
	{
		if(AttackingPlayerState->CurrentQuestionAnswerSent == false)
		{
			OwnerGameState->GenerateAutoPlayerChoice(AttackingPlayerState);
		}
		if (DefendingPlayerState->CurrentQuestionAnswerSent == false)
		{
			OwnerGameState->GenerateAutoPlayerChoice(DefendingPlayerState);
		}
		PlayersCurrentChoices.Add(AttackingPlayerState->QuestionChoices.Last());
		PlayersCurrentChoices.Add(DefendingPlayerState->QuestionChoices.Last());
	}
}

TMap<int32, EQuestionResult> UFightForTerritoryRound::VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	bShotQuestionIsNeeded = false;
	const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	TMap<int32, EQuestionResult> LQuestionResults;
	ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[DefendingPlayerIndex]);
	if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer
		&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
	{
		// if both Players answered their Choose Question, we need a Shot round
		bShotQuestionIsNeeded = true;
		LQuestionResults.Add(PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::ShotQuestionNeeded);
		LQuestionResults.Add(PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::ShotQuestionNeeded);
		return LQuestionResults;
	}
	
	if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer
		&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
	{
		CurrentSiegeTileQuestionCount = 0;
		// both Players gave wrong answer, cancel attack for Attacking, and don't give points to Defending
		OwnerGameState->ConstructQuestionResult(DefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
		OwnerGameState->ConstructQuestionResult(AttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
		LQuestionResults.Add(PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::NobodyAnswered);
		LQuestionResults.Add(PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::NobodyAnswered);
		return LQuestionResults;
	}
	// Someone got the right answer
	CurrentSiegeTileQuestionCount--;
	for (const auto AnsweredQuestion : PlayersCurrentChoices)
	{
		if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
		{
			// correct answer
			int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
			if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->PlayerID == CurrentPlayerIndex)
			{
				// attacking player was right
				//HandleQuestionResults(EAnsweredPlayer::AttackingPlayer);
				if (CurrentSiegeTileQuestionCount > 0 && TileManager->GetStatusOfCurrentClickedTile(CurrentPlayerIndex) == ETileStatus::Castle)
				{
					LPoints = 0;	
				}
				OwnerGameState->ConstructQuestionResult(DefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, -1 * LPoints, false);
				OwnerGameState->ConstructQuestionResult(AttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, LPoints, true);
				// Apply 1 damage to CurrentClickedTile of AttackingPlayer
				TileManager->SC_ApplyDamageToTile(CurrentPlayerIndex, 1);
				if (CurrentSiegeTileQuestionCount == 0)
				{
					LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileCaptured);
					LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::WrongAnswer);
					PlayerQuestionPoints.Add(CurrentPlayerIndex, LPoints);
					PlayerQuestionPoints.Add(CurrentPlayerIndex, -LPoints);
				}
				else
				{
					LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDamaged);
					LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::WrongAnswer);
				}
			}
			else 
			{
				// defending player was right
				CurrentSiegeTileQuestionCount = 0; // stop Siege
				OwnerGameState->ConstructQuestionResult(DefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 100, true);
				OwnerGameState->ConstructQuestionResult(AttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
				LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::WrongAnswer);
				LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDefended);
				PlayerQuestionPoints.Add(DefendingPlayerIndex, 100);
			}
			break;
		}
	}
	return LQuestionResults;
}

void UFightForTerritoryRound::ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults)
{
	/*TMap<int32, int32> LPlayerPointsUpdate;
	for (const auto LQuestionResult : QuestionResults)
	{
		switch (LQuestionResult.Value)
		{
			case EQuestionResult::ShotQuestionNeeded:
				break;
			case EQuestionResult::WrongAnswer:
				{
					if (LQuestionResult.Key == DefendingPlayerIndex)
					{
						int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
						LPlayerPointsUpdate.Add(LQuestionResult.Key, -LPoints);	
					}
				}
				break;
			case EQuestionResult::TileDefended:
				{
					if (LQuestionResult.Key == DefendingPlayerIndex)
					{
						int32 LPoints = 100;
						LPlayerPointsUpdate.Add(LQuestionResult.Key, LPoints);
					}
				}
				break;
			case EQuestionResult::TileDamaged:
				break;
			case EQuestionResult::TileCaptured:
				{
					if (LQuestionResult.Key == CurrentPlayerIndex)
					{
						int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
						LPlayerPointsUpdate.Add(LQuestionResult.Key, LPoints);
					}
				}
				break;
			default: break;
		}
	}*/
	OwnerGameState->ChangePlayerPoints(PlayerQuestionPoints);
}

TMap<int32, EQuestionResult> UFightForTerritoryRound::VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	bShotQuestionIsNeeded = false;
	TArray<FPlayerChoiceShot> ShotChoices;
	TMap<int32, EQuestionResult> LQuestionResults;
	TMap<FPlayerChoiceShot, int32> LSortedToOriginalIndices;
	ABM_PlayerState* LAttackingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerState* LDefendingPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[DefendingPlayerIndex]);
	for (int32 i = 0; i < PlayersCurrentChoices.Num(); i++)
	{
		ShotChoices.Add(PlayersCurrentChoices[i].Get<FPlayerChoiceShot>());
		LSortedToOriginalIndices.Add(PlayersCurrentChoices[i].Get<FPlayerChoiceShot>(), i);
	}
	// Sort Answers by Difference and ElapsedTime (overriden < operator in FPlayerChoiceShot)
	if (ShotChoices.Num() > 1)
	{
		ShotChoices.Sort();
	}
	CurrentSiegeTileQuestionCount--;
	if (ShotChoices[0].Answer == MAX_int32)
	{
		// both players haven't sent their answers, cancel Attack and don't add points to the Defender
		CurrentSiegeTileQuestionCount = 0;
		OwnerGameState->ConstructQuestionResult(LDefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
		OwnerGameState->ConstructQuestionResult(LAttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
		LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::NobodyAnswered);
		LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::NobodyAnswered);
		return LQuestionResults;
	}
	int32 LWinnerIndex = ShotChoices[0].PlayerID;
	int32 LOriginalIndex = LSortedToOriginalIndices.FindRef(ShotChoices[0]);
	if (PlayersCurrentChoices.IsValidIndex(LOriginalIndex))
	{
		PlayersCurrentChoices[LOriginalIndex].GetMutable<FPlayerChoiceShot>().bAnswered = true;
	}
	if (IsValid(LAttackingPlayerState) && IsValid(LDefendingPlayerState))
	{
		if (CurrentPlayerIndex == LWinnerIndex)		// Attacking Player was right
		{
			int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
			if (CurrentSiegeTileQuestionCount > 0 && TileManager->GetStatusOfCurrentClickedTile(CurrentPlayerIndex) == ETileStatus::Castle)
			{
				LPoints = 0;	
			}
			OwnerGameState->ConstructQuestionResult(LDefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, -1 * LPoints, false);
			OwnerGameState->ConstructQuestionResult(LAttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, LPoints, true);
			// Apply 1 damage to CurrentClickedTile of AttackingPlayer
			TileManager->SC_ApplyDamageToTile(CurrentPlayerIndex, 1);
			if (CurrentSiegeTileQuestionCount == 0)
			{
				LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileCaptured);
				LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::WrongAnswer);
				PlayerQuestionPoints.Add(CurrentPlayerIndex, LPoints);
				PlayerQuestionPoints.Add(DefendingPlayerIndex, -LPoints);
			}
			else
			{
				LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDamaged);
				LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::WrongAnswer);
			}
		}
		else // Defending Player was right
		{
			OwnerGameState->ConstructQuestionResult(LDefendingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 100, true);
			OwnerGameState->ConstructQuestionResult(LAttackingPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
			LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::WrongAnswer);
			LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDefended);
			CurrentSiegeTileQuestionCount = 0;
			PlayerQuestionPoints.Add(DefendingPlayerIndex, 100);
			return LQuestionResults;
		}
	}
	return LQuestionResults;
}

void UFightForTerritoryRound::WrapUpCurrentPlayersCycle()
{
	Super::WrapUpCurrentPlayersCycle();
	if (!HasMoreTurns())
	{
		Exit(EGameRound::End);
		return;
	}
	CurrentPlayerCounter = -1;
	CurrentPlayerTurnsCycle++;
	PassTurnToTheNextPlayer();
}

bool UFightForTerritoryRound::HasMoreTurns() const
{
	return PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle);
}

void UFightForTerritoryRound::PrepareNextTurn()
{
	/*
	* check how many questions left to answer to conquer the tile
	* if 0, then
	*		check how many players are in game
	*		if >=2,
	*		check if the Cycle counter is not exceeds MaxCycles, continue FightForTerritory round
	*		else
	*		switch to the CountResults round
	*	else Continue the siege
	*/
	if (bShotQuestionIsNeeded)
	{
		OwnerGameState->RequestToOpenQuestion(EQuestionType::Shot, 0.5f);
		return;
	}
	if (CurrentSiegeTileQuestionCount == 0)
	{
		if (OwnerGameState->GetRemainingPlayersCount() < 2 || !HasMoreTurns())
		{
			Exit(EGameRound::End);
		}
		else
		{
			PassTurnToTheNextPlayer();
		}
	}
	else
	{
		OwnerGameState->OpenNextQuestion();
	}
}

void UFightForTerritoryRound::ConstructPlayerTurnsCycles()
{
	TArray<int32> LElements;
	PlayerTurnsCycles.Empty();
	for (int32 i = 0; i < OwnerGameState->PlayerArray. Num(); i++)
	{
		LElements.Add(i);
	}
	TArray<FPermutation> LPermutations =
		UBM_Types::GenerateNumberOfPermutations(LElements, OwnerGameState->GetMaxCyclesForRound(EGameRound::FightForTerritory));
	for (int32 i = 0; i < LPermutations.Num(); i++)
	{
		PlayerTurnsCycles.Add(FPlayersCycle(i, LPermutations[i], false));
	}
	CurrentPlayerTurnsCycle = 0;
	CurrentPlayerCounter = -1;
	Super::ConstructPlayerTurnsCycles();
}

void UFightForTerritoryRound::SetDefendingPlayer(FIntPoint InClickedTile)
{
	DefendingPlayerIndex = TileManager->GetTileOwnerID(InClickedTile);
	TileManager->HandleClickedTile(DefendingPlayerIndex, InClickedTile);
}

bool UFightForTerritoryRound::IsShotQuestionNeeded() const
{
	return bShotQuestionIsNeeded;
}

void UFightForTerritoryRound::OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults)
{
	TArray<EQuestionResult> LQuestionResultsArray;
	QuestionResults.GenerateValueArray(LQuestionResultsArray);
	if (!LQuestionResultsArray.Contains(EQuestionResult::TileDamaged))
	{
		// Change points only if there was no Castle damage
		Super::OnStartPostQuestion(QuestionResults);
	}
}

bool UFightForTerritoryRound::ShouldSkipToPostQuestionComplete() const
{
	return bShotQuestionIsNeeded;
}