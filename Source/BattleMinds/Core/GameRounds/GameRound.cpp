// Battle Minds, 2022. All rights reserved.

#include "GameRound.h"
#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"

void UGameRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	OwnerGameState = InGameState;
	TileManager = InTileManager;
	ConstructPlayerTurnsCycles();
}

void UGameRound::ConstructPlayerTurnsCycles()
{
	OwnerGameState->UpdatePlayersCyclesWidget();
}

bool UGameRound::IsValidPlayerIndex(int32 IndexToCheck) const
{
	return IsValid(OwnerGameState) &&
		OwnerGameState->PlayerArray.IsValidIndex(IndexToCheck) &&
		OwnerGameState->IsRemainingPlayer(IndexToCheck);
}

void UGameRound::HandleClickedTile(const FIntPoint& InClickedTile, ABM_PlayerState* CurrentPlayerState) {}

void UGameRound::PassTurnToTheNextPlayer()
{
	for (const auto LPlayerState : OwnerGameState->PlayerArray)
	{
		ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
		ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
		if (IsValid(LPlayerController))
		{
			LPlayerController->CC_SetInputEnabled(false);
			LBMPlayerState->SetPlayerTurn(false);
		}
	}
	OwnerGameState->SetNextPlayerIndex();
	if (IsValidPlayerIndex(OwnerGameState->GetCurrentPlayerIndex()))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[OwnerGameState->GetCurrentPlayerIndex()]);
		if (IsValid(LCurrentPlayerState))
		{
			TileManager->SC_ResetFirstAvailableTile();
			LCurrentPlayerState->SetPlayerTurn(true);
			OwnerGameState->UpdatePlayerTurn();
		}
		OwnerGameState->StartPrePlayerTurnPhase();
	}
	else
	{
		OwnerGameState->SetCurrentPlayerCounter(-1);
		WrapUpCurrentPlayersCycle();
	}
}

TMap<int32, EQuestionResult> UGameRound::VerifyChooseAnswers(FInstancedStruct& LastQuestion, TArray<FInstancedStruct>& PlayerCurrentChoices, int32 QuestionNumber)
{
	TMap<int32, EQuestionResult> LQuestionResults;
	return LQuestionResults;
}

TMap<int32, EQuestionResult> UGameRound::VerifyShotAnswers()
{
	TMap<int32, EQuestionResult> LQuestionResults;
	return LQuestionResults;
}

void UGameRound::HandleQuestionResults(EAnsweredPlayer AnsweredPlayer) {}

void UGameRound::WrapUpCurrentPlayersCycle()
{
	OwnerGameState->ClearPlayerTurnTimer();
}

void UGameRound::PrepareNextTurn() {}

void UGameRound::Exit(EGameRound NextRound)
{
	OwnerGameState->PrepareNextRound(NextRound);
}