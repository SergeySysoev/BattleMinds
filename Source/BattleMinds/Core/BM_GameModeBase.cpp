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
		PlayerState->BMPlayerID = NumberOfActivePlayers-1;
		PlayerState->MaterialTile = MaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->Nickname = NicknameMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialCastle = CastleMaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialAttack = MaterialAttackMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialNeighbour = MaterialNeighborMap.FindRef(NumberOfActivePlayers);
	}
}
void ABM_GameModeBase::OpenQuestion(EQuestionType QuestionType)
{
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	QuestionsCount++;
	int32 TableIndex = -1;
	TArray<FName> RowNames;
	int32 QuestionIndex = -1;
	FString ContextString;
	FTableRowBase OutRow;
	switch (QuestionType)
	{
		case EQuestionType::Choose:
			TableIndex = FMath::RandRange(0, QuestionTablesChoose.Num()-1);
			ContextString = FString("Question Choose");
			RowNames.Append(QuestionTablesChoose[TableIndex]->GetRowNames());
			QuestionIndex = FMath::RandRange(0, RowNames.Num()-1);
			LastQuestion = std::ref(*QuestionTablesChoose[TableIndex]->FindRow<FQuestion>(RowNames[QuestionIndex],ContextString));
			if (!UsedQuestions.Contains(RowNames[QuestionIndex]))
				UsedQuestions.Add(RowNames[QuestionIndex], LastQuestion);
			break;
		case EQuestionType::Shot:
			TableIndex = FMath::RandRange(0, QuestionTablesShot.Num()-1);
			ContextString = FString("Question Shot");
			RowNames.Append(QuestionTablesShot[TableIndex]->GetRowNames());
			QuestionIndex = FMath::RandRange(0, RowNames.Num()-1);
			LastQuestion = std::ref(*QuestionTablesShot[TableIndex]->FindRow<FQuestion>(RowNames[QuestionIndex],ContextString));
			if (!UsedQuestions.Contains(RowNames[QuestionIndex]))
				UsedQuestions.Add(RowNames[QuestionIndex], LastQuestion);
			break;
		default:
			break;
	}
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_OpenQuestionWidget(RowNames[QuestionIndex]);
			//PlayerController->OpenQuestionWidget(QuestionType ,RowNames[QuestionIndex]);
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
		// Answers may not be pushed by the Player manually ,TODO: add default answer instead
		if(Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Num() < QuestionsCount)
		{
			switch (LastQuestion.Type)
			{
				case EQuestionType::Choose:
				{
					Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Add(LastQuestion);
					break;
				}
				case EQuestionType::Shot:
				{
					Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Add(LastQuestion);
					break;
				}
				default: break;
			}
		}
		CurrentAnsweredQuestions.Add(Cast<ABM_PlayerState>(PlayerState)->AnsweredQuestions.Last());
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameModeBase::ShowCorrectAnswers, 1.0f, false);
}

void ABM_GameModeBase::ShowCorrectAnswers()
{
	TArray<int32> PlayersChoices;
	switch (LastQuestion.Type)
	{
		case EQuestionType::Choose:
		{
			for (const auto AnsweredQuestion: CurrentAnsweredQuestions)
			{
				for (int i=0; i < AnsweredQuestion.Answers.Num(); i++)
				{
					if (AnsweredQuestion.Answers[i].bWasChosen)
					{
						PlayersChoices.Add(i);
					}
				}
			}
			break;
		}
		case EQuestionType::Shot:
		{
			for (const auto AnsweredQuestion: CurrentAnsweredQuestions)
			{
				PlayersChoices.Add(AnsweredQuestion.AnswerShot.PlayerAnswer);
			}
			break;
		}
	}
	
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_ShowCorrectAnswers(PlayersChoices);
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameModeBase::VerifyAnswers, 2.0f, false);
}

void ABM_GameModeBase::VerifyAnswers()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_RemoveQuestionWidget();
		}
	}
	//Verify Players answers:
	switch (LastQuestion.Type)
	{
		case EQuestionType::Choose:
		{
			VertifyChooseAnswers();
			break;
		}
		case EQuestionType::Shot:
		{
			VerifyShotAnswers();
			break;
		}
		default:
			break;
	}
	
	//If GameRound == SetTerritory,
	// check if there are available tiles and their amount == NumberOfActivePlayers%
	// if yes, Continue SetTerritory round
	// if no, Start Battle Mode for the rest of the tiles
	// If GameRound == FightForTerritory, check how many
	switch (Round)
	{
		case EGameRound::SetTerritory:
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
				if(NotOwnedTiles >= NumberOfActivePlayers)
				{
					Round = EGameRound::SetTerritory;
					GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
					CurrentPlayerID = 0;
					StartPlayerTurnTimer(CurrentPlayerID);
				}
				else
				{
					Round = EGameRound::FightForTheRestTiles;
					GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
					CurrentPlayerID = NumberOfActivePlayers;
					StartPlayerTurnTimer(CurrentPlayerID);
					UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Last Tiles round"));
				}
			}
			else
			{
				CurrentPlayerID = 0;
				Round = EGameRound::FightForTerritory;
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Territory round"));
				StartPlayerTurnTimer(CurrentPlayerID);
			}
			break;
		}
		case EGameRound::FightForTheRestTiles:
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
				GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
				CurrentPlayerID = NumberOfActivePlayers;
				StartPlayerTurnTimer(CurrentPlayerID);
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Last Tiles round"));
			}
			else
			{
				Round = EGameRound::FightForTerritory;
				GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
				CurrentPlayerID = 0;
				StartPlayerTurnTimer(CurrentPlayerID);
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight For Territory round"));
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			/*
			 * check how many players are in game
			 * if >=2,
			 *	check how many turns left and decrease that counter for Attacking Player, continue FightForTerritory round
			 * else
			 * switch to the CountResults round
			 */
			break;
		}
	}
}

void ABM_GameModeBase::VertifyChooseAnswers()
{
	switch (Round)
	{
		case EGameRound::SetTerritory:
		{
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
				}
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			if (CurrentAnsweredQuestions[0].bWasAnswered && CurrentAnsweredQuestions[1].bWasAnswered)
			{
				QuestionDelegate.Unbind();
				QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Shot);
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 1.0, false, 2.0);
			}
			break;
		}
	}
}

void ABM_GameModeBase::VerifyShotAnswers()
{
	bool HasExactAnswer = false;
	TMap<int32, bool> ExactAnswersMap;
	// Find Exact answer
	for (int32 i = 0; i < CurrentAnsweredQuestions.Num(); i++)
	{
		if(CurrentAnsweredQuestions[i].bExactAnswer)
		{
			HasExactAnswer = true;
			ExactAnswersMap.Add(i, true);
		}
		else
		{
			ExactAnswersMap.Add(i, false);
		}
	}
	// If there are exact answers, remove the ones that are not exact
	if(HasExactAnswer)
	{
		for(const auto Answer : ExactAnswersMap)
		{
			if(Answer.Value == false)
			{
				if (CurrentAnsweredQuestions.IsValidIndex(Answer.Key))
					CurrentAnsweredQuestions.RemoveAtSwap(Answer.Key);
				if (Round == EGameRound::FightForTerritory)
				{
					const ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[Answer.Key]->GetPlayerController());
					if(CurrentPlayerController)
					{
						CurrentPlayerController->CurrentClickedTile->CancelAttack();
						break;
					}
				}
			}
		}
	}
	else
	// If there are no exact answers at all, find the one who is the closest to the correct answer
	{
		TArray<int32> AnswersToDelete;
		int32 MinDifference = CurrentAnsweredQuestions[0].AnswerShot.Difference;
		AnswersToDelete.Add(0);
		bool bIsCloserThenFirst = false;
		for (int32 i = 1; i < CurrentAnsweredQuestions.Num(); i++)
		{
			if (MinDifference >= CurrentAnsweredQuestions[i].AnswerShot.Difference)
			{
				bIsCloserThenFirst = true;
				MinDifference = CurrentAnsweredQuestions[i].AnswerShot.Difference;
			}
			else
			{
				AnswersToDelete.Add(i);
			}
		}
		if(!bIsCloserThenFirst)
		{
			AnswersToDelete.RemoveAt(0);
		}
		// Remove all answers that are bigger than the minimum delta
		for (int32 i = 0; i < AnswersToDelete.Num(); i++)
		{
			if(CurrentAnsweredQuestions.IsValidIndex(AnswersToDelete[i]))
				CurrentAnsweredQuestions.RemoveAt(AnswersToDelete[i]);
		}
	}
	// Sort left Answers by ElapsedTime to find the quickest;
	CurrentAnsweredQuestions.Sort();
	const ABM_PlayerControllerBase* WinnerPlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentAnsweredQuestions[0].PlayerID]->GetPlayerController());
	if(WinnerPlayerController)
	{
		WinnerPlayerController->CurrentClickedTile->AddTileToPlayerTerritory(WinnerPlayerController->GetPlayerState<ABM_PlayerState>());
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
	CurrentPlayerAvailableTiles.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Current PlayerID %d"), CurrentPlayerID);
	CurrentTurnTimer = TurnTimer;
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
			if(Cast<ABM_PlayerState>(PlayerState)->BMPlayerID == PlayerID)
				Cast<ABM_PlayerState>(PlayerState)->SetPlayerTurn(true);
			else
			{
				Cast<ABM_PlayerState>(PlayerState)->SetPlayerTurn(false);
			}
		}
	}
	for (const auto Tile : Tiles)
	{
		ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
		if(BMTile->GetStatus()==ETileStatus::NotOwned)
		{
			BMTile->MC_RemoveHighlighting();
		}
	}
	if (CurrentPlayerID < NumberOfActivePlayers)
	{
		switch (Round)
		{
			case EGameRound::ChooseCastle:
			{
				for (const auto Tile : Tiles)
				{
					ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
					if (BMTile->GetStatus() == ETileStatus::NotOwned)
					{
						bool NeighborsAvailable = true;;
						for (const auto Neighbor: BMTile->NeighbourTiles)
						{
							if(Neighbor->GetStatus() != ETileStatus::NotOwned)
							{
								NeighborsAvailable = false;
								break;
							}
						}
						if(NeighborsAvailable)
						{
							BMTile->TurnOnHighlight(Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID])->MaterialNeighbour);
							CurrentPlayerAvailableTiles.Add(BMTile);
						}
					}
				}
				break;
			}
			case EGameRound::SetTerritory:
			{
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					if (NeighborTile->GetStatus() == ETileStatus::NotOwned && !NeighborTile->bIsAttacked)
					{
						UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
						NeighborTile->TurnOnHighlight(CurrentPlayerState->MaterialNeighbour);
						CurrentPlayerAvailableTiles.Add(NeighborTile);
					}
				}
				break;
			}
			case EGameRound::FightForTerritory:
			{
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
					NeighborTile->TurnOnHighlight(CurrentPlayerState->MaterialNeighbour);
					CurrentPlayerAvailableTiles.Add(NeighborTile);
				}
				break;
			}
		}
		GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameModeBase::UpdatePlayerTurnTimers, 1.0, true, 1.0f);
	}
		
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
				QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Choose);
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 1.0, false, 2.0);
				break;
			case EGameRound::FightForTheRestTiles:
				//Start Shot question for an available Tile
				//Find first not owned tile
				for (auto Tile: Tiles)
				{
					auto FoundTile = Cast<ABM_TileBase>(Tile);
					if (FoundTile->GetStatus() == ETileStatus::NotOwned)
					{
						for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
						{
							if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
							{
								PlayerController->CurrentClickedTile = FoundTile;
							}
						}
						break;
					}
				}
				QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Shot);
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 1.0, false, 2.0);
				break;
			case EGameRound::FightForTerritory:
				break;
			default: break;
		}
	}
}

void ABM_GameModeBase::ChooseFirstAvailableTileForPlayer(int32 PlayerID)
{
	const int32 RandomTileIndex = FMath::RandRange(0, CurrentPlayerAvailableTiles.Num()-1);
	if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[PlayerID]->GetPlayerController()))
	{
		PlayerController->SC_TryClickTheTile(CurrentPlayerAvailableTiles[RandomTileIndex]);
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
