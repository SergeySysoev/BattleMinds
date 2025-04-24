// Battle Minds, 2022. All rights reserved.


#include "FightForTheRemainsRound.h"

#include "Characters/BMCharacterSpawnSlot.h"
#include "Characters/BM_CharacterBase.h"
#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"

void UFightForTheRemainsRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	Super::Enter(InGameState, InTileManager);
	WrapUpCurrentPlayersCycle();
}

void UFightForTheRemainsRound::HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState)
{
	Super::HandleClickedTile(InClickedTile, CurrentPlayerState);
	TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerState->GetPlayerIndex(), ETileStatus::Controlled, CurrentPlayerState->GetPlayerColor(), EGameRound::FightForTheRemains);
	TMap<int32,int32> LUpdatePoints;
	LUpdatePoints.Add(CurrentPlayerState->GetPlayerIndex(), TileManager->GetPointsOfTile(InClickedTile));
	OwnerGameState->ChangePlayerPoints(LUpdatePoints);
}

void UFightForTheRemainsRound::PassTurnToTheNextPlayer()
{
	ABM_PlayerState* LWinnerPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[CurrentPlayerIndex]);
	for (const auto LPlayerState : OwnerGameState->PlayerArray)
	{
		ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
		ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
		if (IsValid(LPlayerController))
		{
			LPlayerController->CC_SetInputEnabled(false);
			LBMPlayerState->SetPlayerTurn(false);
			LPlayerController->UpdateCurrentPlayerTurnSlot(CurrentPlayerCounter,
				LWinnerPlayerState->GetUniqueId(), LWinnerPlayerState->GetPlayerColor());
		}
	}
	if (IsValidPlayerIndex(CurrentPlayerIndex))
	{
		ABM_PlayerControllerBase* LWinnerPlayerController = Cast<ABM_PlayerControllerBase>(LWinnerPlayerState->GetPlayerController());
		if (IsValid(LWinnerPlayerState) && IsValid(LWinnerPlayerController))
		{
			LWinnerPlayerState->SetPlayerTurn(true);
			LWinnerPlayerController->CC_SetInputEnabled(true);
			OwnerGameState->UpdatePlayerTurn();
		}
		//OwnerGameState->StartPlayerTurnTimer();
		OwnerGameState->StartPrePlayerTurnPhase();
	}
}

void UFightForTheRemainsRound::AssignAnsweringPlayers(TArray<int32>& AnsweringPlayers)
{
	Super::AssignAnsweringPlayers(AnsweringPlayers);
	for (int i = 0; i < AnsweringPlayers.Num(); i++)
	{
		ABMCharacterSpawnSlot* LSpawnSlot = CurrentRoundSpawnSlots.FindRef(i);
		if (IsValid(LSpawnSlot))
		{
			ABM_CharacterBase* LSpawnedCharacter = LSpawnSlot->SpawnCharacter();
			if (IsValid(LSpawnedCharacter))
			{
				LSpawnedCharacter->SC_SetColor(OwnerGameState->GetPlayerColorByIndex(AnsweringPlayers[i]));
				LSpawnedCharacter->SC_SetAttacker(true);
				CurrentSpawnedCharacters.Add(AnsweringPlayers[i], LSpawnedCharacter);
			}
		}
	}
}

void UFightForTheRemainsRound::GatherPlayerAnswers()
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

TMap<int32, EQuestionResult> UFightForTheRemainsRound::VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	TArray<FPlayerChoiceShot> ShotChoices;
	TMap<int32, EQuestionResult> LQuestionResults;
	TMap<FPlayerChoiceShot, int32> LSortedToOriginalIndices;
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
	
	if (ShotChoices[0].Answer == MAX_int32)   // No players have sent their answers
	{
		for (const auto LShotChoice: ShotChoices)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(OwnerGameState->PlayerArray[LShotChoice.PlayerID]->GetPlayerController());
			if (IsValid(LoserPlayerController))
			{
				//TileManager->SC_CancelAttackOnClickedTile(LShotChoice.PlayerID);
				OwnerGameState->ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
				LQuestionResults.Add(LShotChoice.PlayerID, EQuestionResult::NobodyAnswered);
			}
		}
		return LQuestionResults;
	}
	//OwnerGameState->SetCurrentPlayerIndex(ShotChoices[0].PlayerID);
	FightForTheRestTileWinnerIndex = ShotChoices[0].PlayerID;
	ABM_PlayerState* WinnerPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[ShotChoices[0].PlayerID]);
	for (int i = 1; i < ShotChoices.Num(); i++)
	{
		ABM_PlayerState* LLoserPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[ShotChoices[i].PlayerID]);
		OwnerGameState->ConstructQuestionResult(LLoserPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, 0, false);
		LQuestionResults.Add(ShotChoices[i].PlayerID, EQuestionResult::WrongAnswer);
	}
	if(IsValid(WinnerPlayerState))
	{
		int32 LOriginalIndex = LSortedToOriginalIndices.FindRef(ShotChoices[0]);
		if (PlayersCurrentChoices.IsValidIndex(LOriginalIndex))
		{
			PlayersCurrentChoices[LOriginalIndex].GetMutable<FPlayerChoiceShot>().bAnswered = true;
		}
		LQuestionResults.Add(ShotChoices[0].PlayerID, EQuestionResult::TileCaptured);
		OwnerGameState->ConstructQuestionResult(WinnerPlayerState, QuestionNumber, LastQuestion, PlayersCurrentChoices, OwnerGameState->GetPointsOfTile(EGameRound::FightForTheRemains), true);
	}
	return LQuestionResults;
}

void UFightForTheRemainsRound::OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults)
{
	CurrentPlayerIndex = FightForTheRestTileWinnerIndex; // Winner of the Shot question
	FightForTheRestTileWinnerIndex = -1;
	PassTurnToTheNextPlayer();
}

void UFightForTheRemainsRound::WrapUpCurrentPlayersCycle()
{
	Super::WrapUpCurrentPlayersCycle();
	CurrentPlayerCounter++;
	OwnerGameState->UpdatePlayerTurn();
	OwnerGameState->RequestToOpenQuestion(EQuestionType::Shot, 2.0f);
}

UE_DISABLE_OPTIMIZATION
void UFightForTheRemainsRound::PrepareNextTurn()
{
	Super::PrepareNextTurn();
	int32 NotOwnedTiles = OwnerGameState->GetNotOwnedTilesCount();
	UE_LOG(LogBM_GameStateBase, Display, TEXT("NotOwnedTiles %d CurrentPlayerCounter %d CurrentCycle %d"), NotOwnedTiles, CurrentPlayerCounter, CurrentPlayerTurnsCycle);
	if (NotOwnedTiles > 0 && HasMoreTurns())
	{
		// instantly Wrap Up Player Cycle as there is no choice in FightForTheRestTiles round
		UE_LOG(LogBM_GameStateBase, Display, TEXT("Continue Fight for Last Tiles round"));
		WrapUpCurrentPlayersCycle();
	}
	else
	{
		UE_LOG(LogBM_GameStateBase, Display, TEXT("Switch to Fight For Territory round"));
		Exit(EGameRound::FightForTerritory);
	}
}

void UFightForTheRemainsRound::ConstructPlayerTurnsCycles()
{
	PlayerTurnsCycles.Empty();
	FPermutation LPermutation;
	for (int32 i = 0; i < OwnerGameState->GetMaxCyclesForRound(EGameRound::FightForTheRemains); i++)
	{
		LPermutation.Values.Add(-1); // All turns will be blacked out until the question is answered
	}
	PlayerTurnsCycles.Add(FPlayersCycle(0, LPermutation, false));
	CurrentPlayerTurnsCycle = 0;
	CurrentPlayerCounter = -1;
	Super::ConstructPlayerTurnsCycles();
}

bool UFightForTheRemainsRound::HasMoreTurns() const
{
	return PlayerTurnsCycles[0].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter+1);
}
UE_ENABLE_OPTIMIZATION
bool UFightForTheRemainsRound::ShouldSkipToPostQuestionComplete() const
{
	return FightForTheRestTileWinnerIndex < 0;
}
