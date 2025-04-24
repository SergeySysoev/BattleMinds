// Battle Minds, 2022. All rights reserved.

#include "GameRound.h"

#include "Characters/BMCharacterSpawnSlot.h"
#include "Characters/BM_CharacterBase.h"
#include "Core/BM_GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"

void UGameRound::Enter(ABM_GameStateBase* InGameState, ABM_TileManager* InTileManager)
{
	OwnerGameState = InGameState;
	TileManager = InTileManager;
	ConstructPlayerTurnsCycles();
	SetCharacterSpawnSlots();
	PassTurnToNextPlayerPtr = &ThisClass::PassTurnToTheNextPlayer;
}

void UGameRound::ConstructPlayerTurnsCycles()
{
	OwnerGameState->UpdatePlayersCyclesWidget();
}

void UGameRound::SetCharacterSpawnSlots()
{
	TArray<AActor*> LSpawnSlots;
	UGameplayStatics::GetAllActorsOfClass(this, ABMCharacterSpawnSlot::StaticClass(), LSpawnSlots);
	int32 LSlotIndex = 0;
	for (const auto LSpawnSlot : LSpawnSlots)
	{
		if (IsValid(LSpawnSlot))
		{
			ABMCharacterSpawnSlot* LCastedSpawnSlot = Cast<ABMCharacterSpawnSlot>(LSpawnSlot);
			if (OwnerGameState->GetCurrentRound() == LCastedSpawnSlot->GetGameRound())
			{
				CurrentRoundSpawnSlots.Add(LSlotIndex, LCastedSpawnSlot);
				LSlotIndex++;
			}
		}
	}
}

bool UGameRound::HasMoreTurns() const
{
	return true;
}

bool UGameRound::IsShotQuestionNeeded() const
{
	return false;
}

int32 UGameRound::GetCurrentPlayerIndex() const
{
	return CurrentPlayerIndex;
}

int32 UGameRound::GetCurrentCycle() const
{
	return CurrentPlayerTurnsCycle;
}

int32 UGameRound::GetCurrentPlayerCounter() const
{
	return CurrentPlayerCounter;
}

TArray<FPlayersCycle> UGameRound::GetPlayersCycles() const
{
	return PlayerTurnsCycles;
}

void UGameRound::OnStartPostQuestion(TMap<int32, EQuestionResult> QuestionResults)
{
	ChangePlayersPoints(QuestionResults);
	DestroySpawnedCharacters();
}

bool UGameRound::ShouldSkipToPostQuestionComplete() const
{
	return false;
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
	CurrentPlayerIndex = GetNextPlayerIndex();
	if (IsValidPlayerIndex(CurrentPlayerIndex))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(OwnerGameState->PlayerArray[CurrentPlayerIndex]);
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
		CurrentPlayerIndex = -1;
		WrapUpCurrentPlayersCycle();
	}
}

void UGameRound::AssignAnsweringPlayers(TArray<int32>& AnsweringPlayers)
{
	CurrentSpawnedCharacters.Empty();
	for (const auto PlayerState : OwnerGameState->PlayerArray)
	{
		AnsweringPlayers.Add(Cast<ABM_PlayerState>(PlayerState)->GetPlayerIndex());
	}
}

void UGameRound::GatherPlayerAnswers()
{
	PlayersCurrentChoices.Empty();
	PlayerQuestionPoints.Empty();
}

TMap<int32, EQuestionResult> UGameRound::VerifyChooseAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	TMap<int32, EQuestionResult> LQuestionResults;
	return LQuestionResults;
}

TMap<int32, EQuestionResult> UGameRound::VerifyShotAnswers(FInstancedStruct& LastQuestion, int32 QuestionNumber)
{
	TMap<int32, EQuestionResult> LQuestionResults;
	return LQuestionResults;
}

void UGameRound::ChangePlayersPoints(TMap<int32, EQuestionResult>& QuestionResults)
{
	OwnerGameState->ChangePlayerPoints(PlayerQuestionPoints);
}

void UGameRound::WrapUpCurrentPlayersCycle()
{
	OwnerGameState->StopPlayerTurnTimer();
}

void UGameRound::PrepareNextTurn() {}

void UGameRound::Exit(EGameRound NextRound)
{
	OwnerGameState->PrepareNextRound(NextRound);
}

int32 UGameRound::GetNextPlayerIndex()
{
	++CurrentPlayerCounter;
	if (!PlayerTurnsCycles.IsEmpty())
	{
		if (PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
		{
			if (PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter))
			{
				return PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values[CurrentPlayerCounter];
			}
			return -1;
		}
		return -1;
	}
	return -1;
}

void UGameRound::DestroySpawnedCharacters()
{
	for (auto LCharacter : CurrentSpawnedCharacters)
	{
		LCharacter.Value->Destroy();
	}
}

void UGameRound::PlayAnimationOnSpawnedCharacters(TMap<int32, EQuestionResult> QuestionResults)
{
	for (const auto LQuestionResult : QuestionResults)
	{
		ABM_CharacterBase* LCharacter = CurrentSpawnedCharacters.FindRef(LQuestionResult.Key);
		if (IsValid(LCharacter))
		{
			LCharacter->MC_PlayAnimation(LQuestionResult.Value);
		}
	}
}
