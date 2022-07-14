// Battle Minds, 2021. All rights reserved.


#include "BM_GameModeBase.h"

#include "BM_GameInstance.h"
#include "BM_GameStateBase.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

void ABM_GameModeBase::InitPlayer(APlayerController* NewPlayer)
{
	if (ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(NewPlayer->GetPlayerState<ABM_PlayerState>()))
	{
		Cast<ABM_PlayerControllerBase>(NewPlayer)->PlayerID = NumberOfActivePlayers-1;
		PlayerState->MaterialTile = MaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->Nickname = NicknameMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialCastle = CastleMaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialAttack = MaterialAttackMap.FindRef(NumberOfActivePlayers);
	}
}
void ABM_GameModeBase::OpenQuestion()
{
	int32 TableIndex = FMath::RandRange(0, QuestionTables.Num()-1);
	FString ContextString = FString("Questions");
	TArray<FName> RowNames = QuestionTables[TableIndex]->GetRowNames();
	int32 QuestionIndex = FMath::RandRange(0, RowNames.Num()-1);
	FTableRowBase OutRow;
	FQuestion* Question = QuestionTables[TableIndex]->FindRow<FQuestion>(RowNames[QuestionIndex],"Question");
	if (!UsedQuestions.Contains(RowNames[QuestionIndex]))
		UsedQuestions.Add(RowNames[QuestionIndex], std::ref(*Question));
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_OpenQuestionWidget(RowNames[QuestionIndex]);
		}
	}
	StartQuestionTimer();
}
void ABM_GameModeBase::StartQuestionTimer()
{
	GetWorld()->GetTimerManager().SetTimer(QuestionTimerHandle, this, &ABM_GameModeBase::GatherPlayersAnswers, QuestionTimer, false);
}
void ABM_GameModeBase::GatherPlayersAnswers()
{
	CurrentAnsweredQuestions.Empty();
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			//TODO: Add actual gathering logic
			CurrentAnsweredQuestions.Add(Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Last());
			PlayerController->CC_RemoveQuestionWidget();
		}
	}
	for (int i = 0; i < CurrentAnsweredQuestions.Num(); i++)
	{
		const ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[i]->GetPlayerController());
		const ABM_PlayerState* CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[i]);
		if (CurrentAnsweredQuestions[i].bWasAnswered == false)
		{
			//Set Tile color back to default
			CurrentPlayerController->CurrentClickedTile->CancelAttack();
		}
		else
		{
			CurrentPlayerController->CurrentClickedTile->TileWasChosen(CurrentPlayerState->Nickname, CurrentPlayerState->MaterialTile);
			CurrentPlayerController->CurrentClickedTile->bIsAttacked = false;
		}
	}
}

void ABM_GameModeBase::StartPlayerTurnTimer(int32 PlayerID)
{
	CurrentTurnTimer = TurnTimer;
	GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameModeBase::UpdatePlayerTurnTimers, 1.0, true, 1.0f);
}

void ABM_GameModeBase::ChooseFirstAvailableTileForPlayer(int32 PlayerID)
{
	TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, Tiles);
	for (auto Tile: Tiles)
	{
		auto FoundTile = Cast<ABM_TileBase>(Tile);
		//TODO additional check if this tile is 1-tile close to the current one
		// TArray<ABM_Tiles> NearestTiles = FoundTile->GetNearestTiles();
		// for (Tile : NearestTiles)
		//{ the code below}
		if (FoundTile->GetStatus() == ETileStatus::NotOwned)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[PlayerID]->GetPlayerController()))
			{
				PlayerController->SC_TryClickTheTile(FoundTile);
			}
			break;
		}
	}
}

void ABM_GameModeBase::UpdatePlayerTurnTimers()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdateTurnTimer();
		}
	}
	CurrentTurnTimer--;
	if(CurrentTurnTimer == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
		ChooseFirstAvailableTileForPlayer(CurrentPlayerID);
		for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
			{
				PlayerController->ResetTurnTimer();
			}
		}
	}
}

void ABM_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NumberOfActivePlayers++;
	InitPlayer(NewPlayer);
}

void ABM_GameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_GameModeBase, Round);
}
