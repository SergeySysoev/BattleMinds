// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameStateBase.h"
#include "BM_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerState.h"
#include "Player/BM_PlayerControllerBase.h"

void ABM_GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_GameStateBase, Round);
	DOREPLIFETIME(ABM_GameStateBase, LastQuestion);
}

void ABM_GameStateBase::SetDefendingPlayerID(int32 InID)
{
	DefendingPlayerID = InID;
}

void ABM_GameStateBase::RequestToClearPlayerTurnTimer()
{
	GetWorldTimerManager().ClearTimer(PlayerTurnHandle);
}

void ABM_GameStateBase::RequestToOpenQuestion(EQuestionType QuestionType)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), QuestionType);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
}

void ABM_GameStateBase::PassTurnToTheNextPlayer()
{
	CurrentPlayerID++;
	StartPlayerTurnTimer(CurrentPlayerID);
}

void ABM_GameStateBase::RequestToStartPlayerTurnTimer(int32 PlayerID)
{
	StartPlayerTurnTimer(PlayerID);
}

ABM_PlayerControllerBase* ABM_GameStateBase::GetPlayerController(int32 PlayerID)
{
	if (PlayerArray.IsValidIndex(PlayerID))
	{
		return Cast<ABM_PlayerControllerBase>(PlayerArray[PlayerID]->GetPlayerController());
	}
	return nullptr;
}


void ABM_GameStateBase::InitGameState()
{
	TSubclassOf<ABM_TileBase> LTileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> LFoundTiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), LTileClass, LFoundTiles);
	Tiles.Append(LFoundTiles);
	Round = EGameRound::ChooseCastle;
	if (IsValid(BMGameMode))
	{
		NumberOfTotalTurns = BMGameMode->GetNumberOfTotalTurns();
	}
}

void ABM_GameStateBase::BeginPlay()
{
	Super::BeginPlay();
	BMGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
}

void ABM_GameStateBase::UpdatePlayersHUD()
{
	for (const auto PlayerState: PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_UpdatePlayerHUD();
		}
	}
}

void ABM_GameStateBase::SetViewTargetForQuestion(EQuestionType QuestionType, TArray<FName> RowNames, int32 QuestionIndex) const
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			TObjectPtr<AActor> LNewViewTarget = nullptr;
			LNewViewTarget = Cast<AActor>(BMGameMode->GetQuestionCamera(QuestionType));
			PlayerController->CC_OpenQuestionWidget(LastQuestion, AnsweringPlayers, LNewViewTarget);
		}
	}
}

int32 ABM_GameStateBase::FindNextQuestion(EQuestionType Question, TArray<FName> Array, FString& ContextString)
{
	if (!IsValid(BMGameMode))
	{
		return -1;
	}
	
	int32 QuestionIndex = -1;
	FQuestionChooseText LFoundRowChoose;
	FQuestionShot LFoundRowShot;
	UDataTable* LRandDataTable = BMGameMode->GetQuestionsDataTable(Question);
	if (!IsValid(LRandDataTable))
	{
		return -1;
	}
	
	switch (Question)
	{
		case EQuestionType::Choose:
			ContextString = FString("Question Choose");
			Array.Append(LRandDataTable->GetRowNames());
			QuestionIndex = FMath::RandRange(0, Array.Num()-1);
			LFoundRowChoose = std::ref(*LRandDataTable->FindRow<FQuestionChooseText>(Array[QuestionIndex],ContextString));
			LFoundRowChoose.QuestionID = UsedQuestions.Num();
			LastQuestion = FInstancedStruct::Make(LFoundRowChoose);
			UsedQuestions.Add(LastQuestion);
			break;
		
		case EQuestionType::Shot:
			ContextString = FString("Question Shot");
			Array.Append(LRandDataTable->GetRowNames());
			QuestionIndex = FMath::RandRange(0, Array.Num()-1);
			LFoundRowShot = std::ref(*LRandDataTable->FindRow<FQuestionShot>(Array[QuestionIndex],ContextString));
			LFoundRowShot.QuestionID = UsedQuestions.Num();
			LastQuestion = FInstancedStruct::Make(LFoundRowShot);
			UsedQuestions.Add(LastQuestion);
			break;
		default:
			break;
	}
	
	if (QuestionIndex < 0)
	{
		QuestionIndex = 0;
	}
	
	return QuestionIndex;
}

void ABM_GameStateBase::AssignAnsweringPlayers()
{
	if(Round != EGameRound::FightForTerritory)
	{
		for (const auto PlayerState : PlayerArray)
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

void ABM_GameStateBase::StopAllTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
}

void ABM_GameStateBase::OpenQuestion(EQuestionType QuestionType)
{
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	QuestionsCount++;
	TArray<FName> RowNames;
	FString ContextString = "OpenQuestion";
	int32 QuestionIndex = FindNextQuestion(QuestionType, RowNames, ContextString);
	FTableRowBase OutRow;
	AnsweringPlayers.Empty();
	AssignAnsweringPlayers();
	SetViewTargetForQuestion(QuestionType, RowNames, QuestionIndex);
	NumberOfSentAnswers = 0;
	if (!OnAnswerSent.IsBound())
	{
		OnAnswerSent.AddDynamic(this, &ABM_GameStateBase::ResetQuestionTimer);
	}
	StartQuestionTimer();
}

void ABM_GameStateBase::StartQuestionTimer()
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	// Gather Players answers after QuestionTimer seconds
	GetWorld()->GetTimerManager().SetTimer(QuestionTimerHandle, this, &ABM_GameStateBase::GatherPlayersAnswers, BMGameMode->GetQuestionTimer(), false);
}

void ABM_GameStateBase::ResetQuestionTimer(int32 LastSentPlayer)
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	
	NumberOfSentAnswers++;
	for (const auto PlayerState : PlayerArray)
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
			if(NumberOfSentAnswers == BMGameMode->GetNumberOfActivePlayers())
			{
				GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
				GatherPlayersAnswers();
			}
			break;
		}
		case EGameRound::FightForTheRestTiles:
		{
			if(NumberOfSentAnswers == BMGameMode->GetNumberOfActivePlayers())
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

void ABM_GameStateBase::GenerateAutoPlayerChoice(ABM_PlayerState* const PlayerState)
{
	switch (LastQuestion.GetPtr<FQuestion>()->GetType())
	{
		case EQuestionType::Choose:
		{
			// Generate auto choice Choose
			FInstancedStruct LAutoChoice = FInstancedStruct::Make(FPlayerChoiceChoose());
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().PlayerID = PlayerState->BMPlayerID;
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().AnswerID = -1;
			PlayerState->AnsweredQuestions.Add(LAutoChoice);
			break;
		}
		case EQuestionType::Shot:
		{
			// Generate auto choice Shot
			FInstancedStruct LAutoChoice = FInstancedStruct::Make(FPlayerChoiceShot());
			LAutoChoice.GetMutable<FPlayerChoiceShot>().PlayerID = PlayerState->BMPlayerID;
			LAutoChoice.GetMutable<FPlayerChoiceShot>().Answer = -1;
			PlayerState->AnsweredQuestions.Add(LAutoChoice);
			break;
		}
		default: break;
	}
}

void ABM_GameStateBase::GatherPlayersAnswers()
{
	PlayersCurrentChoices.Empty();
	//Remove Question Widget and add Player's answer to the CurrentAnswers array
	if (Round != EGameRound::FightForTerritory)
	{
		for (const auto PlayerState : PlayerArray)
		{
			const auto LPlayerState = Cast<ABM_PlayerState>(PlayerState);
			// Answers may not be pushed by the Player manually
			if (!IsValid(LPlayerState))
			{
				continue;
			}
			
			if(LPlayerState->CurrentQuestionAnswerSent == false)
			{
				GenerateAutoPlayerChoice(LPlayerState);
			}
			PlayersCurrentChoices.Add(LPlayerState->AnsweredQuestions.Last());
		}
	}
	else
	{
		ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]);
		ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(PlayerArray[DefendingPlayerID]);

		if (IsValid(AttackingPlayerState) && IsValid(DefendingPlayerState))
		{
			if(AttackingPlayerState->CurrentQuestionAnswerSent == false)
			{
				GenerateAutoPlayerChoice(AttackingPlayerState);
			}
			if (DefendingPlayerState->CurrentQuestionAnswerSent == false)
			{
				GenerateAutoPlayerChoice(DefendingPlayerState);
			}
			PlayersCurrentChoices.Add(AttackingPlayerState->AnsweredQuestions.Last());
			PlayersCurrentChoices.Add(DefendingPlayerState->AnsweredQuestions.Last());
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);

	//Show correct answers after 1 second
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::ShowCorrectAnswers, 1.0f, false);
}

void ABM_GameStateBase::ConstructQuestionResult(ABM_PlayerState* InPlayerState, int32 InQuestionNumber, FInstancedStruct InQuestion, TArray<FInstancedStruct> InPlayerChoices, int32 InReceivedPoints, bool InWasAnswered)
{
	if (!IsValid(InPlayerState))
	{
		return;
	}
	FQuestionResult LQuestionResult;
	LQuestionResult.QuestionNumber = InQuestionNumber;
	if (const FQuestion* LQuestion = InQuestion.GetPtr<FQuestion>())
	{
		LQuestionResult.Question = InQuestion;
	}
	LQuestionResult.PlayerChoices.Append(InPlayerChoices);
	LQuestionResult.ReceivedPoints = InReceivedPoints;
	LQuestionResult.bWasAnswered = InWasAnswered;
	InPlayerState->QuestionResults.Add(LQuestionResult);
}

void ABM_GameStateBase::ShowCorrectAnswers()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_ShowCorrectAnswers(PlayersCurrentChoices);
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);

	// Verify answers after 2 seconds
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::VerifyAnswers, 2.0f, false);
}

void ABM_GameStateBase::SetNextGameRound() {
	//If GameRound == SetTerritory,
	// check if there are available tiles and their amount == NumberOfActivePlayers%
	// if yes, Continue SetTerritory round
	// if no, Start Battle Mode for the rest of the tiles
	// If GameRound == FightForTerritory, check how many Player Turns are left
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
				if(NotOwnedTiles >= BMGameMode->GetNumberOfActivePlayers())
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
					CurrentPlayerID = BMGameMode->GetNumberOfActivePlayers();
					StartPlayerTurnTimer(CurrentPlayerID);
					UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round"));
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
				CurrentPlayerID = BMGameMode->GetNumberOfActivePlayers();
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
			if(ABM_PlayerState* AttackingPlayer = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]))
			{
				AttackingPlayer->NumberOfTurns--;
			}
			NumberOfTotalTurns--;
			if (BMGameMode->GetNumberOfActivePlayers() < 2 || NumberOfTotalTurns == 0)
			{
				Round = NextGameRound();
				FTimerHandle LCountResultsHandle;
				GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
			}
			else
			{
				CurrentPlayerID++;
				if (CurrentPlayerID >= BMGameMode->GetNumberOfActivePlayers())
				{
					CurrentPlayerID = 0;
				}
				StartPlayerTurnTimer(CurrentPlayerID);
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameStateBase::VerifyAnswers()
{
	bShotQuestionIsNeeded = false;
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_RemoveQuestionWidget();
		}
	}
	//Verify Players answers:
	switch (LastQuestion.GetPtr<FQuestion>()->Type)
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
	UpdatePlayersHUD();
	SetNextGameRound();
}

void ABM_GameStateBase::VerifyChooseAnswers()
{
	const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	switch (Round)
	{
		case EGameRound::SetTerritory:
		{
			for (const auto LPlayerChoice : PlayersCurrentChoices)
			{
				const auto LPlayerID =  LPlayerChoice.GetPtr<FPlayerChoice>()->PlayerID;
				const ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[LPlayerID]->GetPlayerController());
				ABM_PlayerState* CurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerID]);
				CurrentPlayerController->CurrentClickedTile->MC_RemoveSelection();
				if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
				{
					//Set Tile color back to default
					ConstructQuestionResult(CurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
					CurrentPlayerController->CurrentClickedTile->CancelAttack();
				}
				else
				{
					ConstructQuestionResult(CurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, CurrentPlayerController->CurrentClickedTile->GetPoints(), true);
					CurrentPlayerController->CurrentClickedTile->AddTileToPlayerTerritory(CurrentPlayerState);
				}
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerID]->GetPlayerController());
			ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
			ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[DefendingPlayerID]->GetPlayerController());
			ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayer->PlayerState);
			
			if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer
				&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
			{
				// if both Players answered their Choose Question, we need a Shot round
				bShotQuestionIsNeeded = true;
			}
			else if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer
				&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
			{
				// both Players gave wrong answer, cancel attack for Attacking, and don't give points to Defending
				ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
				ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
				AttackingPlayer->CurrentClickedTile->CancelAttack();
			}
			else
			{
				for (const auto AnsweredQuestion : PlayersCurrentChoices)
				{
					if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
					{
						// correct answer
						if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->PlayerID == CurrentPlayerID)
						{
							// attacking player was right
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1*DefendingPlayer->CurrentClickedTile->GetPoints(), false);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, DefendingPlayer->CurrentClickedTile->GetPoints(), true);
							DefendingPlayer->CurrentClickedTile->RemoveTileFromPlayerTerritory(DefendingPlayerState);
							AttackingPlayer->CurrentClickedTile->AddTileToPlayerTerritory(AttackingPlayerState);
						}
						else 
						{
							// defending player was right
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
							AttackingPlayer->CurrentClickedTile->CancelAttack();
							DefendingPlayerState->AddPoints(100);
						}
						break;
					}
				}
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameStateBase::VerifyShotAnswers()
{
	TArray<FPlayerChoiceShot> ShotChoices;
	for (const auto LPlayerChoice : PlayersCurrentChoices)
	{
		ShotChoices.Add(LPlayerChoice.Get<FPlayerChoiceShot>());
	}
	// Sort Answers by Difference and ElapsedTime (overriden < operator in FPlayerChoiceShot)
	if (ShotChoices.Num() > 1)
	{
		ShotChoices.Sort();
	}
	//TODO: REFACTOR THIS
	// because autoanswer sets -1 and automatically sorts to the first position
	if (ShotChoices[0].Answer < 0)
	{
		// No players have sent their answers
		for (const auto LShotChoice: ShotChoices)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[LShotChoice.PlayerID]->GetPlayerController());
			if (IsValid(LoserPlayerController) && IsValid(LoserPlayerController->CurrentClickedTile))
			{
				LoserPlayerController->CurrentClickedTile->CancelAttack();
				ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1*LoserPlayerController->CurrentClickedTile->GetPoints(), false);
			}
		}
		return;
	}
	const ABM_PlayerControllerBase* WinnerPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[0].PlayerID]->GetPlayerController());
	if (Round == EGameRound::FightForTheRestTiles)
	{
		for (int i =1; i < ShotChoices.Num(); i++)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[i].PlayerID]->GetPlayerController());
			if (LoserPlayerController && LoserPlayerController->CurrentClickedTile)
			{
				LoserPlayerController->CurrentClickedTile->CancelAttack();
				ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			}
		}
		if(WinnerPlayerController && WinnerPlayerController->CurrentClickedTile)
		{
			WinnerPlayerController->CurrentClickedTile->AddTileToPlayerTerritory(WinnerPlayerController->GetPlayerState<ABM_PlayerState>());
			ConstructQuestionResult(WinnerPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, WinnerPlayerController->CurrentClickedTile->GetPoints(), true);
		}
	}
	if (Round == EGameRound::FightForTerritory)
	{
		const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[DefendingPlayerID].PlayerID]->GetPlayerController());
		if (LoserPlayerController && LoserPlayerController->CurrentClickedTile)
		{
			LoserPlayerController->CurrentClickedTile->RemoveTileFromPlayerTerritory(Cast<ABM_PlayerState>(LoserPlayerController->PlayerState));
			ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1* LoserPlayerController->CurrentClickedTile->GetPoints(), false);
		}
		if(WinnerPlayerController && WinnerPlayerController->CurrentClickedTile)
		{
			WinnerPlayerController->CurrentClickedTile->AddTileToPlayerTerritory(WinnerPlayerController->GetPlayerState<ABM_PlayerState>());
			ConstructQuestionResult(WinnerPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, WinnerPlayerController->CurrentClickedTile->GetPoints(), true);
		}
	}
}

EGameRound ABM_GameStateBase::NextGameRound() const
{
	if(Round == EGameRound::ChooseCastle)
		return EGameRound::SetTerritory;
	if(Round == EGameRound::SetTerritory)
		return EGameRound::FightForTerritory;
	if(Round == EGameRound::FightForTerritory)
		return  EGameRound::End;
	return EGameRound::End;
}

void ABM_GameStateBase::ResetPlayersTurns(int32 PlayerID)
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			if (PlayerID < BMGameMode->GetNumberOfActivePlayers())
			{
				PlayerController->UpdateCurrentPlayerNickname(Cast<ABM_PlayerState>(PlayerArray[PlayerID])->Nickname);
			}
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
}

void ABM_GameStateBase::StartPlayerTurnTimer(int32 PlayerID)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	CurrentPlayerAvailableTiles.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("Current PlayerID %d"), PlayerID);
	CurrentTurnTimer = BMGameMode->GetTurnTimer()+1; // to be able to see timer widget fades away
	ResetPlayersTurns(PlayerID);
	
	for (const auto Tile : Tiles)
	{
		ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
		BMTile->MC_RemoveHighlighting();
		BMTile->MC_ShowEdges(false, FColor::Black);
	}
	
	if (CurrentPlayerID < BMGameMode->GetNumberOfActivePlayers())
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
						bool NeighborsAvailable = true;
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
							const ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]);
							BMTile->MC_ShowEdges(true, PlayerState->PlayerColor);
							CurrentPlayerAvailableTiles.Add(BMTile);
						}
					}
				}
				break;
			}
			case EGameRound::SetTerritory:
			{
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[PlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					if (NeighborTile->GetStatus() == ETileStatus::NotOwned && !NeighborTile->bIsAttacked)
					{
						UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
						NeighborTile->MC_ShowEdges(true, CurrentPlayerState->PlayerColor);
						CurrentPlayerAvailableTiles.Add(NeighborTile);
					}
				}
				// If there are no neighbour tiles, check other tiles and suggest them for attack
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
				const auto CurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[PlayerID]);
				for (const auto NeighborTile : CurrentPlayerState->GetNeighbors())
				{
					if (!CurrentPlayerState->OwnedTiles.Contains(NeighborTile))
					{
						UE_LOG(LogBM_GameMode, Display, TEXT("Neighbor tile: %s"), *NeighborTile->GetActorNameOrLabel());
						NeighborTile->MC_ShowEdges(true, CurrentPlayerState->PlayerColor);
						CurrentPlayerAvailableTiles.Add(NeighborTile);
					}
				}
				break;
			}
			default:
				break;
		}
		GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameStateBase::UpdatePlayerTurnTimers, 1.0, true, 0.0f);
	}
	else
	{
		FTimerDelegate LNextRound;
		switch (Round)
		{
			case EGameRound::ChooseCastle:
				//All players set their castles, continue to SetTerritory round with 2 sec delay
				CurrentPlayerID = 0;
				Round = NextGameRound();
				LNextRound.BindUObject(this, &ThisClass::StartPlayerTurnTimer, CurrentPlayerID);
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRound, 2.0, false);
				break;
			case EGameRound::SetTerritory:
				//All players have chosen their tiles, open Choose Question with 2sec delay
				QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Choose);
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
				break;
			case EGameRound::FightForTheRestTiles:
				//Start Shot question for the rest tiles
				//Find first not owned tile
				for (auto Tile: Tiles)
				{
					auto FoundTile = Cast<ABM_TileBase>(Tile);
					if (FoundTile->GetStatus() == ETileStatus::NotOwned)
					{
						for (const auto PlayerState : PlayerArray)
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
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
				break;
			case EGameRound::FightForTerritory:
				break;
			default: break;
		}
	}
}

void ABM_GameStateBase::ChooseFirstAvailableTileForPlayer(int32 PlayerID)
{
	const int32 RandomTileIndex = FMath::RandRange(0, CurrentPlayerAvailableTiles.Num()-1);
	if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[PlayerID]->GetPlayerController()))
	{
		PlayerController->SC_TryClickTheTile(CurrentPlayerAvailableTiles[RandomTileIndex]);
	}
}

void ABM_GameStateBase::UpdatePlayerTurnTimers()
{
	if(CurrentTurnTimer == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
		ChooseFirstAvailableTileForPlayer(CurrentPlayerID);
		for (const auto PlayerState : PlayerArray)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
			{
				PlayerController->ResetTurnTimer(Round);
			}
		}
	}
	else
	{
		for (const auto PlayerState : PlayerArray)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
			{
				PlayerController->UpdateTurnTimer(Round);
			}
		}
		CurrentTurnTimer--;
	}
}

void ABM_GameStateBase::CountResults()
{
	StopAllTimers();
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->bEnableClickEvents = false;
			PlayerController->bEnableTouchEvents = false;
			PlayerController->CC_ShowResultsWidget(PlayerArray);
		}
	}
}