// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"

#include "BM_UWQuestion.h"
#include "BattleMinds/Tiles/BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Core/BM_GameModeBase.h"
#include "Core/Classic/BM_GameModeClassic.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerController);

ABM_PlayerControllerBase::ABM_PlayerControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABM_PlayerControllerBase::CC_RemoveQuestionWidget_Implementation()
{
	if(QuestionWidget)
		QuestionWidget->RemoveFromViewport();
}

void ABM_PlayerControllerBase::CC_OpenQuestionWidget_Implementation(FName QuestionRowName)
{
	QuestionWidget = CreateWidget<UBM_UWQuestion>(this,QuestionWidgetClass);
	QuestionWidget->QuestionName = QuestionRowName;
	if (QuestionWidget)
		QuestionWidget->AddToViewport(0);
}

void ABM_PlayerControllerBase::SC_RequestToOpenQuestion_Implementation()
{
	if (ABM_GameModeBase* GameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OpenQuestion();
		GetWorldTimerManager().ClearTimer(GameMode->PlayerTurnHandle);
	}
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(ABM_TileBase* TargetTile)
{
	ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	ABM_GameModeBase* GameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
	CurrentClickedTile = TargetTile;
	if(GameMode->Round == EGameRound::ChooseCastle)
	{
		if (GameMode->CurrentPlayerID == PlayerID)
		{
			GetPlayerState<ABM_PlayerState>()->SetPlayerTurn(true);
		}
		if (GetPlayerState<ABM_PlayerState>()->IsPlayerTurn() &&(TargetTile->GetOwningPlayerNickname() == BM_PlayerState->Nickname || TargetTile->GetStatus() == ETileStatus::NotOwned))
			//TargetTile->TileWasClicked(EKeys::LeftMouseButton, BM_PlayerState->Nickname, BM_PlayerState->MaterialCastle, GameRound);
			{
				TargetTile->TileWasChosen(BM_PlayerState->Nickname, BM_PlayerState->MaterialCastle);
				if(ABM_GameModeClassic* ClassicGameMode = Cast<ABM_GameModeClassic>(GameMode))
				{
					GetPlayerState<ABM_PlayerState>()->SetPlayerTurn(false);
					ClassicGameMode->CurrentPlayerID++;
					GetWorld()->GetTimerManager().ClearTimer(ClassicGameMode->CastleTurnTimer);
					ClassicGameMode->StartChooseCastleTimer();
				}
			}
		else
			UE_LOG(LogBM_PlayerController, Warning, TEXT("You have clicked a tile that is already owned"));
	}
	else
	{
		if (TargetTile->GetOwningPlayerNickname() == BM_PlayerState->Nickname || TargetTile->GetStatus() == ETileStatus::NotOwned)
			TargetTile->TileWasClicked(EKeys::LeftMouseButton, BM_PlayerState->Nickname, BM_PlayerState->MaterialAttack, GameMode->Round);
		else
			UE_LOG(LogBM_PlayerController, Warning, TEXT("You have clicked a tile that is already owned"));
	}
}

bool ABM_PlayerControllerBase::SC_TryClickTheTile_Validate(ABM_TileBase* TargetTile)
{
	return true;
}