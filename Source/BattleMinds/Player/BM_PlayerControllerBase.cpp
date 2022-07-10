// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"

#include "BM_UWQuestion.h"
#include "BattleMinds/Tiles/BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Core/BM_GameModeBase.h"

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
	}
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(ABM_TileBase* TargetTile)
{
	const ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	const ABM_GameModeBase* GameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
	if(GameMode->Round == EGameRound::ChooseCastle)
	{
		if (TargetTile->GetOwningPlayerNickname() == BM_PlayerState->Nickname || TargetTile->GetStatus() == ETileStatus::NotOwned)
			TargetTile->TileWasClicked(EKeys::LeftMouseButton, BM_PlayerState->Nickname, BM_PlayerState->MaterialCastle);
		else
			UE_LOG(LogBM_PlayerController, Warning, TEXT("You have clicked a tile that is already owned"));
	}
	else
	{
		if (TargetTile->GetOwningPlayerNickname() == BM_PlayerState->Nickname || TargetTile->GetStatus() == ETileStatus::NotOwned)
			TargetTile->TileWasClicked(EKeys::LeftMouseButton, BM_PlayerState->Nickname, BM_PlayerState->MaterialTile);
		else
			UE_LOG(LogBM_PlayerController, Warning, TEXT("You have clicked a tile that is already owned"));
	}
}

bool ABM_PlayerControllerBase::SC_TryClickTheTile_Validate(ABM_TileBase* TargetTile)
{
	return true;
}