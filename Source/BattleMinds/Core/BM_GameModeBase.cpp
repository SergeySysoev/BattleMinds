// Battle Minds, 2021. All rights reserved.


#include "BM_GameModeBase.h"

#include "BM_GameInstance.h"
#include "BM_GameStateBase.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

DEFINE_LOG_CATEGORY(LogBM_GameMode);

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
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
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
	//Remove Question Widget and add Player's answer to the CurrentAnswers array
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			//TODO: Add actual gathering logic
			CurrentAnsweredQuestions.Add(Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Last());
			PlayerController->CC_RemoveQuestionWidget();
		}
	}
	//Verify Players answers: If false, cancel attack, if true, add the tile to the Player's territory
	for (int i = 0; i < CurrentAnsweredQuestions.Num(); i++)
	{
		const ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[i]->GetPlayerController());
		ABM_PlayerState* CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[i]);
		CurrentPlayerController->CurrentClickedTile->MC_RemoveSelection();
		if (CurrentAnsweredQuestions[i].bWasAnswered == false)
		{
			//Set Tile color back to default
			CurrentPlayerController->CurrentClickedTile->CancelAttack();
			if(Round == EGameRound::FightForTerritory)
			{
				CurrentPlayerState->AddPoints(-1* CurrentPlayerController->CurrentClickedTile->GetPoints());
			}
		}
		else
		{
			CurrentPlayerController->CurrentClickedTile->AddTileToPlayerTerritory(CurrentPlayerState);
			CurrentPlayerState->AddPoints(CurrentPlayerController->CurrentClickedTile->GetPoints());
		}
	}
	//If GameRound == SetTerritory,
	// check if there are available tiles and their amount == NumberOfActivePlayers%
	// if yes, Continue SetTerritory round
	// if no, Start Battle Mode for the rest of the tiles - TODO later
	// If GameRound == FightForTerritory, check how many
	if (Round == EGameRound::SetTerritory)
	{
		int32 NotOwnedTiles = 0;
		for (auto Tile:Tiles)
		{
			ABM_TileBase* FoundTile = Cast<ABM_TileBase>(Tile);
			if (FoundTile && FoundTile->GetStatus()==ETileStatus::NotOwned)
			{
				NotOwnedTiles++;
			}
		}
		if (NotOwnedTiles > 0)
		{
			Round = EGameRound::SetTerritory;
			GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
			CurrentPlayerID = 0;
			StartPlayerTurnTimer(CurrentPlayerID);
		}
		else
		{
			Round = EGameRound::FightForTerritory;
			UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Territory round"));
		}
	}
}

EGameRound ABM_GameModeBase::NextGameRound()
{
	if(Round == EGameRound::ChooseCastle)
		return EGameRound::SetTerritory;
	if(Round == EGameRound::SetTerritory)
		return EGameRound::FightForTerritory;
	if(Round == EGameRound::FightForTerritory)
		return  EGameRound::End;
	return EGameRound::End;
}

void ABM_GameModeBase::StartPlayerTurnTimer(int32 PlayerID)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	UE_LOG(LogTemp, Warning, TEXT("Current PlayerID %d"), CurrentPlayerID);
	CurrentTurnTimer = TurnTimer;
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
		}
	}
	if (CurrentPlayerID < NumberOfActivePlayers)
		GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameModeBase::UpdatePlayerTurnTimers, 1.0, true, 1.0f);
	else
	{
		switch (Round)
		{
			case EGameRound::ChooseCastle:
				CurrentPlayerID = 0;
				Round = NextGameRound();
				StartPlayerTurnTimer(CurrentPlayerID);
				break;
			case EGameRound::SetTerritory:
				//All players have chosen their tiles, open Choose Question with 2sec delay
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, this, &ABM_GameModeBase::OpenQuestion, 1.0, false, 2.0);
				break;
			case EGameRound::FightForTerritory:
				break;
			default: break;
		}
	}
}

void ABM_GameModeBase::ChooseFirstAvailableTileForPlayer(int32 PlayerID)
{
	/*TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, Tiles);*/
	for (auto Tile: Tiles)
	{
		auto FoundTile = Cast<ABM_TileBase>(Tile);
		//TODO additional check if this tile is 1-tile close to the current one
		// TArray<ABM_Tiles> NearestTiles = FoundTile->GetNearestTiles();
		// for (Tile : NearestTiles)
		//{ the code below}
		if (FoundTile->GetStatus() == ETileStatus::NotOwned && !FoundTile->bIsAttacked)
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
			PlayerController->UpdateTurnTimer(Round);
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
				PlayerController->ResetTurnTimer(Round);
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
