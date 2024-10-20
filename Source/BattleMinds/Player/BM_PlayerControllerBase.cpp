// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"

#include "BM_UWQuestion.h"
#include "BattleMinds/Tiles/BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"
#include "Core/Classic/BM_GameModeClassic.h"
#include "UI/BM_UWResults.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerController);

ABM_PlayerControllerBase::ABM_PlayerControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABM_PlayerControllerBase::CC_ShowWarningPopup_Implementation(const FText& InText)
{
	if (IsValid(PlayerHUD))
	{
		PlayerHUD->ShowWarningPopup(InText);
	}
}

void ABM_PlayerControllerBase::SC_RequestToUpdateHUD_Implementation()
{
	if(ABM_GameModeBase* LGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		LGameMode->UpdatePlayersHUD();
	}
}

void ABM_PlayerControllerBase::CC_UpdatePlayerHUD_Implementation()
{
	ABM_GameStateBase* GameState = GetWorld() != NULL ? GetWorld()->GetGameState<ABM_GameStateBase>() : NULL;
	if(GameState)
	{
		UE_LOG(LogBM_PlayerController, Display, TEXT("Client %s copy of GameState: %s"), *this->GetName(),*GameState->GetName());
	}
	else
	{
		UE_LOG(LogBM_PlayerController, Warning, TEXT("No GameState copy is available at this client %s"), *this->GetName());
	}
	if(PlayerHUD)
	{
		PlayerHUD->UpdatePlayersInfo(GameState->PlayerArray);
	}
}

void ABM_PlayerControllerBase::CC_InitPlayerHUD_Implementation(const TArray<APlayerState*>& PlayerArray)
{
	ABM_GameStateBase* GameState = GetWorld() != NULL ? GetWorld()->GetGameState<ABM_GameStateBase>() : NULL;
	if(GameState)
	{
		UE_LOG(LogBM_PlayerController, Display, TEXT("Client %s copy of GameState: %s"), *this->GetName(),*GameState->GetName());
	}
	else
	{
		UE_LOG(LogBM_PlayerController, Display, TEXT("No GameState copy is available at this client %s"), *this->GetName());
	}
	PlayerHUD = CreateWidget<UBM_UWPlayerHUD>(this, PlayerHUDClass);
	if(PlayerHUD)
	{
		PlayerHUD->AddToViewport();
		PlayerHUD->InitPlayersInfo(PlayerArray);
	}
}

void ABM_PlayerControllerBase::CC_MarkAnsweredPlayers_Implementation(int32 LastSentPlayer)
{
	if (QuestionWidget)
	{
		QuestionWidget->MarkAnsweredPlayers(LastSentPlayer);
	}
}

void ABM_PlayerControllerBase::CC_ShowResultsWidget_Implementation(const TArray<APlayerState*>& PlayerArray)
{
	if (ResultsWidget)
	{
		ResultsWidget->AddToViewport(0);
	}
	else
	{
		ResultsWidget = CreateWidget<UBM_UWResults>(this, ResultsWidgetClass);
		ResultsWidget->PlayerArray.Append(PlayerArray);
		ResultsWidget->AddToViewport(0);
	}
}

void ABM_PlayerControllerBase::CC_ShowCorrectAnswers_Implementation(const TArray<FPlayerChoice>& PlayersChoices)
{
	if (QuestionWidget)
	{
		QuestionWidget->ShowCorrectAnswers(PlayersChoices);
	}
}

void ABM_PlayerControllerBase::CC_RemoveQuestionWidget_Implementation()
{
	if (QuestionWidget)
	{
		QuestionWidget->RemoveFromParent();
	}
	if (PlayerHUD)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);
		SetViewTargetWithBlend(GetPawn(), 0.5);
	}
}

void ABM_PlayerControllerBase::CC_OpenQuestionWidget_Implementation(FQuestion LastQuestion, const TArray<int32>& AnsweringPlayers)
{
	if(PlayerHUD)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (LastQuestion.Type == EQuestionType::Choose)
	{
		QuestionWidget = CreateWidget<UBM_UWQuestion>(this, ChooseQuestionWidgetClass);
	}
	if (LastQuestion.Type == EQuestionType::Shot)
	{
		QuestionWidget = CreateWidget<UBM_UWQuestion>(this, ShotQuestionWidgetClass);
	}
	QuestionWidget->LastQuestion = LastQuestion;
	QuestionWidget->AnsweringPlayers.Empty();
	QuestionWidget->AnsweringPlayers.Append(AnsweringPlayers);
	if (QuestionWidget)
	{
		QuestionWidget->AddToViewport(0);
	}
}

void ABM_PlayerControllerBase::SC_RequestToOpenQuestion_Implementation()
{
	if (ABM_GameModeBase* GameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		//GameMode->OpenQuestion();
		GetWorldTimerManager().ClearTimer(GameMode->PlayerTurnHandle);
	}
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(ABM_TileBase* TargetTile)
{
	ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	ABM_GameModeBase* GameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
	CurrentClickedTile = TargetTile;
	if (GameMode->CurrentPlayerID == BM_PlayerState->BMPlayerID)
	{
		BM_PlayerState->SetPlayerTurn(true);
		if (BM_PlayerState->IsPlayerTurn())
		{
			if (GameMode->CurrentPlayerAvailableTiles.Contains(CurrentClickedTile))
			{
				if (GameMode->Round != EGameRound::FightForTerritory)
				{
					TargetTile->TileWasClicked(EKeys::LeftMouseButton, GameMode->Round, BM_PlayerState);
					BM_PlayerState->SetPlayerTurn(false);
					GameMode->CurrentPlayerID++;
					GameMode->StartPlayerTurnTimer(GameMode->CurrentPlayerID);
				}
				else
				{
					TargetTile->TileWasClicked(EKeys::LeftMouseButton, GameMode->Round, BM_PlayerState);
					BM_PlayerState->SetPlayerTurn(false);
					GameMode->DefendingPlayerID = CurrentClickedTile->GetOwningPlayerID();
					GameMode->OpenQuestion(EQuestionType::Choose);
					Cast<ABM_PlayerControllerBase>(GameMode->GetGameState<ABM_GameStateBase>()->PlayerArray[GameMode->DefendingPlayerID]->GetPlayerController())->CurrentClickedTile = CurrentClickedTile;
				}
			}
			else
			{
				//TODO: add option to attack any tile once per game
				//TODO: add Widget notification about inability to attack selected tile
				UE_LOG(LogBM_PlayerController, Warning, TEXT("This tile is not available"));
			}
		}
	}
	else
	{
		UE_LOG(LogBM_PlayerController, Warning, TEXT("It's not your turn"));
		CC_ShowWarningPopup(FText::FromString("It's not your turn"));
	}
	
}

bool ABM_PlayerControllerBase::SC_TryClickTheTile_Validate(ABM_TileBase* TargetTile)
{
	return true;
}
