// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"
#include "BM_UWQuestion.h"
#include "BattleMinds/Tiles/BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"
#include "Tiles/BM_TileBase.h"
#include "UI/BM_UWResults.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerController);

ABM_PlayerControllerBase::ABM_PlayerControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

bool ABM_PlayerControllerBase::HasValidCurrentClickedTile() const
{
	return IsValid(CurrentClickedTile);
}

void ABM_PlayerControllerBase::SC_RemoveCurrentTileFromTerritory_Implementation() const
{
	check(HasAuthority());
	
	if (IsValid(CurrentClickedTile))
	{
		Cast<ABM_PlayerState>(PlayerState)->SC_RemoveTileFromTerritory(CurrentClickedTile);
	}
}

void ABM_PlayerControllerBase::SC_AssignCurrentTile_Implementation(ABM_TileBase* TargetTile)
{
	check(HasAuthority());
	CurrentClickedTile = TargetTile;
}

void ABM_PlayerControllerBase::SC_ApplyDamageToClickedTile_Implementation(int32 DamageAmount) const
{
	if (IsValid(CurrentClickedTile))
	{
		CurrentClickedTile->SC_ApplyDamage(DamageAmount);
	}
}

void ABM_PlayerControllerBase::CC_SetGameLength_Implementation(const EGameLength GameLength)
{
	if (IsValid(PlayerHUD))
	{
		PlayerHUD->SetGameLength(GameLength);
	}
}

int32 ABM_PlayerControllerBase::GetPointsOfCurrentClickedTile() const
{
	if (!IsValid(CurrentClickedTile))
	{
		return 0;	
	}
	return CurrentClickedTile->GetPoints();
}

void ABM_PlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	Cast<ABM_PlayerState>(PlayerState)->OnCastleConquered.AddUniqueDynamic(this, &ThisClass::SC_BeginTransferTerritory);
}

FLinearColor ABM_PlayerControllerBase::GetPlayerColorByID(int32 PlayerID) const
{
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		return LGameState->GetPlayerColorByID(PlayerID);
	}
	return FLinearColor::White;
}

void ABM_PlayerControllerBase::SC_BeginTransferTerritory_Implementation()
{
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		LGameState->TransferDefendingPlayerTerritoryToAttacker();
	}
}

void ABM_PlayerControllerBase::SC_AddCurrentTileToTerritory_Implementation(ETileStatus InTileStatus) const
{
	check(HasAuthority());
	
	if (IsValid(CurrentClickedTile))
	{
		Cast<ABM_PlayerState>(PlayerState)->SC_AddTileToTerritory(CurrentClickedTile, InTileStatus);
	}
}

void ABM_PlayerControllerBase::SC_CancelAttackForCurrentTile_Implementation() const
{
	check(HasAuthority());
	
	if (IsValid(CurrentClickedTile))
	{
		CurrentClickedTile->CancelAttack();
	}
}

void ABM_PlayerControllerBase::SC_AddAnsweredQuestionChoice_Implementation(FInstancedStruct InPlayerChoice)
{
	ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(PlayerState);
	if (!IsValid(BM_PlayerState))
	{
		return;
	}
	BM_PlayerState->CurrentQuestionAnswerSent = true;
	BM_PlayerState->QuestionChoices.Add(InPlayerChoice);
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		LGameState->OnAnswerSent.Broadcast(BM_PlayerState->BMPlayerID);
	}
}

void ABM_PlayerControllerBase::CC_ShowWarningPopup_Implementation(const FText& InText)
{
	if (IsValid(PlayerHUD))
	{
		PlayerHUD->ShowWarningPopup(InText);
	}
}


void ABM_PlayerControllerBase::CC_InitPlayerHUD_Implementation(const TArray<FPlayerInfo>& PlayersHUDInfo)
{
	ABM_GameStateBase* GameState = GetWorld() != nullptr ? GetWorld()->GetGameState<ABM_GameStateBase>() : nullptr;
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
		PlayerHUD->InitPlayersInfo(PlayersHUDInfo);
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

void ABM_PlayerControllerBase::CC_ShowCorrectAnswers_Implementation(const TArray<FInstancedStruct>& PlayersChoices)
{
	if (QuestionWidget)
	{
		QuestionWidget->ShowCorrectAnswers(PlayersChoices);
	}
}

void ABM_PlayerControllerBase::CC_RemoveQuestionWidget_Implementation(bool bSwitchViewTargetBackToTiles)
{
	if (QuestionWidget)
	{
		QuestionWidget->RemoveFromParent();
	}
	if (PlayerHUD && bSwitchViewTargetBackToTiles)
	{
		//PlayerHUD->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->ShowAllWidgets();
		SetViewTargetWithBlend(GetPawn(), 0.5);
	}
}

void ABM_PlayerControllerBase::CC_OpenQuestionWidget_Implementation(FInstancedStruct LastQuestion, const TArray<int32>& AnsweringPlayers, AActor* NewViewTarget)
{
	if (!LastQuestion.GetPtr<FQuestion>())
	{
		return;
	}
	if(PlayerHUD)
	{
		//PlayerHUD->SetVisibility(ESlateVisibility::Collapsed);
		PlayerHUD->HideAllExceptForPlayerInfo();
	}
	if (IsValid(NewViewTarget))
	{
		SetViewTargetWithBlend(NewViewTarget, 0.5);	
	}
	if (LastQuestion.GetPtr<FQuestion>()->Type == EQuestionType::Choose)
	{
		QuestionWidget = CreateWidget<UBM_UWQuestion>(this, ChooseQuestionWidgetClass);
	}
	if (LastQuestion.GetPtr<FQuestion>()->Type == EQuestionType::Shot)
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

void ABM_PlayerControllerBase::SC_RequestToOpenQuestion_Implementation(EQuestionType QuestionType)
{
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		LGameState->RequestToOpenQuestion(QuestionType);
	}
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(ABM_TileBase* TargetTile)
{
	ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	SC_AssignCurrentTile(TargetTile);
	
	if (!IsValid(BM_PlayerState) || !IsValid(LGameState))
	{
		return;
	}
	
	if (LGameState->GetCurrentPlayerID() == BM_PlayerState->BMPlayerID)
	{
		if (BM_PlayerState->IsPlayerTurn())
		{
			if (LGameState->GetCurrentPlayerAvailableTiles().Contains(CurrentClickedTile))
			{
				LGameState->HandleClickedTile(TargetTile);
			}
			else
			{
				//TODO: add option to attack any tile once per game
				//TODO: add Widget notification about inability to attack selected tile
				UE_LOG(LogBM_PlayerController, Warning, TEXT("This tile is not available"));
				CC_ShowWarningPopup(FText::FromString("This Tile is not available for attack"));
			}
		}
		else
		{
			UE_LOG(LogBM_PlayerController, Warning, TEXT("It's not your turn"));
			CC_ShowWarningPopup(FText::FromString("It's not your turn"));
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
