// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameStateBase.h"

#include "BM_GameInstance.h"
#include "BM_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerState.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Tiles/BM_TileManager.h"

DEFINE_LOG_CATEGORY(BMLogGameStateBase);

//--------------------------------------Base methods---------------------------------------------//
void ABM_GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_GameStateBase, Round);
	DOREPLIFETIME(ABM_GameStateBase, LastQuestion);
}

void ABM_GameStateBase::CalculateAndSetMaxCyclesPerRound()
{
	TWeakObjectPtr<UEnum> LRounds = StaticEnum<EGameRound>();
	for (int i = 0; i < LRounds->GetMaxEnumValue(); i++)
	{
		switch (static_cast<EGameRound>(i))
		{
			case EGameRound::ChooseCastle:
			{
				MaxCyclesPerRound.Add(static_cast<EGameRound>(i), 1);
			}
			break;
			case EGameRound::SetTerritory:
			{
				MaxCyclesPerRound.Add(static_cast<EGameRound>(i),
					(TileManager->GetTilesCount()-BMGameMode->GetNumberOfActivePlayers()*2)/BMGameMode->GetNumberOfActivePlayers() + 1);
			}
			break;
			case EGameRound::FightForTheRestTiles:
			{
				MaxCyclesPerRound.Add(static_cast<EGameRound>(i), 3);
			}
			break;
			case EGameRound::FightForTerritory:
			{
				MaxCyclesPerRound.Add(static_cast<EGameRound>(i),
					(TileManager->GetTilesCount()-BMGameMode->GetNumberOfActivePlayers()*2)/BMGameMode->GetNumberOfActivePlayers() + 1);
			}
			break;
			default:break;
		}
	}
	ConstructPlayerTurnsCycles();
}

void ABM_GameStateBase::InitGameState()
{
	Round = EGameRound::ChooseCastle;
	TSubclassOf<ABM_TileManager> LTileManagerClass = ABM_TileManager::StaticClass();
	TileManager = Cast<ABM_TileManager>(UGameplayStatics::GetActorOfClass(GetWorld(), LTileManagerClass));
	TileManager->OnMapGeneratedNative.AddUObject(this, &ABM_GameStateBase::CalculateAndSetMaxCyclesPerRound);
	OpenChooseQuestionPtr = &ABM_GameStateBase::OpenChooseQuestion;
	StartSiegePtr = &ABM_GameStateBase::StartSiege;
	PassTurnToNextPlayerPtr = &ABM_GameStateBase::PassTurnToTheNextPlayer;
}

void ABM_GameStateBase::BeginPlay()
{
	Super::BeginPlay();
	BMGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
}

void ABM_GameStateBase::ConstructPlayerTurnsCycles()
{
	PlayerTurnsCycles.Empty();
	CurrentPlayerTurnsCycle = 0;
	switch (Round)
	{
		case EGameRound::ChooseCastle:
		{
			FPermutation LPermutation;
			for (int32 i = 0; i < PlayerArray. Num(); i++)
			{
				LPermutation.Values.Add(i);
			}
			PlayerTurnsCycles.Add(FPlayersCycle(0, LPermutation, false));
		}
			break;
		case EGameRound::FightForTheRestTiles:
		{
			// Construct N cycles of 1 element == 0
			FPermutation LPermutation;
			for (int32 i = 0; i < MaxCyclesPerRound.FindRef(EGameRound::FightForTheRestTiles); i++)
			{
				LPermutation.Values.Add(0);
				PlayerTurnsCycles.Add(FPlayersCycle(i, LPermutation, false));
			}
		}
			break;
		default:
		{
			TArray<int32> LElements;
			for (int32 i = 0; i < PlayerArray. Num(); i++)
			{
				LElements.Add(i);
			}
			TArray<FPermutation> LPermutations =
				UBM_Types::GenerateNumberOfPermutations(LElements, MaxCyclesPerRound.FindRef(Round));
			for (int32 i = 0; i < LPermutations.Num(); i++)
			{
				PlayerTurnsCycles.Add(FPlayersCycle(i, LPermutations[i], false));
			}
		}
			break;
	}
	UpdatePlayersTurnsWidget();
}

void ABM_GameStateBase::DisableTileEdgesHighlight()
{
	if (IsValid(TileManager))
	{
		TileManager->UnhighlightTiles();
	}
}

void ABM_GameStateBase::UnbindAllOnBannerSpawnedTiles()
{
	TileManager->UnbindAllOnBannerSpawnedDelegates();
}

int32 ABM_GameStateBase::GetNextPlayerArrayIndex() const
{
	if (!PlayerTurnsCycles.IsEmpty())
	{
		if (PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter))
		{
			return PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values[CurrentPlayerCounter];
		}
	}
	return -1;
}

int32 ABM_GameStateBase::GetPreviousPlayerArrayIndex() const
{
	if (!PlayerTurnsCycles.IsEmpty())
	{
		if (PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter-1))
		{
			return PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values[CurrentPlayerCounter-1];
		}
	}
	return -1;
}

void ABM_GameStateBase::StopPlayerTurnTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
		}
	}
}

int32 ABM_GameStateBase::CountNotOwnedTiles()
{
	return TileManager->GetUncontrolledTiles().Num();
}

//-----------------------------------END Base methods---------------------------------------------//

//--------------------------------------Player methods-------------------------------------------//

ABM_PlayerControllerBase* ABM_GameStateBase::GetPlayerController(int32 PlayerIndex)
{
	if (PlayerArray.IsValidIndex(PlayerIndex))
	{
		return Cast<ABM_PlayerControllerBase>(PlayerArray[PlayerIndex]->GetPlayerController());
	}
	return nullptr;
}

TArray<FIntPoint> ABM_GameStateBase::GetPlayerAvailableTiles(EGameRound CurrentRound, int32 PlayerIndex) const
{
	TArray<FIntPoint> AvailableTiles;
	if (IsValid(TileManager))
	{
		AvailableTiles.Append(TileManager->GetCurrentPlayerAvailableTilesAxials(CurrentRound, PlayerIndex));
	}
	return AvailableTiles;
}

FLinearColor ABM_GameStateBase::GetPlayerColorByIndex(int32 PlayerIndex) const
{
	for (const auto Player : PlayerArray)
	{
		ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(Player); 
		if (IsValid(LPlayerState) && LPlayerState->GetPlayerIndex() == PlayerIndex)
		{
			UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
			if (IsValid(LGameInstance))
			{
				return LGameInstance->AnswersColors.FindRef(LPlayerState->GetPlayerColor());
			}
		}
	}
	return FLinearColor::White;
}

void ABM_GameStateBase::SetDefendingPlayer(FIntPoint InClickedTile)
{
	check(HasAuthority());
	DefendingPlayerIndex = TileManager->GetTileOwnerID(InClickedTile);
	TileManager->HandleClickedTile(DefendingPlayerIndex, InClickedTile);
}

void ABM_GameStateBase::ClearPlayerTurnTimer()
{
	GetWorldTimerManager().ClearTimer(PlayerTurnHandle);
}

void ABM_GameStateBase::ChooseFirstAvailableTileForPlayer(int32 PlayerIndex)
{
	const auto LAvailableTiles = GetPlayerAvailableTiles(Round, PlayerIndex);
	const int32 RandomTileIndex = FMath::RandRange(0, LAvailableTiles.Num()-1);
	TileManager->HandleClickedTile(PlayerIndex, LAvailableTiles[RandomTileIndex]);
	HandleClickedTile(LAvailableTiles[RandomTileIndex]);
}

void ABM_GameStateBase::ConstructQuestionResult(ABM_PlayerState* InPlayerState, int32 InQuestionNumber, FInstancedStruct InQuestion, TArray<FInstancedStruct> InPlayerChoices, int32 InReceivedPoints, bool InWasAnswered)
{
	if (!IsValid(InPlayerState))
	{
		return;
	}
	FQuestionResult LQuestionResult;
	LQuestionResult.QuestionNumber = InQuestionNumber;
	if (InQuestion.GetPtr<FQuestion>() != nullptr)
	{
		LQuestionResult.Question = InQuestion;
	}
	LQuestionResult.PlayerChoices.Append(InPlayerChoices);
	LQuestionResult.ReceivedPoints = InReceivedPoints;
	LQuestionResult.bWasAnswered = InWasAnswered;
	InPlayerState->QuestionResults.Add(LQuestionResult);
}
//----------------------------------END Player methods-------------------------------------------//

//--------------------------------------Game flow methods----------------------------------------//
EGameRound ABM_GameStateBase::GetNextGameRound() const
{
	if(Round == EGameRound::ChooseCastle)
		return EGameRound::SetTerritory;
	if(Round == EGameRound::SetTerritory)
		return EGameRound::FightForTerritory;
	if(Round == EGameRound::FightForTerritory)
		return  EGameRound::End;
	return EGameRound::End;
}

void ABM_GameStateBase::SetNextGameRound(EGameRound NewRound)
{
	Round = NewRound;
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			break;
		case EGameRound::SetTerritory:
			break;
		default: break;
	}
}

void ABM_GameStateBase::StopAllTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
}

void ABM_GameStateBase::PassTurnToTheNextPlayer()
{
	check(HasAuthority());

	CurrentPlayerIndex = GetNextPlayerArrayIndex();
	if (PlayerArray.IsValidIndex(CurrentPlayerIndex))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		if (IsValid(LCurrentPlayerState))
		{
			LCurrentPlayerState->SetPlayerTurn(true);
			UpdatePlayerTurn();
			int32 LPreviousPlayerArrayIndex = GetPreviousPlayerArrayIndex();
			if (PlayerArray.IsValidIndex(LPreviousPlayerArrayIndex))
			{
				ABM_PlayerState* LPreviousPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPreviousPlayerArrayIndex]);
				if (IsValid(LPreviousPlayerState))
				{
					LPreviousPlayerState->SetPlayerTurn(false);
				}
			}
		}
		StartPlayerTurnTimer(CurrentPlayerIndex);
	}
	else
	{
		CurrentPlayerCounter = 0;
		WrapUpCurrentPlayersCycle();
	}
}

void ABM_GameStateBase::HandleClickedTile(FIntPoint InClickedTile)
{
	check(HasAuthority());
	ABM_PlayerControllerBase* LCurrentPlayerController = GetPlayerController(CurrentPlayerIndex);
	ABM_PlayerState* LCurrentPlayerState = LCurrentPlayerController->GetPlayerState<ABM_PlayerState>();
	TileManager->HandleClickedTile(CurrentPlayerIndex, InClickedTile);
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			/* Player chose their Castle tile
			 * so add it to their territory
			 and pass the turn to the nex Player when Castle mesh is spawned*/
			TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerIndex, ETileStatus::Castle, LCurrentPlayerState->GetPlayerColor(), Round);
			LCurrentPlayerState->SC_ChangePoints(TileManager->GetPointsOfTile(InClickedTile));
			MC_UpdatePoints(LCurrentPlayerState->GetPlayerIndex(),LCurrentPlayerState->GetPoints());
			CurrentPlayerCounter++;
			DisableTileEdgesHighlight();
			StopPlayerTurnTimer();
			TileManager->BindPassTurnToTileCastleMeshSpawned(InClickedTile);
			break;
		case EGameRound::SetTerritory:
			/* Spawn Attacking Banner Mesh on Clicked Tile, pass Turn to the next player, after all players choose their tile
			 the GameState will open the question */
			{
				DisableTileEdgesHighlight();
				StopPlayerTurnTimer();
				TileManager->SC_AttackTile(InClickedTile, LCurrentPlayerState->GetPlayerColor());
				TileManager->BindGameStateToTileBannerMeshSpawned(InClickedTile, PassTurnToNextPlayerPtr);
				CurrentPlayerCounter++;
			}
			break;
		case EGameRound::FightForTheRestTiles:
			/* In this round Player don't manually choose the tiles,
			 * it's done automatically */
			break;
		case EGameRound::FightForTerritory:
			// TODO: need to check if the click was on the owning Tile (for fortification)
			//  in that case no question is needed
			TileManager->SC_AttackTile(InClickedTile, LCurrentPlayerState->GetPlayerColor());
			CurrentSiegeTileQuestionCount = TileManager->GetTileQuestionsCount(InClickedTile);
			SetDefendingPlayer(InClickedTile);
			DisableTileEdgesHighlight();
			StopPlayerTurnTimer();
			TileManager->BindGameStateToTileBannerMeshSpawned(InClickedTile, OpenChooseQuestionPtr);
			break;
		default: break;
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
			PlayerController->SetPlayerBorderBlinking(false);
			PlayerController->CC_OpenQuestionWidget(LastQuestion, AnsweringPlayers, LNewViewTarget);
			
		}
	}
}

void ABM_GameStateBase::StartPlayerTurnTimer(int32 PlayerArrayIndex)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	
	CurrentTurnTimer = BMGameMode->GetTurnTimer()+1; // to be able to see timer widget fades away
	//ResetPlayersTurns(PlayerID);
	
	DisableTileEdgesHighlight();
	HighlightAvailableTiles(PlayerArrayIndex);
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
		}
	}
	
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->StartCountdownTimer();
		}
	}
	float LTurnSeconds = BMGameMode->GetTurnTimer();
	FTimerDelegate LAutoChooseTileDelegate;
	LAutoChooseTileDelegate.BindUObject(this, &ABM_GameStateBase::ForceChooseAvailableTile, CurrentPlayerIndex);
	GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, LAutoChooseTileDelegate, LTurnSeconds, false);
}

void ABM_GameStateBase::HighlightAvailableTiles(int32 PlayerArrayIndex)
{
	const ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerArray[PlayerArrayIndex]);
	if (IsValid(TileManager) && IsValid(LPlayerState))
	{
		TileManager->SC_HighlightAvailableTiles(Round, LPlayerState->GetPlayerIndex(), LPlayerState->GetPlayerColor());
	}
}

void ABM_GameStateBase::ForceChooseAvailableTile(int32 PlayerArrayIndex)
{
	if (IsValid(TileManager) && PlayerArray.IsValidIndex(PlayerArrayIndex))
	{
		ChooseFirstAvailableTileForPlayer(PlayerArrayIndex);	
	}
}

void ABM_GameStateBase::StartSiege()
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerIndex]->GetOwningController());
	if (!IsValid(LCurrentPlayerController))
	{
		return;
	}
	ABM_TileBase* LCurrentClickedTile = nullptr;		//TODO
	if (!IsValid(LCurrentClickedTile))
	{
		return;
	}
	RequestToOpenQuestion(LCurrentClickedTile->GetTileNextQuestionType());
}

void ABM_GameStateBase::RequestToOpenQuestion(EQuestionType QuestionType)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), QuestionType);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
}

void ABM_GameStateBase::OpenChooseQuestion()
{
	RequestToOpenQuestion(EQuestionType::Choose);
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
		OnAnswerSent.AddDynamic(this, &ABM_GameStateBase::OnPlayerAnswerSent);
	}
	StartQuestionTimer();
}

void ABM_GameStateBase::TransferDefendingPlayerTerritoryToAttacker()
{
	StopAllTimers();
	if (HasAuthority())
	{
		ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerIndex]->GetPlayerController());
		ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
		ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[DefendingPlayerIndex]->GetPlayerController());
		ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayer->PlayerState);
		
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

void ABM_GameStateBase::StartQuestionTimer()
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	// Gather Players answers after QuestionTimer seconds
	GetWorld()->GetTimerManager().SetTimer(QuestionTimerHandle, this, &ABM_GameStateBase::GatherPlayersAnswers, BMGameMode->GetQuestionTimer(), false);
}

void ABM_GameStateBase::OnPlayerAnswerSent(int32 LastSentPlayer)
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

void ABM_GameStateBase::AssignAnsweringPlayers()
{
	if(Round != EGameRound::FightForTerritory)
	{
		for (const auto PlayerState : PlayerArray)
		{
			AnsweringPlayers.Add(Cast<ABM_PlayerState>(PlayerState)->GetPlayerIndex());
		}
	}
	else
	{
		AnsweringPlayers.Add(CurrentPlayerCounter);
		AnsweringPlayers.Add(DefendingPlayerIndex);
	}
}

void ABM_GameStateBase::GenerateAutoPlayerChoice(ABM_PlayerState* const PlayerState) const
{
	switch (LastQuestion.GetPtr<FQuestion>()->GetType())
	{
		case EQuestionType::Choose:
		{
			// Generate auto choice Choose
			FInstancedStruct LAutoChoice = FInstancedStruct::Make(FPlayerChoiceChoose());
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().PlayerID = PlayerState->GetPlayerIndex();
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().AnswerID = -1;
			PlayerState->QuestionChoices.Add(LAutoChoice);
			break;
		}
		case EQuestionType::Shot:
		{
			// Generate auto choice Shot
			FInstancedStruct LAutoChoice = FInstancedStruct::Make(FPlayerChoiceShot());
			LAutoChoice.GetMutable<FPlayerChoiceShot>().PlayerID = PlayerState->GetPlayerIndex();
			LAutoChoice.GetMutable<FPlayerChoiceShot>().Answer = MAX_int32;
			LAutoChoice.GetMutable<FPlayerChoiceShot>().Difference = MAX_int32;
			LAutoChoice.GetMutable<FPlayerChoiceShot>().ElapsedTime = FTimespan::FromSeconds(BMGameMode->GetQuestionTimer());
			PlayerState->QuestionChoices.Add(LAutoChoice);
			break;
		}
		default: break;
	}
}

void ABM_GameStateBase::GatherPlayersAnswers()
{
	//TODO: triggered when the actual WBP_Timer is 2 seconds delayed 
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
			
			if(LPlayerState->CurrentQuestionAnswerSent == false)	// TODO: not set properly : Testcase - FightForTheRestTiles round with Shot question
			{
				GenerateAutoPlayerChoice(LPlayerState);
			}
			PlayersCurrentChoices.Add(LPlayerState->QuestionChoices.Last());
		}
	}
	else
	{
		ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(PlayerArray[DefendingPlayerIndex]);

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
			PlayersCurrentChoices.Add(AttackingPlayerState->QuestionChoices.Last());
			PlayersCurrentChoices.Add(DefendingPlayerState->QuestionChoices.Last());
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);

	//Verify answers after 1 second
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::VerifyAnswers, 1.0f, false);
}

void ABM_GameStateBase::VerifyAnswers()
{
	bShotQuestionIsNeeded = false;
	//Verify Players answers:
	if (LastQuestion.GetPtr<FQuestion>())
	{
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
		UnbindAllOnBannerSpawnedTiles();
		//Show Correct answers after 1 second
		//GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::ShowPlayerChoicesAndCorrectAnswer, 1.0f, false);
		ShowPlayerChoicesAndCorrectAnswer();
	}
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
				ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerID]);
				ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
				if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
				{
					// Wrong answer was given
					ConstructQuestionResult(LCurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
					TileManager->SC_CancelAttackOnClickedTile(LPlayerID);
				}
				else
				{
					// Correct answer was given
					int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
					ConstructQuestionResult(LCurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
					LCurrentPlayerState->SC_ChangePoints(LPoints);
					TileManager->SC_AddClickedTileToTheTerritory(LPlayerID, ETileStatus::Controlled, LCurrentPlayerState->GetPlayerColor(), Round);
					MC_UpdatePoints(LPlayerID,LCurrentPlayerState->GetPoints());
				}
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerIndex]->GetPlayerController());
			ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
			ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[DefendingPlayerIndex]->GetPlayerController());
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
				TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);
			}
			else // Someone got the right answer
			{
				CurrentSiegeTileQuestionCount--;
				for (const auto AnsweredQuestion : PlayersCurrentChoices)
				{
					if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
					{
						// correct answer
						if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->PlayerID == CurrentPlayerIndex)
						{
							// attacking player was right
							if (CurrentSiegeTileQuestionCount == 0)
							{
								DefendingPlayerState->SC_ChangePoints(-TileManager->GetPointsOfCurrentClickedTile(DefendingPlayerIndex));
								AttackingPlayerState->SC_ChangePoints(TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex));
								TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerIndex, ETileStatus::Controlled, AttackingPlayerState->GetPlayerColor(), Round);
								MC_UpdatePoints(DefendingPlayerIndex,DefendingPlayerState->GetPoints());
								MC_UpdatePoints(CurrentPlayerIndex,AttackingPlayerState->GetPoints());
							}
							else
							{
								// Apply 1Damage to Current Clicked by Attacking player Tile
							}
							int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1 * LPoints, false);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
						}
						else 
						{
							// defending player was right
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
							TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);
							DefendingPlayerState->SC_ChangePoints(100);
							MC_UpdatePoints(DefendingPlayerIndex,DefendingPlayerState->GetPoints());
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

PRAGMA_DISABLE_OPTIMIZATION

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
	CurrentSiegeTileQuestionCount--;
	if (ShotChoices[0].Answer == MAX_int32)   // No players have sent their answers
	{
		if (Round == EGameRound::FightForTheRestTiles)
		{
			for (const auto LShotChoice: ShotChoices)
			{
				const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[LShotChoice.PlayerID]->GetPlayerController());
				if (IsValid(LoserPlayerController))
				{
					TileManager->SC_CancelAttackOnClickedTile(LShotChoice.PlayerID);
					ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
				}
			}
			return;
		}
		if (Round == EGameRound::FightForTerritory)
		{
			// both players haven't sent their answers, cancel Attack and add 100 points to the Defender
			ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerIndex]->GetPlayerController());
			ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
			ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[DefendingPlayerIndex]->GetPlayerController());
			ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayer->PlayerState);
			TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);
			ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			DefendingPlayerState->SC_ChangePoints(100);
			MC_UpdatePoints(DefendingPlayerIndex,DefendingPlayerState->GetPoints());
			return;
		}
	}

	const ABM_PlayerControllerBase* WinnerPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[0].PlayerID]->GetPlayerController());;
	ABM_PlayerState* WinnerPlayerState = WinnerPlayerController->GetPlayerState<ABM_PlayerState>();
	if (Round == EGameRound::FightForTheRestTiles)
	{
		for (int i = 1; i < ShotChoices.Num(); i++)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[i].PlayerID]->GetPlayerController());
			if (LoserPlayerController)
			{
				//LoserPlayerController->SC_CancelAttackForCurrentTile();
				//TileManager->SC_CancelAttackOnClickedTile(ShotChoices[i].PlayerID);
				ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			}
		}
		if(IsValid(WinnerPlayerController) && IsValid(WinnerPlayerState))
		{
			//WinnerPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
			int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
			WinnerPlayerState->SC_ChangePoints(LPoints);
			TileManager->SC_AddClickedTileToTheTerritory(ShotChoices[0].PlayerID, ETileStatus::Controlled, WinnerPlayerState->GetPlayerColor(), Round);
			MC_UpdatePoints(WinnerPlayerState->GetPlayerIndex(),WinnerPlayerState->GetPoints());
			ConstructQuestionResult(WinnerPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
		}
	}
	if (Round == EGameRound::FightForTerritory)
	{
		ABM_PlayerState* LAttackingPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		ABM_PlayerState* LDefendingPlayerState = Cast<ABM_PlayerState>(PlayerArray[DefendingPlayerIndex]);
		int32 LWinnerIndex = ShotChoices[0].PlayerID;
		int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
		if (IsValid(LAttackingPlayerState) && IsValid(LDefendingPlayerState))
		{
			if (CurrentPlayerIndex == LWinnerIndex)		// Attacking Player was right
			{
				if (CurrentSiegeTileQuestionCount == 0)
				{
					LAttackingPlayerState->SC_ChangePoints(LPoints);
					LDefendingPlayerState->SC_ChangePoints(-LPoints);
					MC_UpdatePoints(LAttackingPlayerState->GetPlayerIndex(),LAttackingPlayerState->GetPoints());
					MC_UpdatePoints(LDefendingPlayerState->GetPlayerIndex(),LDefendingPlayerState->GetPoints());
					TileManager->SC_AddClickedTileToTheTerritory(ShotChoices[0].PlayerID, ETileStatus::Controlled, WinnerPlayerState->GetPlayerColor(), Round);	
				}
				else
				{
					// Apply Damage and continue with PrepareNextTurn()
				}
				ConstructQuestionResult(LDefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1* LPoints, false);
				ConstructQuestionResult(LAttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
			}
			else // Defending Player was right
			{
				LDefendingPlayerState->SC_ChangePoints(100);
				MC_UpdatePoints(LAttackingPlayerState->GetPlayerIndex(),LAttackingPlayerState->GetPoints());
				MC_UpdatePoints(LDefendingPlayerState->GetPlayerIndex(),LDefendingPlayerState->GetPoints());
				TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);	
			}
		}
	}
}
PRAGMA_ENABLE_OPTIMIZATION

void ABM_GameStateBase::ShowPlayerChoicesAndCorrectAnswer()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_ShowCorrectAnswers(PlayersCurrentChoices);
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	
	// Process to the next turn after 3 seconds
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::PrepareNextTurn, 3.0f, false);
}

void ABM_GameStateBase::HandleSiegedTile(ABM_PlayerControllerBase* AttackingPlayerController, ABM_PlayerControllerBase* DefendingPlayerController, bool QuestionWasAnsweredByAttacker)
{
	ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayerController);
	ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayerController);
	switch (LastQuestion.GetPtr<FQuestion>()->Type)
	{
		case EQuestionType::Choose:
			if (QuestionWasAnsweredByAttacker)
			{
				int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
				ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1 * LPoints, false);
				ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
				if (CurrentSiegeTileQuestionCount == 0)
				{
					/*DefendingPlayerController->SC_RemoveCurrentTileFromTerritory();
					AttackingPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);*/
					DefendingPlayerState->SC_ChangePoints(-LPoints);
					TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerIndex, ETileStatus::Controlled, AttackingPlayerState->GetPlayerColor(), Round);
				}
			}
			else
			{
				// defending player was right
				ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
				ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
				//AttackingPlayerController->SC_CancelAttackForCurrentTile();
				DefendingPlayerState->SC_ChangePoints(100);
				TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);
			}
			
			break;
		case EQuestionType::Shot:
			break;
		default: break;
	}
}

void ABM_GameStateBase::MC_UpdatePoints_Implementation(int32 PlayerID, float NewScore)
{
	OnPlayerPointsChanged.Broadcast(PlayerID, NewScore);
}

void ABM_GameStateBase::UpdatePlayersTurnsWidget()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdatePlayerTurnsAmount(PlayerTurnsCycles);
		}
	}
}

void ABM_GameStateBase::WrapUpCurrentPlayersCycle()
{
	ClearPlayerTurnTimer();
	FTimerDelegate LNextRound;
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			//All players set their castles, continue to SetTerritory round with 2 sec delay
			Round = GetNextGameRound();
			ConstructPlayerTurnsCycles();
			LNextRound.BindUObject(this, &ThisClass::PassTurnToTheNextPlayer);
			DisableTileEdgesHighlight();
			UpdatePlayersTurnsWidget();
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRound, 2.0, false);
			break;
		case EGameRound::SetTerritory:
			//All players have chosen their tiles, open Choose Question with 2sec delay
			DisableTileEdgesHighlight();
			// Mark current Cycle as completed, increment Cycle number
			PlayerTurnsCycles[CurrentPlayerTurnsCycle].bIsCompleted = true;
			CurrentPlayerTurnsCycle++;
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Choose);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
			break;
		case EGameRound::FightForTheRestTiles:
			//Start Shot question for the rest tiles
			//Find first not owned tile
			CurrentPlayerCounter = 0;
			UpdatePlayerTurn();
			for (int32 i = 0; i < PlayerArray.Num(); i++)
			{
				ForceChooseAvailableTile(i);
			}
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Shot);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
			CurrentPlayerTurnsCycle++;
			CurrentPlayerIndex = GetNextPlayerArrayIndex();
			break;
		case EGameRound::FightForTerritory:
			CurrentPlayerTurnsCycle++;
			if (!PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
			{
				Round = GetNextGameRound();
				FTimerHandle LCountResultsHandle;
				GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
				break;
			}
			PassTurnToTheNextPlayer();
			break;
		default: break;
	}
}

void ABM_GameStateBase::PrepareNextTurn()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			if (CurrentSiegeTileQuestionCount <= 0)
			{
				PlayerController->CC_RemoveQuestionWidget(true);
			}
			else
			{
				PlayerController->CC_RemoveQuestionWidget(false);
			}
		}
	}
	switch (Round)
	{
		case EGameRound::SetTerritory:
		{
			/*
			 * Count NotOwned tiles.
			 * If there are more than 0
			 *		If there are more than Players, continue SetTerritoryRound
			 *		Else start FightForTheRestTiles special round
			 * Else start FightForTerritory round
			 */
			int32 NotOwnedTiles = CountNotOwnedTiles();	
			if (NotOwnedTiles > 0)
			{
				if(NotOwnedTiles >= BMGameMode->GetNumberOfActivePlayers())
				{
					if (!PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
					{
						Round = EGameRound::FightForTheRestTiles;
						CurrentPlayerIndex = 0;
						ConstructPlayerTurnsCycles();
						UpdatePlayersTurnsWidget();
						// instantly Wrap Up Player Cycle as there is no choosing of tiles
						UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round due to Max SetTerritory turns reached"));
						WrapUpCurrentPlayersCycle();
						break;
					}
					PassTurnToTheNextPlayer();
				}
				else
				{
					Round = EGameRound::FightForTheRestTiles;
					CurrentPlayerIndex = 0;
					ConstructPlayerTurnsCycles();
					UpdatePlayersTurnsWidget();
					// instantly Wrap Up Player Cycle as there is no choosing of tiles
					WrapUpCurrentPlayersCycle();
					UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round"));
				}
			}
			else
			{
				Round = EGameRound::FightForTerritory;
				CurrentPlayerIndex = 0;
				ConstructPlayerTurnsCycles();
				UpdatePlayersTurnsWidget();
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, this, &ABM_GameStateBase::PassTurnToTheNextPlayer, 3.0f, false);
				//PassTurnToTheNextPlayer();
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Territory round"));
			}
			break;
		}
		case EGameRound::FightForTheRestTiles:
		{
			int32 NotOwnedTiles = CountNotOwnedTiles();
			if (NotOwnedTiles > 0 && PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
			{
				// instantly Wrap Up Player Cycle as there is no choosing of tiles
				WrapUpCurrentPlayersCycle();
				UE_LOG(LogBM_GameMode, Display, TEXT("Continue Fight for Last Tiles round"));
			}
			else
			{
				Round = EGameRound::FightForTerritory;
				CurrentPlayerIndex = 0;
				ConstructPlayerTurnsCycles();
				UpdatePlayersTurnsWidget();
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, this, &ABM_GameStateBase::PassTurnToTheNextPlayer, 3.0f, false);
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight For Territory round"));
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			/*
			 * check how many questions left to answer to conquer the tile
			 * if 0, then
			 *		check how many players are in game
			 *		if >=2,
			 *		check if the Cycle counter is not exceeds MaxCycles, continue FightForTerritory round
			 *		else
			 *		switch to the CountResults round
			 *	else Continue the siege
			 */
			if (bShotQuestionIsNeeded)
			{
				OpenQuestion(EQuestionType::Shot);
				break;
			}
			if (CurrentSiegeTileQuestionCount == 0)
			{
				if (BMGameMode->GetNumberOfActivePlayers() < 2 || !PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
				{
					Round = GetNextGameRound();
					FTimerHandle LCountResultsHandle;
					GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
				}
				else
				{
					CurrentPlayerCounter++;
					PassTurnToTheNextPlayer();
				}
			}
			else
			{
				//StartSiege();
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameStateBase::UpdatePlayerTurn()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdatePlayerTurnWidget(CurrentPlayerTurnsCycle, CurrentPlayerCounter);
			Cast<ABM_PlayerState>(PlayerState)->CurrentQuestionAnswerSent = false;
			PlayerController->ResetTurnTimer(Round);
		}
	}
}

void ABM_GameStateBase::UpdateGameMap() {}

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

//----------------------------------END Game flow methods----------------------------------------//