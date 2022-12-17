// Battle Minds, 2021. All rights reserved.


#include "BM_GameModeBase.h"
#include "BM_GameInstance.h"
#include "BM_GameStateBase.h"
#include "ToolBuilderUtil.h"
#include "Camera/CameraActor.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerPawn.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

DEFINE_LOG_CATEGORY(LogBM_GameMode);

void ABM_GameModeBase::InitPlayer(APlayerController* NewPlayer)
{
	if (ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(NewPlayer->GetPlayerState<ABM_PlayerState>()))
	{
		PlayerState->BMPlayerID = NumberOfActivePlayers;
		PlayerState->MaterialTile = MaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->Nickname = NicknameMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialCastle = CastleMaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialAttack = MaterialAttackMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialNeighbour = MaterialNeighborMap.FindRef(NumberOfActivePlayers);
		PlayerState->PlayerColor = ColorMap.FindRef(NumberOfActivePlayers);
		PlayerState->NumberOfTurns = NumberOfPlayerTurns;
		NumberOfTotalTurns += NumberOfPlayerTurns;
		const auto AvailablePlayerStart = FindPlayerStart(NewPlayer, TEXT(""));
		const FVector Location = AvailablePlayerStart->GetActorLocation();
		const FRotator Rotation = AvailablePlayerStart->GetActorRotation();
		ABM_PlayerPawn* SpawnedPawn = GetWorld()->SpawnActor<ABM_PlayerPawn>(PawnClass, Location, Rotation);
		//SpawnedPawn->SetActorRotation(Rotation);
		NewPlayer->Possess(SpawnedPawn);
	}
}

void ABM_GameModeBase::FindNextQuestion(EQuestionType Question, int32 INT32, TArray<FName> Array, int32 QuestionIndex, FString& ContextString)
{
	switch (Question)
	{
		case EQuestionType::Choose:
			INT32 = FMath::RandRange(0, QuestionTablesChoose.Num()-1);
			ContextString = FString("Question Choose");
			Array.Append(QuestionTablesChoose[INT32]->GetRowNames());
			QuestionIndex = FMath::RandRange(0, Array.Num()-1);
			LastQuestion = std::ref(*QuestionTablesChoose[INT32]->FindRow<FQuestion>(Array[QuestionIndex],ContextString));
			if (!UsedQuestions.Contains(Array[QuestionIndex]))
				UsedQuestions.Add(Array[QuestionIndex], LastQuestion);
			break;
		case EQuestionType::Shot:
			INT32 = FMath::RandRange(0, QuestionTablesShot.Num()-1);
			ContextString = FString("Question Shot");
			Array.Append(QuestionTablesShot[INT32]->GetRowNames());
			QuestionIndex = FMath::RandRange(0, Array.Num()-1);
			LastQuestion = std::ref(*QuestionTablesShot[INT32]->FindRow<FQuestion>(Array[QuestionIndex],ContextString));
			if (!UsedQuestions.Contains(Array[QuestionIndex]))
				UsedQuestions.Add(Array[QuestionIndex], LastQuestion);
			break;
		default:
			break;
	}
}

void ABM_GameModeBase::AssignAnsweringPlayers()
{
	if(Round != EGameRound::FightForTerritory)
	{
		for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			AnsweringPlayers.Add(Cast<ABM_PlayerState>(PlayerState)->BMPlayerID);
		}
	}
	else
	{
		AnsweringPlayers.Add(CurrentPlayerID);
		AnsweringPlayers.Add(DefendingPlayerID);
	}
}

void ABM_GameModeBase::SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			switch (QuestionType)
			{
				case EQuestionType::Choose:
					PlayerController->SetViewTargetWithBlend(ChooseQuestionCamera, 0.5);
					UE_LOG(LogBM_GameMode, Display, TEXT("Camera: %s, ViewTarget: %s"), *ChooseQuestionCamera->GetName(),*PlayerController->GetViewTarget()->GetName());
					break;
				case EQuestionType::Shot:
					PlayerController->SetViewTargetWithBlend(ShotQuestionCamera, 0.5);
					UE_LOG(LogBM_GameMode, Display, TEXT("Camera: %s, ViewTarget: %s"), *ShotQuestionCamera->GetName(),*PlayerController->GetViewTarget()->GetName());
					break;
				default: break;
			}
			PlayerController->CC_OpenQuestionWidget(RowNames[QuestionIndex], AnsweringPlayers);
		}
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
	FindNextQuestion(QuestionType, TableIndex, RowNames, QuestionIndex, ContextString);
	AnsweringPlayers.Empty();
	AssignAnsweringPlayers();
	SetViewTargetForQuestion(QuestionType, RowNames, QuestionIndex);
	NumberOfSentAnswers = 0;
	if (!OnAnswerSent.IsBound())
		OnAnswerSent.AddDynamic(this, &ABM_GameModeBase::ResetQuestionTimer);
	StartQuestionTimer();
}

void ABM_GameModeBase::StartQuestionTimer()
{
	GetWorld()->GetTimerManager().SetTimer(QuestionTimerHandle, this, &ABM_GameModeBase::GatherPlayersAnswers, QuestionTimer, false);
}

void ABM_GameModeBase::ResetQuestionTimer(int32 LastSentPlayer)
{
	NumberOfSentAnswers++;
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController= Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_MarkAnsweredPlayers(LastSentPlayer);
		}
	}
	switch (Round)
	{
		case EGameRound::SetTerritory:
		{
			if(NumberOfSentAnswers == NumberOfActivePlayers)
			{
				GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
				GatherPlayersAnswers();
			}
			break;
		}
		case EGameRound::FightForTheRestTiles:
		{
			if(NumberOfSentAnswers == NumberOfActivePlayers)
			{
				GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
				GatherPlayersAnswers();
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			if(NumberOfSentAnswers == 2)
			{
				GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
				GatherPlayersAnswers();
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameModeBase::GatherPlayersAnswers()
{
	CurrentAnsweredQuestions.Empty();
	//Remove Question Widget and add Player's answer to the CurrentAnswers array
	if (Round != EGameRound::FightForTerritory)
	{
		for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			// Answers may not be pushed by the Player manually
			if(Cast<ABM_PlayerState>(PlayerState)->CurrentQuestionAnswerSent == false)
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
	}
	else
	{
		ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]);
		ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[DefendingPlayerID]);
		if(AttackingPlayerState && AttackingPlayerState->CurrentQuestionAnswerSent == false)
			AttackingPlayerState->AnsweredQuestions.Add(LastQuestion);
		if (DefendingPlayerState && DefendingPlayerState->CurrentQuestionAnswerSent == false)
			DefendingPlayerState->AnsweredQuestions.Add(LastQuestion);
		CurrentAnsweredQuestions.Add(Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID])->AnsweredQuestions.Last());
		CurrentAnsweredQuestions.Add(Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[DefendingPlayerID])->AnsweredQuestions.Last());
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameModeBase::ShowCorrectAnswers, 1.0f, false);
}

void ABM_GameModeBase::ShowCorrectAnswers()
{
	TArray<FPlayerChoice> PlayersChoices;
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
						PlayersChoices.Add(FPlayerChoice(AnsweredQuestion.PlayerID, i, AnsweredQuestion.ElapsedTime));
					}
				}
			}
			break;
		}
		case EQuestionType::Shot:
		{
			for (const auto AnsweredQuestion: CurrentAnsweredQuestions)
			{
				PlayersChoices.Add(FPlayerChoice(AnsweredQuestion.PlayerID ,AnsweredQuestion.AnswerShot.PlayerAnswer, AnsweredQuestion.ElapsedTime));
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

void ABM_GameModeBase::SetNextGameRound() {
	//If GameRound == SetTerritory,
	// check if there are available tiles and their amount == NumberOfActivePlayers%
	// if yes, Continue SetTerritory round
	// if no, Start Battle Mode for the rest of the tiles
	// If GameRound == FightForTerritory, TODO: check how many Player Turns are left
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
			if (bShotQuestionIsNeeded)
			{
				OpenQuestion(EQuestionType::Shot);
				break;
			}
			if(ABM_PlayerState* AttackingPlayer = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]))
			{
				AttackingPlayer->NumberOfTurns--;
			}
			NumberOfTotalTurns--;
			if (NumberOfActivePlayers < 2 || NumberOfTotalTurns == 0)
			{
				Round = NextGameRound();
				CountResults();
			}
			else
			{
				CurrentPlayerID++;
				if (CurrentPlayerID >= NumberOfActivePlayers)
					CurrentPlayerID = 0;
				StartPlayerTurnTimer(CurrentPlayerID);
			}
			break;
		}
	}
}

void ABM_GameModeBase::VerifyAnswers()
{
	bShotQuestionIsNeeded = false;
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
			VerifyChooseAnswers();
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
	SetNextGameRound();
}

void ABM_GameModeBase::VerifyChooseAnswers()
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
			// if both Players answered their Choose Question, we need a Shot round
			if (CurrentAnsweredQuestions[0].bWasAnswered && CurrentAnsweredQuestions[1].bWasAnswered)
			{
				bShotQuestionIsNeeded = true;
			}
			else
			{
				ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]->GetPlayerController());
				ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
				ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(GetGameState<ABM_GameStateBase>()->PlayerArray[DefendingPlayerID]->GetPlayerController());
				ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayer->PlayerState);
				for (const auto AnsweredQuestion : CurrentAnsweredQuestions)
				{
					if(AnsweredQuestion.bWasAnswered)
					{
						// correct answer
						if(AnsweredQuestion.PlayerID == CurrentPlayerID)
						{
							// attacking player was right
							DefendingPlayer->CurrentClickedTile->RemoveTileFromPlayerTerritory(DefendingPlayerState);
							AttackingPlayer->CurrentClickedTile->AddTileToPlayerTerritory(AttackingPlayerState);
						}
						else
						{
							// defending player was right
							AttackingPlayer->CurrentClickedTile->CancelAttack();
							DefendingPlayerState->AddPoints(200);
						}
						break;
					}
				}
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

EGameRound ABM_GameModeBase::NextGameRound() const
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
	UE_LOG(LogTemp, Warning, TEXT("Current PlayerID %d"), PlayerID);
	CurrentTurnTimer = TurnTimer;
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			if (PlayerID < NumberOfActivePlayers)
				PlayerController->UpdateCurrentPlayerNickname(Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[PlayerID])->Nickname);
			Cast<ABM_PlayerState>(PlayerState)->CurrentQuestionAnswerSent = false;
			PlayerController->ResetTurnTimer(Round);
			if(Cast<ABM_PlayerState>(PlayerState)->BMPlayerID == PlayerID)
			{
				Cast<ABM_PlayerState>(PlayerState)->SetPlayerTurn(true);
			}
			else
			{
				Cast<ABM_PlayerState>(PlayerState)->SetPlayerTurn(false);
			}
		}
	}
	for (const auto Tile : Tiles)
	{
		ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
		BMTile->MC_RemoveHighlighting();
		BMTile->MC_ShowEdges(false, FColor::Black);
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
							const ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[CurrentPlayerID]);
							//BMTile->TurnOnHighlight(PlayerState->MaterialNeighbour);
							BMTile->MC_ShowEdges(true, PlayerState->PlayerColor);
							CurrentPlayerAvailableTiles.Add(BMTile);
						}
					}
				}
				break;
			}
			case EGameRound::SetTerritory:
			{
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[PlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					if (NeighborTile->GetStatus() == ETileStatus::NotOwned && !NeighborTile->bIsAttacked)
					{
						UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
						//NeighborTile->TurnOnHighlight(CurrentPlayerState->MaterialNeighbour);
						NeighborTile->MC_ShowEdges(true, CurrentPlayerState->PlayerColor);
						CurrentPlayerAvailableTiles.Add(NeighborTile);
					}
				}
				if(CurrentPlayerAvailableTiles.Num()==0)
				{
					for (const auto Tile : Tiles)
					{
						ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
						if (BMTile->GetStatus() == ETileStatus::NotOwned && BMTile->bIsAttacked == false)
						{
							BMTile->MC_ShowEdges(true, CurrentPlayerState->PlayerColor);
							CurrentPlayerAvailableTiles.Add(BMTile);
							break;
						}
					}
				}
				break;
			}
			case EGameRound::FightForTerritory:
			{
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(GetGameState<ABM_GameStateBase>()->PlayerArray[PlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					if (!CurrentPlayerState->OwnedTiles.Contains(NeighborTile))
					{
						UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
						//NeighborTile->TurnOnHighlight(CurrentPlayerState->MaterialNeighbour);
						NeighborTile->MC_ShowEdges(true, CurrentPlayerState->PlayerColor);
						CurrentPlayerAvailableTiles.Add(NeighborTile);
					}
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

void ABM_GameModeBase::CountResults()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_ShowResultsWidget(GetGameState<ABM_GameStateBase>()->PlayerArray);
		}
	}
}

void ABM_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	InitPlayer(NewPlayer);
	NumberOfActivePlayers++;
}

void ABM_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	TArray<TObjectPtr<AActor>> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass() ,FoundCameras);
	for (auto Camera:FoundCameras)
	{
		if(Camera->ActorHasTag(TEXT("ChooseQuestion")))
			ChooseQuestionCamera = Cast<ACameraActor>(Camera);
		if(Camera->ActorHasTag(TEXT("ShotQuestion")))
			ShotQuestionCamera = Cast<ACameraActor>(Camera);
	}
}

void ABM_GameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_GameModeBase, Round);
}
