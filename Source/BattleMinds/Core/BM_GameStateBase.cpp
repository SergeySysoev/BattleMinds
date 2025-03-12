// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameStateBase.h"

#include "BM_GameInstance.h"
#include "BM_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerState.h"
#include "Player/BM_PlayerControllerBase.h"

//--------------------------------------Base methods---------------------------------------------//
void ABM_GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_GameStateBase, Round);
	DOREPLIFETIME(ABM_GameStateBase, LastQuestion);
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
		TotalSetTerritoryTurns = BMGameMode->GetNumberOfActivePlayers()*
			FMath::Floor(Tiles.Num()/BMGameMode->GetNumberOfActivePlayers() - 1);
		TotalFightForTerritoryTurns = TotalSetTerritoryTurns;
	}
	
	TMap<EColor, FTileMaterials> LInGameMaterials;
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetGameInstance());
	if (!IsValid(LGameInstance))
	{
		return;
	}
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerState* BMPlayerState = Cast<ABM_PlayerState>(PlayerState))
		{
			EColor LPlayerColor = BMPlayerState->GetPlayerColor();
			if (LPlayerColor == EColor::Undefined)
			{
				LPlayerColor = static_cast<EColor>(static_cast<uint8>(BMPlayerState->BMPlayerID + 1));
				BMPlayerState->SetPlayerColor(LPlayerColor);
			}
			FTileMaterials LTileMaterials;
			LTileMaterials.CastleMaterial = LGameInstance->CastleMaterials.FindRef(LPlayerColor);
			LTileMaterials.BannerMaterial = LGameInstance->BannerMaterials.FindRef(LPlayerColor);
			LTileMaterials.TileEdgesColor = LGameInstance->TileEdgesColors.FindRef(LPlayerColor);
			LTileMaterials.TileMeshMaterial = LGameInstance->TileMeshMaterials.FindRef(LPlayerColor);;
			LInGameMaterials.Add(LPlayerColor,LTileMaterials);
		}
	}
	for (auto Tile : Tiles)
	{
		if (ABM_TileBase* LBMTile = Cast<ABM_TileBase>(Tile))
		{
			LBMTile->SetInGameTTileMaterials(LInGameMaterials);
		}
	}
}

void ABM_GameStateBase::BeginPlay()
{
	Super::BeginPlay();
	BMGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
	
}

void ABM_GameStateBase::DisableTileEdgesHighlight()
{
	for (const auto Tile : Tiles)
	{
		ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
		BMTile->SC_SetAttackingColorForTileEdges(EColor::Undefined);
	}
}

void ABM_GameStateBase::UnbindAllOnBannerSpawnedTiles()
{
	for (const auto Tile : Tiles)
	{
		ABM_TileBase* BMTile = Cast<ABM_TileBase>(Tile);
		BMTile->OnBannerMeshSpawned.RemoveAll(this);
	}
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
	int32 NotOwnedTiles = 0;
	for (auto Tile:Tiles)
	{
		ABM_TileBase* FoundTile = Cast<ABM_TileBase>(Tile);
		if (FoundTile && FoundTile->GetStatus()==ETileStatus::NotOwned)
		{
			NotOwnedTiles++;
		}
	}
	return NotOwnedTiles;
}

//-----------------------------------END Base methods---------------------------------------------//

//--------------------------------------Player methods-------------------------------------------//

ABM_PlayerControllerBase* ABM_GameStateBase::GetPlayerController(int32 PlayerID)
{
	if (PlayerArray.IsValidIndex(PlayerID))
	{
		return Cast<ABM_PlayerControllerBase>(PlayerArray[PlayerID]->GetPlayerController());
	}
	return nullptr;
}

FLinearColor ABM_GameStateBase::GetPlayerColorByID(int32 PlayerID) const
{
	for (const auto Player : PlayerArray)
	{
		ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(Player); 
		if (IsValid(LPlayerState) && LPlayerState->BMPlayerID == PlayerID)
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

void ABM_GameStateBase::SetDefendingPlayer(int32 InID, ABM_TileBase* InTile)
{
	check(HasAuthority());
	DefendingPlayerID = InID;
	GetPlayerController(DefendingPlayerID)->SC_AssignCurrentTile(InTile);
}

void ABM_GameStateBase::RequestToClearPlayerTurnTimer()
{
	GetWorldTimerManager().ClearTimer(PlayerTurnHandle);
}

void ABM_GameStateBase::ChooseFirstAvailableTileForPlayer(int32 PlayerID)
{
	const int32 RandomTileIndex = FMath::RandRange(0, CurrentPlayerAvailableTiles.Num()-1);
	if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[PlayerID]->GetPlayerController()))
	{
		PlayerController->SC_TryClickTheTile(CurrentPlayerAvailableTiles[RandomTileIndex]);
	}
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

	if (PlayerArray.IsValidIndex(CurrentPlayerID))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]);
		if (IsValid(LCurrentPlayerState))
		{
			LCurrentPlayerState->SetPlayerTurn(true);
			UpdatePlayersTurnTimerAndNickname(CurrentPlayerID);
			if (PlayerArray.IsValidIndex(CurrentPlayerID-1))
			{
				ABM_PlayerState* LPreviousPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID-1]);
				if (IsValid(LPreviousPlayerState))
				{
					LPreviousPlayerState->SetPlayerTurn(false);
				}
			}
		}
		StartPlayerTurnTimer(CurrentPlayerID);
	}
	else
	{
		CurrentPlayerID = 0;
		WrapUpCurrentRound();
	}
}

void ABM_GameStateBase::RequestToStartPlayerTurnTimer(int32 PlayerID)
{
	StartPlayerTurnTimer(PlayerID);
}

void ABM_GameStateBase::HandleClickedTile(ABM_TileBase* InClickedTile)
{
	check(HasAuthority());
	if (!IsValid(InClickedTile))
	{
		return;
	}
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			/* Player chose their Castle tile, add it to their territory
			 and pass the turn to the nex Player */
			GetPlayerController(CurrentPlayerID)->SC_AddCurrentTileToTerritory(ETileStatus::Castle);
			MC_UpdatePoints(CurrentPlayerID,GetPlayerController(CurrentPlayerID)->GetPlayerState<ABM_PlayerState>()->GetPoints() );
			CurrentPlayerID++;
			DisableTileEdgesHighlight();
			StopPlayerTurnTimer();
			InClickedTile->OnCastleMeshSpawned.AddUniqueDynamic(this, &ThisClass::PassTurnToTheNextPlayer);
			//PassTurnToTheNextPlayer();
			break;
		case EGameRound::SetTerritory:
			/* Change Tile appearance, pass Turn to the next player, after all players choose their tile
			 the GameState will open the question */
			if (PlayerArray.IsValidIndex(CurrentPlayerID) && Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]))
			{
				InClickedTile->SC_SiegeTile(Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID])->GetPlayerColor());
				CurrentPlayerID++;
				DisableTileEdgesHighlight();
				StopPlayerTurnTimer();
				InClickedTile->OnBannerMeshSpawned.AddUniqueDynamic(this, &ThisClass::PassTurnToTheNextPlayer);
				//PassTurnToTheNextPlayer();
			}
			break;
		case EGameRound::FightForTheRestTiles:
			/* In this round Player don't manually choose the tiles,
			 * it's done automatically */
			break;
		case EGameRound::FightForTerritory:
			// TODO: need to check if the click was on the owning Tile (for fortification)
			//  in that case no question is needed
			if (PlayerArray.IsValidIndex(CurrentPlayerID) && Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID]))
			{
				InClickedTile->SC_SiegeTile(Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerID])->GetPlayerColor());
			}
			SetDefendingPlayer(InClickedTile->GetOwningPlayerID(), InClickedTile);
			DisableTileEdgesHighlight();
			StopPlayerTurnTimer();
			//InClickedTile->OnBannerMeshSpawned.AddUniqueDynamic(this, &ThisClass::OpenChooseQuestion);
			CurrentSiegeTileQuestionCount = TotalSiegeTileQuestionCount = InClickedTile->GetTileQuestionCount();
			InClickedTile->OnBannerMeshSpawned.AddUniqueDynamic(this, &ThisClass::StartSiege);
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
			PlayerController->CC_OpenQuestionWidget(LastQuestion, AnsweringPlayers, LNewViewTarget);
		}
	}
}

void ABM_GameStateBase::StartPlayerTurnTimer(int32 PlayerID)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	CurrentPlayerAvailableTiles.Empty();
	
	CurrentTurnTimer = BMGameMode->GetTurnTimer()+1; // to be able to see timer widget fades away
	//ResetPlayersTurns(PlayerID);
	
	DisableTileEdgesHighlight();
	HighlightAvailableTiles(PlayerID);
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
	GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameStateBase::ForceChooseAvailableTile, LTurnSeconds, false);
}

void ABM_GameStateBase::HighlightAvailableTiles(int32 PlayerID)
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
						BMTile->SC_SetAttackingColorForTileEdges(PlayerState->GetPlayerColor());
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
					NeighborTile->SC_SetAttackingColorForTileEdges(CurrentPlayerState->GetPlayerColor());
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
						BMTile->SC_SetAttackingColorForTileEdges(CurrentPlayerState->GetPlayerColor());
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
					NeighborTile->SC_SetAttackingColorForTileEdges(CurrentPlayerState->GetPlayerColor());
					CurrentPlayerAvailableTiles.Add(NeighborTile);
				}
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameStateBase::ForceChooseAvailableTile()
{
	ChooseFirstAvailableTileForPlayer(CurrentPlayerID);
}

void ABM_GameStateBase::StartSiege()
{
	if (!IsValid(BMGameMode))
	{
		return;
	}
	ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerID]->GetOwningController());
	if (!IsValid(LCurrentPlayerController))
	{
		return;
	}
	ABM_TileBase* LCurrentClickedTile = LCurrentPlayerController->GetCurrentClickedTile();
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
		ABM_PlayerControllerBase* AttackingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[CurrentPlayerID]->GetPlayerController());
		ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(AttackingPlayer->PlayerState);
		ABM_PlayerControllerBase* DefendingPlayer = Cast<ABM_PlayerControllerBase>(PlayerArray[DefendingPlayerID]->GetPlayerController());
		ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(DefendingPlayer->PlayerState);
		for (const auto LTile: DefendingPlayerState->OwnedTiles)
		{
			AttackingPlayerState->SC_AddTileToTerritory(LTile, ETileStatus::Controlled);
		}
		if (BMGameMode->GetNumberOfActivePlayers() == 2)
		{
			Round = GetNextGameRound();
			FTimerHandle LCountResultsHandle;
			GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
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
			AnsweringPlayers.Add(Cast<ABM_PlayerState>(PlayerState)->BMPlayerID);
		}
	}
	else
	{
		AnsweringPlayers.Add(CurrentPlayerID);
		AnsweringPlayers.Add(DefendingPlayerID);
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
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().PlayerID = PlayerState->BMPlayerID;
			LAutoChoice.GetMutable<FPlayerChoiceChoose>().AnswerID = -1;
			PlayerState->QuestionChoices.Add(LAutoChoice);
			break;
		}
		case EQuestionType::Shot:
		{
			// Generate auto choice Shot
			FInstancedStruct LAutoChoice = FInstancedStruct::Make(FPlayerChoiceShot());
			LAutoChoice.GetMutable<FPlayerChoiceShot>().PlayerID = PlayerState->BMPlayerID;
			LAutoChoice.GetMutable<FPlayerChoiceShot>().Answer = -1;
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
	CurrentSiegeTileQuestionCount--;
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
				ABM_PlayerState* CurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerID]);
				ABM_PlayerControllerBase* CurrentPlayerController = Cast<ABM_PlayerControllerBase>(CurrentPlayerState->GetPlayerController());
				if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
				{
					//Set Tile color back to default
					ConstructQuestionResult(CurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
					CurrentPlayerController->SC_CancelAttackForCurrentTile();
				}
				else
				{
					ConstructQuestionResult(CurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, CurrentPlayerController->GetPointsOfCurrentClickedTile(), true);
					CurrentPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
					MC_UpdatePoints(LPlayerID,CurrentPlayerController->GetPlayerState<ABM_PlayerState>()->GetPoints() );
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
				AttackingPlayer->SC_CancelAttackForCurrentTile();
			}
			else // Someone got the right answer
			{
				for (const auto AnsweredQuestion : PlayersCurrentChoices)
				{
					if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
					{
						// correct answer
						if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->PlayerID == CurrentPlayerID)
						{
							// attacking player was right
							if (CurrentSiegeTileQuestionCount == 0)
							{
								DefendingPlayer->SC_RemoveCurrentTileFromTerritory();
								MC_UpdatePoints(DefendingPlayerID,DefendingPlayerState->GetPoints());
								AttackingPlayer->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
								MC_UpdatePoints(CurrentPlayerID,AttackingPlayerState->GetPoints());
							}
							else
							{
								AttackingPlayer->SC_ApplyDamageToClickedTile(1);
							}
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1 * DefendingPlayer->GetPointsOfCurrentClickedTile(), false);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, DefendingPlayer->GetPointsOfCurrentClickedTile(), true);
						}
						else 
						{
							// defending player was right
							ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
							ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
							AttackingPlayer->SC_CancelAttackForCurrentTile();
							DefendingPlayerState->SC_AddPoints(100);
							MC_UpdatePoints(DefendingPlayerID,DefendingPlayerState->GetPoints());
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
	if (ShotChoices[0].Answer < 0 && ShotChoices.Last().Answer < 0)
	{
		// No players have sent their answers
		for (const auto LShotChoice: ShotChoices)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[LShotChoice.PlayerID]->GetPlayerController());
			if (IsValid(LoserPlayerController) && LoserPlayerController->HasValidCurrentClickedTile())
			{
				LoserPlayerController->SC_CancelAttackForCurrentTile();
				ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1*LoserPlayerController->GetPointsOfCurrentClickedTile(), false);
			}
		}
		return;
	}
	const ABM_PlayerControllerBase* WinnerPlayerController = nullptr;
	ABM_PlayerState* WinnerPlayerState = nullptr;

	for (const auto LShotChoice: ShotChoices)
	{
		if (LShotChoice.Answer > 0)
		{
			for (FInstancedStruct& LPlayerChoice : PlayersCurrentChoices)
			{
				if (LPlayerChoice.Get<FPlayerChoiceShot>().PlayerID == LShotChoice.PlayerID)
				{
					LPlayerChoice.GetMutable<FPlayerChoiceShot>().bAnswered = true;
				}
			}
			WinnerPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[0].PlayerID]->GetPlayerController());
			WinnerPlayerState = WinnerPlayerController->GetPlayerState<ABM_PlayerState>();
			break;
		}
	}

	if (Round == EGameRound::FightForTheRestTiles)
	{
		for (int i =1; i < ShotChoices.Num(); i++)
		{
			const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[i].PlayerID]->GetPlayerController());
			if (LoserPlayerController && LoserPlayerController->HasValidCurrentClickedTile())
			{
				LoserPlayerController->SC_CancelAttackForCurrentTile();
				ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			}
		}
		if(IsValid(WinnerPlayerController) && IsValid(WinnerPlayerState) && WinnerPlayerController->HasValidCurrentClickedTile())
		{
			WinnerPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
			MC_UpdatePoints(WinnerPlayerState->BMPlayerID,WinnerPlayerState->GetPoints());	
			ConstructQuestionResult(WinnerPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, WinnerPlayerController->GetPointsOfCurrentClickedTile(), true);
		}
	}
	if (Round == EGameRound::FightForTerritory)
	{
		const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[ShotChoices[DefendingPlayerID].PlayerID]->GetPlayerController());
		ABM_PlayerState* LoserPlayerState = nullptr;
		if (LoserPlayerController && LoserPlayerController->HasValidCurrentClickedTile())
		{
			LoserPlayerState = LoserPlayerController->GetPlayerState<ABM_PlayerState>();
			LoserPlayerController->SC_RemoveCurrentTileFromTerritory();
			if (IsValid(LoserPlayerState))
			{
				MC_UpdatePoints(LoserPlayerState->BMPlayerID,LoserPlayerState->GetPoints());	
			}
			ConstructQuestionResult(LoserPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1* LoserPlayerController->GetPointsOfCurrentClickedTile(), false);
		}
		if(WinnerPlayerController && WinnerPlayerController->HasValidCurrentClickedTile())
		{
			if (CurrentSiegeTileQuestionCount == 0)
			{
				WinnerPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
				MC_UpdatePoints(WinnerPlayerState->BMPlayerID,WinnerPlayerState->GetPoints());	
			}
			else
			{
				WinnerPlayerController->SC_ApplyDamageToClickedTile(1);
			}
			ConstructQuestionResult(WinnerPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, WinnerPlayerController->GetPointsOfCurrentClickedTile(), true);
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
				ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1 * AttackingPlayerController->GetPointsOfCurrentClickedTile(), false);
				ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, AttackingPlayerController->GetPointsOfCurrentClickedTile(), true);
				if (CurrentSiegeTileQuestionCount == 0)
				{
					DefendingPlayerController->SC_RemoveCurrentTileFromTerritory();
					AttackingPlayerController->SC_AddCurrentTileToTerritory(ETileStatus::Controlled);
				}
			}
			else
			{
				// defending player was right
				ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
				ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
				AttackingPlayerController->SC_CancelAttackForCurrentTile();
				DefendingPlayerState->SC_AddPoints(100);
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

void ABM_GameStateBase::WrapUpCurrentRound()
{
	RequestToClearPlayerTurnTimer();
	FTimerDelegate LNextRound;
	switch (Round)
	{
		case EGameRound::ChooseCastle:
			//All players set their castles, continue to SetTerritory round with 2 sec delay
			Round = GetNextGameRound();
			LNextRound.BindUObject(this, &ThisClass::PassTurnToTheNextPlayer);
			DisableTileEdgesHighlight();
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRound, 2.0, false);
			break;
		case EGameRound::SetTerritory:
			//All players have chosen their tiles, open Choose Question with 2sec delay
			DisableTileEdgesHighlight();
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Choose);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
			break;
		case EGameRound::FightForTheRestTiles:
			//Start Shot question for the rest tiles
			//Find first not owned tile
			UpdatePlayersTurnTimerAndNickname(0);
			for (auto Tile: Tiles)
			{
				auto LFoundTile = Cast<ABM_TileBase>(Tile);
				if (LFoundTile->GetStatus() == ETileStatus::NotOwned)
				{
					for (const auto PlayerState : PlayerArray)
					{
						if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
						{
							PlayerController->SC_AssignCurrentTile(LFoundTile);
						}
					}
					break;
				}
			}
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Shot);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
			break;
		case EGameRound::FightForTerritory:
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
	//If GameRound == SetTerritory,
	// check if there are available tiles and their amount == NumberOfActivePlayers%
	// if yes, Continue SetTerritory round
	// if no, Start Battle Mode for the rest of the tiles
	// If GameRound == FightForTerritory, check how many Player Turns are left
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
			TotalSetTerritoryTurns--;
			int32 NotOwnedTiles = CountNotOwnedTiles();	
			if (NotOwnedTiles > 0)
			{
				if(NotOwnedTiles >= BMGameMode->GetNumberOfActivePlayers())
				{
					if (TotalSetTerritoryTurns <= 0)
					{
						Round = EGameRound::FightForTheRestTiles;
						UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round due to Max SetTerritory turns reached"));
						WrapUpCurrentRound();
						break;
					}
					PassTurnToTheNextPlayer();
				}
				else
				{
					Round = EGameRound::FightForTheRestTiles;
					WrapUpCurrentRound();
					UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round"));
				}
			}
			else
			{
				Round = EGameRound::FightForTerritory;
				PassTurnToTheNextPlayer();
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Territory round"));
			}
			break;
		}
		case EGameRound::FightForTheRestTiles:
		{
			int32 NotOwnedTiles = CountNotOwnedTiles();
			if (NotOwnedTiles > 0)
			{
				WrapUpCurrentRound();
				UE_LOG(LogBM_GameMode, Display, TEXT("Continue Fight for Last Tiles round"));
			}
			else
			{
				Round = EGameRound::FightForTerritory;
				PassTurnToTheNextPlayer();
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
			 *		check how many turns left and decrease that counter for Attacking Player, continue FightForTerritory round
			 *		else
			 *		switch to the CountResults round
			 *	else Continue the siege
			 */
			if (bShotQuestionIsNeeded)
			{
				OpenQuestion(EQuestionType::Shot);
				break;
			}
			TotalFightForTerritoryTurns--;
			if (CurrentSiegeTileQuestionCount == 0)
			{
				if (BMGameMode->GetNumberOfActivePlayers() < 2 || TotalFightForTerritoryTurns <= 0)
				{
					Round = GetNextGameRound();
					FTimerHandle LCountResultsHandle;
					GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
				}
				else
				{
					CurrentPlayerID++;
					PassTurnToTheNextPlayer();
				}
			}
			else
			{
				StartSiege();
			}
			break;
		}
		default:
			break;
	}
}

void ABM_GameStateBase::UpdatePlayersTurnTimerAndNickname(int32 PlayerID)
{
	if (!PlayerArray.IsValidIndex(PlayerID))
	{
		return;
	}
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdateCurrentPlayerNickname(PlayerID);
			Cast<ABM_PlayerState>(PlayerState)->CurrentQuestionAnswerSent = false;
			PlayerController->ResetTurnTimer(Round);
		}
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

//----------------------------------END Game flow methods----------------------------------------//