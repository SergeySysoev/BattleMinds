// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameStateBase.h"
#include "Camera/CameraActor.h"
#include "BM_GameInstance.h"
#include "BM_GameModeBase.h"
#include "GameRounds/BuildCastleRound.h"
#include "GameRounds/GameRound.h"
#include "GameRounds/SetTerritoryRound.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerState.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Tiles/BM_TileManager.h"

DEFINE_LOG_CATEGORY(LogBM_GameStateBase);

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
			case EGameRound::BuildCastle:
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
			case EGameRound::FightForTheRemains:
			{
				MaxCyclesPerRound.Add(static_cast<EGameRound>(i), 2);  // That's not cycles number but amount of turns
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
	SetNextGameRound(EGameRound::BuildCastle);
	RemainingPlayers = BMGameMode->GetNumberOfActivePlayers();
	for (int i = 0; i < PlayerArray.Num(); i++)
	{
		RemainingPlayerIndices.Add(i);
	}
	TSubclassOf<ABM_TileManager> LTileManagerClass = ABM_TileManager::StaticClass();
	TileManager = Cast<ABM_TileManager>(UGameplayStatics::GetActorOfClass(GetWorld(), LTileManagerClass));
	TileManager->OnMapGeneratedNative.AddUObject(this, &ABM_GameStateBase::CalculateAndSetMaxCyclesPerRound);
	OpenNextQuestionPtr = &ABM_GameStateBase::OpenNextQuestion;
	StartSiegePtr = &ABM_GameStateBase::StartSiege;
	PassTurnToNextPlayerPtr = &ABM_GameStateBase::PassTurnToTheNextPlayer;
}

void ABM_GameStateBase::StartPostCastleChosenPhase()
{
	ABM_PlayerControllerBase* LCurrentPlayerController = GetPlayerController(CurrentPlayerIndex);
	FUniversalCameraPositionSaveFormat LCastleCameraProperties;
	FUniversalCameraPositionSaveFormat LPlayerTurnCameraProperties;
	TileManager->GetClickedTileCastleCameraPropertiesByPlayerId(CurrentPlayerIndex, LCastleCameraProperties);
	TileManager->GetClickedTilePlayerTurnCameraPropertiesByPlayerId(CurrentPlayerIndex, LPlayerTurnCameraProperties);
	LCurrentPlayerController->SC_SetCameraDefaultProperties(LCastleCameraProperties);
	LCurrentPlayerController->SC_SetCameraPlayerTurnProperties(LPlayerTurnCameraProperties);
	StartPostQuestionPhase(false);
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
		case EGameRound::BuildCastle:
		{
			FPermutation LPermutation;
			for (int32 i = 0; i < PlayerArray. Num(); i++)
			{
				LPermutation.Values.Add(i);
			}
			PlayerTurnsCycles.Add(FPlayersCycle(0, LPermutation, false));
		}
			break;
		case EGameRound::FightForTheRemains:
		{
			// Construct 1 cycle of N elements
			FPermutation LPermutation;
			for (int32 i = 0; i < MaxCyclesPerRound.FindRef(EGameRound::FightForTheRemains); i++)
			{
				LPermutation.Values.Add(-1); // All turns will be blacked out until the questions is answsered
			}
			PlayerTurnsCycles.Add(FPlayersCycle(0, LPermutation, false));
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
	UpdatePlayersCyclesWidget();
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

int32 ABM_GameStateBase::GetNextPlayerArrayIndex()
{
	++CurrentPlayerCounter;
	if (!PlayerTurnsCycles.IsEmpty())
	{
		if (PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
		{
			if (PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter))
			{
				return PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values[CurrentPlayerCounter];
			}
			return -1;
		}
		return -1;
	}
	return -1;
}

int32 ABM_GameStateBase::GetPreviousPlayerArrayIndex()
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

void ABM_GameStateBase::TogglePlayerTurnTimer(bool ShouldPause)
{
	if (ShouldPause)
	{
		GetWorld()->GetTimerManager().PauseTimer(PlayerTurnHandle);
	}
	else
	{
		GetWorld()->GetTimerManager().UnPauseTimer(PlayerTurnHandle);
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

void ABM_GameStateBase::OpenNextQuestion()
{
	ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
	if (IsValid(LCurrentPlayerState) && IsValid(LCurrentPlayerController))
	{
		LCurrentPlayerController->CC_SetInputEnabled(false);
	}
	if (IsValid(TileManager))
	{
		OpenQuestion(TileManager->GetNextQuestionTypeOfClickedTile());
	}
	else
	{
		OpenQuestion(EQuestionType::Choose);
	}
}

int32 ABM_GameStateBase::GetNotOwnedTilesCount()
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

void ABM_GameStateBase::SetNextPlayerIndex()
{
	check(HasAuthority());
	CurrentPlayerIndex = GetNextPlayerArrayIndex();
}

void ABM_GameStateBase::SetCurrentPlayerCounter(int32 NewPlayerCounter)
{
	check(HasAuthority());
	CurrentPlayerCounter = NewPlayerCounter;
}

void ABM_GameStateBase::SetCurrentPlayerCycles(TArray<FPlayersCycle>& PlayersCycles)
{
	CurrentPlayerTurnsCycle = 0;
	PlayerTurnsCycles.Empty();
	PlayerTurnsCycles = PlayersCycles;
}

void ABM_GameStateBase::IncrementCurrentPlayerCycle()
{
	PlayerTurnsCycles[CurrentPlayerTurnsCycle].bIsCompleted = true;
	++CurrentPlayerTurnsCycle;
}

bool ABM_GameStateBase::CheckCurrentCycleCounter() const
{
	return IsValidCurrentCycleCounter(CurrentPlayerTurnsCycle);
}

bool ABM_GameStateBase::IsPlayerTurn(int32 PlayerIndex) const
{
	if (PlayerArray.IsValidIndex(PlayerIndex))
	{
		return Cast<ABM_PlayerState>(PlayerArray[PlayerIndex])->IsPlayerTurn();
	}
	return false;
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

FLinearColor ABM_GameStateBase::GetPlayerLinearColorByIndex(int32 PlayerIndex) const
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

EColor ABM_GameStateBase::GetPlayerColorByIndex(int32 PlayerIndex) const
{
	for (const auto Player : PlayerArray)
	{
		ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(Player); 
		if (IsValid(LPlayerState) && LPlayerState->GetPlayerIndex() == PlayerIndex)
		{
			return LPlayerState->GetPlayerColor();
		}
	}
	return EColor::Undefined;
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

void ABM_GameStateBase::EliminatePlayer(int32 PlayerIndex)
{
	RemainingPlayers--;
	RemainingPlayerIndices.RemoveSwap(PlayerIndex);
}

void ABM_GameStateBase::SC_ChangePointsOfPlayer_Implementation(int32 PlayerIndex, int32 PointsIncrement)
{
	if (PlayerArray.IsValidIndex(PlayerIndex))
	{
		ABM_PlayerState* LPlayerStateToUpdate = Cast<ABM_PlayerState>(PlayerArray[PlayerIndex]);
		if (IsValid(LPlayerStateToUpdate))
		{
			LPlayerStateToUpdate->SC_ChangePoints(PointsIncrement);
			MC_UpdatePoints(PlayerIndex, LPlayerStateToUpdate->GetPoints());
		}
	}
}

void ABM_GameStateBase::ChooseFirstAvailableTileForPlayer(int32 PlayerIndex)
{
	/*const auto LAvailableTiles = GetPlayerAvailableTiles(Round, PlayerIndex);
	const int32 RandomTileIndex = FMath::RandRange(0, LAvailableTiles.Num()-1);
	TileManager->HandleClickedTile(PlayerIndex, LAvailableTiles[RandomTileIndex]);
	HandleClickedTile(LAvailableTiles[RandomTileIndex]);*/
	TileManager->HandleClickedTile(PlayerIndex, TileManager->GetFirstAvailableTileAxials());
	HandleClickedTile(TileManager->GetFirstAvailableTileAxials());
	
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
	if(Round == EGameRound::BuildCastle)
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
		case EGameRound::BuildCastle:
			break;
		case EGameRound::SetTerritory:
			break;
		default: break;
	}
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdateRoundWidget(NewRound);
		}
	}
	if (IsValid(TileManager))
	{
		TileManager->SwitchToNextRound(NewRound);
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
	for (const auto LPlayerState : PlayerArray)
	{
		ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
		ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
		if (IsValid(LPlayerController))
		{
			LPlayerController->CC_SetInputEnabled(false);
			LBMPlayerState->SetPlayerTurn(false);
		}
	}
	CurrentPlayerIndex = GetNextPlayerArrayIndex();
	if (PlayerArray.IsValidIndex(CurrentPlayerIndex) && RemainingPlayerIndices.Contains(CurrentPlayerIndex))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
		if (IsValid(LCurrentPlayerState) && IsValid(LCurrentPlayerController))
		{
			TileManager->SC_ResetFirstAvailableTile();
			LCurrentPlayerState->SetPlayerTurn(true);
			UpdatePlayerTurn();
		}
		//StartPlayerTurnTimer(CurrentPlayerIndex);
		StartPrePlayerTurnPhase();
	}
	else
	{
		CurrentPlayerCounter = -1;
		WrapUpCurrentPlayersCycle();
	}
}

void ABM_GameStateBase::PassTurnToTheShotQuestionWinner()
{
	check(HasAuthority());
	ABM_PlayerState* LWinnerPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
	for (const auto LPlayerState : PlayerArray)
	{
		ABM_PlayerState* LBMPlayerState = Cast<ABM_PlayerState>(LPlayerState);
		ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
		if (IsValid(LPlayerController))
		{
			LPlayerController->CC_SetInputEnabled(false);
			LBMPlayerState->SetPlayerTurn(false);
			LPlayerController->UpdateCurrentPlayerTurnSlot(CurrentPlayerCounter,
				LWinnerPlayerState->GetUniqueId(), LWinnerPlayerState->GetPlayerColor());
		}
	}
	if (PlayerArray.IsValidIndex(CurrentPlayerIndex) && RemainingPlayerIndices.Contains(CurrentPlayerIndex))
	{
		ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
		if (IsValid(LCurrentPlayerState) && IsValid(LCurrentPlayerController))
		{
			LCurrentPlayerState->SetPlayerTurn(true);
			LCurrentPlayerController->CC_SetInputEnabled(true);
			UpdatePlayerTurn();
		}
		StartPlayerTurnTimer();
	}
}

void ABM_GameStateBase::HandleClickedTile(FIntPoint InClickedTile)
{
	check(HasAuthority());
	ABM_PlayerControllerBase* LCurrentPlayerController = GetPlayerController(CurrentPlayerIndex);
	ABM_PlayerState* LCurrentPlayerState = LCurrentPlayerController->GetPlayerState<ABM_PlayerState>();
	LCurrentPlayerController->CC_SetInputEnabled(false);
	LCurrentPlayerState->SetPlayerTurn(false);
	TileManager->HandleClickedTile(CurrentPlayerIndex, InClickedTile);
	DisableTileEdgesHighlight();
	StopPlayerTurnTimer();
	switch (Round)
	{
		case EGameRound::BuildCastle:
			/* Player chose their Castle tile
			 * so add it to their territory
			 and pass the turn to the nex Player when Castle mesh is spawned*/
			if (CurrentGameRoundObject)
			{
				CurrentGameRoundObject->HandleClickedTile(InClickedTile, LCurrentPlayerState);
			}
			break;
		case EGameRound::SetTerritory:
			/* Spawn Attacking Banner Mesh on Clicked Tile, pass Turn to the next player, after all players choose their tile
			 the GameState will open the question */
			if (CurrentGameRoundObject)
			{
				CurrentGameRoundObject->HandleClickedTile(InClickedTile, LCurrentPlayerState);
			}
			break;
		case EGameRound::FightForTheRemains:
			{
				LCurrentPlayerState->SC_ChangePoints(BMGameMode->GetPointsOfTile(EGameRound::FightForTheRemains));
				TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerIndex, ETileStatus::Controlled, LCurrentPlayerState->GetPlayerColor(), Round);
				MC_UpdatePoints(CurrentPlayerIndex, LCurrentPlayerState->GetPoints());
				LCurrentPlayerState->SetPlayerTurn(false);
			}
			break;
		case EGameRound::FightForTerritory:
			// TODO: need to check if the click was on the owning Tile (for fortification)
			//  in that case no question is needed
			{
				LCurrentPlayerState->SetPlayerTurn(false);
				TileManager->SC_AttackTile(InClickedTile, LCurrentPlayerState->GetPlayerColor());
				CurrentSiegeTileQuestionCount = TileManager->GetTileQuestionsCount(InClickedTile);
				SetDefendingPlayer(InClickedTile);
				TileManager->BindGameStateToTileBannerMeshSpawned(InClickedTile, OpenNextQuestionPtr);
			}
			break;
		default: break;
	}
	StopHUDAnimations();
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
			PlayerController->CC_OpenQuestionWidget(LastQuestion, AnsweringPlayers, LNewViewTarget, BMGameMode->GetQuestionTimer());
		}
	}
}

void ABM_GameStateBase::StartPlayerTurnTimer()
{
	check(HasAuthority());
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	CurrentTurnTimer = BMGameMode->GetTurnTimer()+1; // to be able to see timer widget fades away
	HighlightAvailableTiles(CurrentPlayerIndex);
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
		}
	}
	ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
	if (IsValid(LCurrentPlayerController))
	{
		LCurrentPlayerController->CC_SetInputEnabled(true);
		LCurrentPlayerController->CC_CheckForTileUnderCursor(TileManager->GetFirstAvailableTileAxials());
	}
	
	float LTurnSeconds = BMGameMode->GetTurnTimer();
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->StartCountdownTimer(LTurnSeconds);
		}
	}
	
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
	ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerControllerBase* LCurrentPlayerController = Cast<ABM_PlayerControllerBase>(LCurrentPlayerState->GetPlayerController());
	if (IsValid(LCurrentPlayerState) && IsValid(LCurrentPlayerController))
	{
		LCurrentPlayerController->CC_SetInputEnabled(false);
	}
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

void ABM_GameStateBase::StartPrePlayerTurnPhase()
{
	CurrentPrePlayerTurnReadyActors = 0;
	ExpectedPrePlayerTurnReadyActors = OnPrePlayerTurnPhaseStarted.GetAllObjects().Num();
	UE_LOG(LogTemp, Log, TEXT("PrePlayerTurn Phase started. Expecting %d listeners."), ExpectedPrePlayerTurnReadyActors);
	
	FPrePlayerTurnPhaseInfo LPlayerTurnPhaseInfo = FPrePlayerTurnPhaseInfo(Round);
	OnPrePlayerTurnPhaseStarted.Broadcast(LPlayerTurnPhaseInfo);
	if (ExpectedPrePlayerTurnReadyActors == 0)
	{
		CheckPrePlayerTurnPhaseCompleted();
	}
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
		case EGameRound::FightForTheRemains:
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
		AnsweringPlayers.Add(CurrentPlayerIndex);
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
	PlayersToUpdatePoints.Empty();
	//Verify Players answers:
	QuestionResults.Empty();
	if (LastQuestion.GetPtr<FQuestion>())
	{
		switch (LastQuestion.GetPtr<FQuestion>()->Type)
		{
			case EQuestionType::Choose:
			{
				QuestionResults = VerifyChooseAnswers();
				break;
			}
			case EQuestionType::Shot:
			{
				QuestionResults = VerifyShotAnswers();
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

TMap<int32, EQuestionResult> ABM_GameStateBase::VerifyChooseAnswers()
{
	/*const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	TMap<int32, EQuestionResult> LQuestionResults;
	switch (Round)
	{
		case EGameRound::SetTerritory:
		{
			for (const auto LPlayerChoice : PlayersCurrentChoices)
			{
				const auto LPlayerID =  LPlayerChoice.GetPtr<FPlayerChoice>()->PlayerID;
				ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerID]);
				if (LPlayerChoice.GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
				{
					// Wrong answer was given
					ConstructQuestionResult(LCurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
					//TileManager->SC_CancelAttackOnClickedTile(LPlayerID);
					LQuestionResults.Add(LPlayerID, EQuestionResult::TileDefended);
				}
				else
				{
					// Correct answer was given
					int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
					ConstructQuestionResult(LCurrentPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
					LCurrentPlayerState->SC_ChangePoints(LPoints);
					//TileManager->SC_AddClickedTileToTheTerritory(LPlayerID, ETileStatus::Controlled, LCurrentPlayerState->GetPlayerColor(), Round);
					//MC_UpdatePoints(LPlayerID,LCurrentPlayerState->GetPoints());
					PlayersToUpdatePoints.Add(LPlayerID);
					LQuestionResults.Add(LPlayerID, EQuestionResult::TileCaptured);
				}
			}
			break;
		}
		case EGameRound::FightForTerritory:
		{
			if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer
				&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
			{
				// if both Players answered their Choose Question, we need a Shot round
				bShotQuestionIsNeeded = true;
				LQuestionResults.Add(PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::ShotQuestionNeeded);
				LQuestionResults.Add(PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::ShotQuestionNeeded);
				return LQuestionResults;
			}
			if (PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer
				&& PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->AnswerID != LRightAnswer)
			{
				CurrentSiegeTileQuestionCount = 0;
				// both Players gave wrong answer, cancel attack for Attacking, and don't give points to Defending
				HandleSiegedTile(0);
				LQuestionResults.Add(PlayersCurrentChoices[0].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::TileDefended);
				LQuestionResults.Add(PlayersCurrentChoices[1].GetPtr<FPlayerChoiceChoose>()->PlayerID, EQuestionResult::TileDefended);
				return LQuestionResults;
			}
			// Someone got the right answer
			CurrentSiegeTileQuestionCount--;
			for (const auto AnsweredQuestion : PlayersCurrentChoices)
			{
				if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->AnswerID == LRightAnswer)
				{
					// correct answer
					if(AnsweredQuestion.GetPtr<FPlayerChoiceChoose>()->PlayerID == CurrentPlayerIndex)
					{
						// attacking player was right
						HandleSiegedTile(1);
						if (CurrentSiegeTileQuestionCount == 0)
						{
							LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileCaptured);
							LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileCaptured);
						}
						else
						{
							LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDamaged);
							LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDamaged);
						}
					}
					else 
					{
						// defending player was right
						CurrentSiegeTileQuestionCount = 0; // stop Siege
						HandleSiegedTile(2);
						LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDefended);
						LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDefended);
					}
					break;
				}
			}
			break;
		}
		default:
			break;
	}
	return LQuestionResults;*/
	const auto LRightAnswer = LastQuestion.GetPtr<FQuestionChooseText>()->RightAnswer;
	TMap<int32, EQuestionResult> LQuestionResults = CurrentGameRoundObject->VerifyChooseAnswers(
		LastQuestion, PlayersCurrentChoices, UsedQuestions.Num());
	return LQuestionResults;
}

UE_DISABLE_OPTIMIZATION
TMap<int32, EQuestionResult> ABM_GameStateBase::VerifyShotAnswers()
{
	TArray<FPlayerChoiceShot> ShotChoices;
	TMap<int32, EQuestionResult> LQuestionResults;
	TMap<FPlayerChoiceShot, int32> LSortedToOriginalIndices;
	for (int32 i = 0; i < PlayersCurrentChoices.Num(); i++)
	{
		ShotChoices.Add(PlayersCurrentChoices[i].Get<FPlayerChoiceShot>());
		LSortedToOriginalIndices.Add(PlayersCurrentChoices[i].Get<FPlayerChoiceShot>(), i);
	}
	// Sort Answers by Difference and ElapsedTime (overriden < operator in FPlayerChoiceShot)
	if (ShotChoices.Num() > 1)
	{
		ShotChoices.Sort();
	}
	CurrentSiegeTileQuestionCount--;
	if (ShotChoices[0].Answer == MAX_int32)   // No players have sent their answers
	{
		if (Round == EGameRound::FightForTheRemains)
		{
			for (const auto LShotChoice: ShotChoices)
			{
				const ABM_PlayerControllerBase* LoserPlayerController = Cast<ABM_PlayerControllerBase>(PlayerArray[LShotChoice.PlayerID]->GetPlayerController());
				if (IsValid(LoserPlayerController))
				{
					//TileManager->SC_CancelAttackOnClickedTile(LShotChoice.PlayerID);
					ConstructQuestionResult(LoserPlayerController->GetPlayerState<ABM_PlayerState>(), UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
					LQuestionResults.Add(LShotChoice.PlayerID, EQuestionResult::TileDefended);
				}
			}
			return LQuestionResults;
		}
		if (Round == EGameRound::FightForTerritory)
		{
			// both players haven't sent their answers, cancel Attack and add 100 points to the Defender
			HandleSiegedTile(0);
			LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDefended);
			LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDefended);
		}
	}
	FightForTheRestTileWinnerIndex = ShotChoices[0].PlayerID;
	ABM_PlayerState* WinnerPlayerState = Cast<ABM_PlayerState>(PlayerArray[ShotChoices[0].PlayerID]);
	if (Round == EGameRound::FightForTheRemains)
	{
		for (int i = 1; i < ShotChoices.Num(); i++)
		{
			ABM_PlayerState* LLoserPlayerState = Cast<ABM_PlayerState>(PlayerArray[ShotChoices[i].PlayerID]);
			ConstructQuestionResult(LLoserPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			LQuestionResults.Add(ShotChoices[i].PlayerID, EQuestionResult::TileDefended);
		}
		if(IsValid(WinnerPlayerState))
		{
			int32 LOriginalIndex = LSortedToOriginalIndices.FindRef(ShotChoices[0]);
			if (PlayersCurrentChoices.IsValidIndex(LOriginalIndex))
			{
				PlayersCurrentChoices[LOriginalIndex].GetMutable<FPlayerChoiceShot>().bAnswered = true;
			}
			LQuestionResults.Add(ShotChoices[0].PlayerID, EQuestionResult::TileCaptured);
			//TileManager->SC_AddClickedTileToTheTerritory(ShotChoices[0].PlayerID, ETileStatus::Controlled, WinnerPlayerState->GetPlayerColor(), Round);
			//MC_UpdatePoints(WinnerPlayerState->GetPlayerIndex(),WinnerPlayerState->GetPoints());
			PlayersToUpdatePoints.Add(WinnerPlayerState->GetPlayerIndex());
			ConstructQuestionResult(WinnerPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, BMGameMode->GetPointsOfTile(EGameRound::FightForTheRemains), true);
		}
	}
	if (Round == EGameRound::FightForTerritory)
	{
		ABM_PlayerState* LAttackingPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
		ABM_PlayerState* LDefendingPlayerState = Cast<ABM_PlayerState>(PlayerArray[DefendingPlayerIndex]);
		int32 LWinnerIndex = ShotChoices[0].PlayerID;
		int32 LOriginalIndex = LSortedToOriginalIndices.FindRef(ShotChoices[0]);
		if (PlayersCurrentChoices.IsValidIndex(LOriginalIndex))
		{
			PlayersCurrentChoices[LOriginalIndex].GetMutable<FPlayerChoiceShot>().bAnswered = true;
		}
		if (IsValid(LAttackingPlayerState) && IsValid(LDefendingPlayerState))
		{
			if (CurrentPlayerIndex == LWinnerIndex)		// Attacking Player was right
			{
				HandleSiegedTile(1);
				if (CurrentSiegeTileQuestionCount == 0)
				{
					LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileCaptured);
					LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileCaptured);
				}
				else
				{
					LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDamaged);
					LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDamaged);
				}
			}
			else // Defending Player was right
			{
				HandleSiegedTile(2);
				LQuestionResults.Add(CurrentPlayerIndex, EQuestionResult::TileDefended);
				LQuestionResults.Add(DefendingPlayerIndex, EQuestionResult::TileDefended);
			}
		}
	}
	return LQuestionResults;
}

void ABM_GameStateBase::PrepareNextRound(EGameRound NextRound)
{
	StopAllTimers();
	SetNextGameRound(NextRound);
	FTimerDelegate LNextRoundDelegate;
	LNextRoundDelegate.BindLambda([this, NextRound]() {
		switch (NextRound)
		{
			case EGameRound::BuildCastle:
			{
				SwitchGameRound<UBuildCastleRound>();
			}
			break;
			case EGameRound::SetTerritory:
			{
				SwitchGameRound<USetTerritoryRound>();
			}
			break;
			default:
				break;
		}
	});
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRoundDelegate, 3.0f, false);
	
}

UE_ENABLE_OPTIMIZATION

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
	FTimerDelegate LPostQuestionDelegate;
	LPostQuestionDelegate.BindUObject(this, &ThisClass::StartPostQuestionPhase, bShotQuestionIsNeeded);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, LPostQuestionDelegate, 3.f, false);	
}

/*
 * AnsweredPlayer = 0 - no one gave the right answer
 * AnsweredPlayer = 1 - Attacker was right
 * AnsweredPlayer = 2 - Defender was right
 */
void ABM_GameStateBase::HandleSiegedTile(uint8 AnsweredPlayer)
{
	ABM_PlayerState* AttackingPlayerState = Cast<ABM_PlayerState>(PlayerArray[CurrentPlayerIndex]);
	ABM_PlayerState* DefendingPlayerState = Cast<ABM_PlayerState>(PlayerArray[DefendingPlayerIndex]);
	int32 LPoints;
	switch (AnsweredPlayer)
	{
		case 0:
			ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			break;
		case 1:
			LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
			if (CurrentSiegeTileQuestionCount > 0 && TileManager->GetStatusOfCurrentClickedTile(CurrentPlayerIndex) == ETileStatus::Castle)
			{
				LPoints = 0;	
			}
			ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, -1 * LPoints, false);
			ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, LPoints, true);
			// Apply 1 damage to CurrentClickedTile of AttackingPlayer
			TileManager->SC_ApplyDamageToTile(CurrentPlayerIndex, 1);
			if (CurrentSiegeTileQuestionCount == 0)
			{
				DefendingPlayerState->SC_ChangePoints(-LPoints);
				AttackingPlayerState->SC_ChangePoints(LPoints);
				//TileManager->SC_AddClickedTileToTheTerritory(CurrentPlayerIndex, ETileStatus::Controlled, AttackingPlayerState->GetPlayerColor(), Round);
				/*MC_UpdatePoints(DefendingPlayerIndex,DefendingPlayerState->GetPoints());
				MC_UpdatePoints(CurrentPlayerIndex,AttackingPlayerState->GetPoints());*/
				PlayersToUpdatePoints.Add(DefendingPlayerIndex);
				PlayersToUpdatePoints.Add(CurrentPlayerIndex);
			}
			break;
		case 2:
			ConstructQuestionResult(DefendingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 100, true);
			ConstructQuestionResult(AttackingPlayerState, UsedQuestions.Num(), LastQuestion, PlayersCurrentChoices, 0, false);
			DefendingPlayerState->SC_ChangePoints(100);
			//TileManager->SC_CancelAttackOnClickedTile(CurrentPlayerIndex);
			//MC_UpdatePoints(DefendingPlayerIndex,DefendingPlayerState->GetPoints());
			PlayersToUpdatePoints.Add(DefendingPlayerIndex);
			break;
		default:
			break;
	}
}

void ABM_GameStateBase::StartPostQuestionPhase(bool bSkipToPostQuestionComplete)
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_RemoveQuestionWidget(true);
		}
	}
	if (bSkipToPostQuestionComplete ||
		(Round == EGameRound::FightForTheRemains && FightForTheRestTileWinnerIndex < 0))
	{
		PrepareNextTurn();
	}
	else
	{
		CurrentPostQuestionReadyActors = 0;
		ExpectedPostQuestionReadyActors = OnQuestionCompleted.GetAllObjects().Num();
		UE_LOG(LogTemp, Log, TEXT("PostQuestion Phase started. Expecting %d listeners."), ExpectedPostQuestionReadyActors);
		TMap<int32, EColor> LPlayerColors;
		for (const auto LAnsweringPlayerIndex : AnsweringPlayers)
		{
			LPlayerColors.Add(LAnsweringPlayerIndex, Cast<ABM_PlayerState>(PlayerArray[LAnsweringPlayerIndex])->GetPlayerColor());	
		}
		TMap<int32, ABM_TileBase*> LClickedTiles = TileManager->GetClickedTiles();
		FPostQuestionPhaseInfo PostQuestionPhaseInfo = FPostQuestionPhaseInfo(QuestionResults, LPlayerColors, Round, LClickedTiles);
		OnQuestionCompleted.Broadcast(PostQuestionPhaseInfo);
		if (Round != EGameRound::FightForTheRemains)
		{
			TMap<int32, int32> LPlayerPointsUpdate;
			for (const auto LQuestionResult : QuestionResults)
			{
				if (LQuestionResult.Value == EQuestionResult::TileCaptured)
				{
					int32 LPoints = TileManager->GetPointsOfCurrentClickedTile(CurrentPlayerIndex);
					LPlayerPointsUpdate.Add(LQuestionResult.Key, LPoints);	
				}
				else if (LQuestionResult.Value == EQuestionResult::TileDefended)
				{
					int32 LPoints = 100;
					LPlayerPointsUpdate.Add(LQuestionResult.Key, LPoints);
				}
			}
			ChangePlayerPoints(LPlayerPointsUpdate);
		}
		if (Round == EGameRound::FightForTheRemains)
		{
			CurrentPlayerIndex = FightForTheRestTileWinnerIndex; // Winner of the Shot question
			FightForTheRestTileWinnerIndex = -1;
			PassTurnToTheShotQuestionWinner();
		}
		if (ExpectedPostQuestionReadyActors == 0)
		{
			CheckPostQuestionPhaseComplete();
		}
	}
}

void ABM_GameStateBase::NotifyPostQuestionPhaseReady()
{
	++CurrentPostQuestionReadyActors;
	UE_LOG(LogBM_GameStateBase, Display, TEXT("PostQuestionListener confirmed (%d/%d)"),CurrentPostQuestionReadyActors, ExpectedPostQuestionReadyActors);
	CheckPostQuestionPhaseComplete();
}

void ABM_GameStateBase::CheckPostQuestionPhaseComplete()
{
	if (CurrentPostQuestionReadyActors >= ExpectedPostQuestionReadyActors)
	{
		UE_LOG(LogTemp, Log, TEXT("All listeners ready. Proceeding logic."));
		CurrentPostQuestionReadyActors = 0;
		ExpectedPostQuestionReadyActors = 0;
		for (auto LPlayerState : PlayerArray)
		{
			ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
			if (IsValid(LPlayerController))
			{
				ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(LPlayerController->GetPawn());
				if (IsValid(LPlayerPawn))
				{
					//LPlayerPawn->CC_RestoreCameraPropertiesFromCache();
					LPlayerPawn->CC_TravelCameraToDefault();
				}
			}
		}
		// Process to the next turn after 3 seconds
		GetWorld()->GetTimerManager().SetTimer(PauseHandle,this, &ABM_GameStateBase::PrepareNextTurn, 3.0f, false);
	}
}

void ABM_GameStateBase::NotifyPrePlayerTurnPhaseReady()
{
	++CurrentPrePlayerTurnReadyActors;
	UE_LOG(LogBM_GameStateBase, Display, TEXT("PrePlayerTurnListener confirmed (%d/%d)"), CurrentPrePlayerTurnReadyActors, ExpectedPrePlayerTurnReadyActors);
	CheckPrePlayerTurnPhaseCompleted();
}

void ABM_GameStateBase::ChangePlayerPoints(TMap<int32, int32>& NewPlayersPoints)
{
	for (const auto LPlayerIndex : NewPlayersPoints)
	{
		if (PlayerArray.IsValidIndex(LPlayerIndex.Key))
		{
			ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerIndex.Key]);
			if (IsValid(LPlayerState))
			{
				LPlayerState->SC_ChangePoints(LPlayerIndex.Value);
				MC_UpdatePoints(LPlayerIndex.Key,LPlayerState->GetPoints());
			}
		}
	}
}

void ABM_GameStateBase::CheckPrePlayerTurnPhaseCompleted()
{
	if (CurrentPrePlayerTurnReadyActors >= ExpectedPrePlayerTurnReadyActors)
	{
		UE_LOG(LogTemp, Log, TEXT("All listeners of PrePlayerTurn Phase ready. Proceeding logic to PassTurnToTheNextPlayer."));
		CurrentPrePlayerTurnReadyActors = 0;
		ExpectedPrePlayerTurnReadyActors = 0;
		// PassTurnToTheNextPlayer after 1 second
		if (Round != EGameRound::FightForTheRemains)
		{
			GetWorld()->GetTimerManager().SetTimer(PauseHandle,this, &ABM_GameStateBase::StartPlayerTurnTimer, 1.0f, false);
		}
	}
}

void ABM_GameStateBase::MC_UpdatePoints_Implementation(int32 PlayerID, float NewScore)
{
	OnPlayerPointsChanged.Broadcast(PlayerID, NewScore);
}

void ABM_GameStateBase::UpdatePlayersCyclesWidget()
{
	TArray<FPlayersCycleUI> LPlayerCyclesUI;
	ConvertPlayerCyclesToPlayerCyclesUI(LPlayerCyclesUI);
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdatePlayerTurnsAmount(LPlayerCyclesUI);
		}
	}
}

void ABM_GameStateBase::ConvertPlayerCyclesToPlayerCyclesUI(TArray<FPlayersCycleUI>& OutPlayersCyclesUI)
{
	TMap<int32, FUniqueNetIdRepl> PlayerIdsMap;
	TMap<int32, EColor> PlayerColorsMap;
	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		PlayerIdsMap.Add(i, PlayerArray[i]->GetUniqueId());
		PlayerColorsMap.Add(i, Cast<ABM_PlayerStateBase>(PlayerArray[i])->GetPlayerColor());
	}
	for (const auto LPlayerCycle : PlayerTurnsCycles)
	{
		FPlayersCycleUI LPlayerCycleUI;
		LPlayerCycleUI.CycleNumber = LPlayerCycle.CycleNumber;
		for (const auto LPermutationValue : LPlayerCycle.PlayersPermutation.Values)
		{
			if (!PlayerIdsMap.Contains(LPermutationValue))
			{
				LPlayerCycleUI.PlayersPermutation.Values.Add(FPermutationUIValue(FUniqueNetIdRepl(),
				EColor::Undefined));
			}
			else
			{
				LPlayerCycleUI.PlayersPermutation.Values.Add(FPermutationUIValue(PlayerIdsMap.FindRef(LPermutationValue),
				PlayerColorsMap.FindRef(LPermutationValue)));
			}
		}
		OutPlayersCyclesUI.Add(LPlayerCycleUI);
	}
}

void ABM_GameStateBase::WrapUpCurrentPlayersCycle()
{
	ClearPlayerTurnTimer();
	FTimerDelegate LNextRoundDelegate;
	switch (Round)
	{
		case EGameRound::BuildCastle:
			//All players set their castles, continue to SetTerritory round with 2 sec delay
			{
				SetNextGameRound(EGameRound::SetTerritory);
				ConstructPlayerTurnsCycles();
				LNextRoundDelegate.BindUObject(this, &ThisClass::PassTurnToTheNextPlayer);
				UpdatePlayersCyclesWidget();
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRoundDelegate, 2.0, false);
			}
			break;
		case EGameRound::SetTerritory:
		{
			//All players have chosen their tiles, open Choose Question with 2sec delay
			//DisableTileEdgesHighlight();
			// Mark current Cycle as completed, increment Cycle number
			PlayerTurnsCycles[CurrentPlayerTurnsCycle].bIsCompleted = true;
			++CurrentPlayerTurnsCycle;
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Choose);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);	
		}
			break;
		case EGameRound::FightForTheRemains:
		{
			//Start Shot question for the rest tiles
			//Find first not owned tile
			++CurrentPlayerCounter;
			UpdatePlayerTurn();
			QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), EQuestionType::Shot);
			GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, 2.0, false);
		}
			break;
		case EGameRound::FightForTerritory:
		{
			if (!PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
			{
				Round = GetNextGameRound();
				SetNextGameRound(EGameRound::End);
				FTimerHandle LCountResultsHandle;
				GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
				break;
			}
			++CurrentPlayerTurnsCycle;
			PassTurnToTheNextPlayer();
		}
			break;
		default: break;
	}
}

void ABM_GameStateBase::PrepareNextTurn()
{
	/*switch (Round)
	{
		case EGameRound::BuildCastle:
		{
			PassTurnToTheNextPlayer();
		}
		break;
		case EGameRound::SetTerritory:
		{
			/*
			 * Count NotOwned tiles.
			 * If there are more than 0
			 *		If there are more than Players, continue SetTerritoryRound
			 *		Else start FightForTheRestTiles special round
			 * Else start FightForTerritory round
			 #1#
			int32 NotOwnedTiles = GetNotOwnedTilesCount();	
			if (NotOwnedTiles > 0)
			{
				if(NotOwnedTiles >= BMGameMode->GetNumberOfActivePlayers())
				{
					if (!PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
					{
						SetNextGameRound(EGameRound::FightForTheRemains);
						CurrentPlayerCounter = -1;
						ConstructPlayerTurnsCycles();
						UpdatePlayersCyclesWidget();
						UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round due to Max SetTerritory turns reached"));
						WrapUpCurrentPlayersCycle();
						break;
					}
					PassTurnToTheNextPlayer();
				}
				else
				{
					SetNextGameRound(EGameRound::FightForTheRemains);
					CurrentPlayerCounter = -1;
					ConstructPlayerTurnsCycles();
					UpdatePlayersCyclesWidget();
					// instantly Wrap Up Player Cycle as there choosing of tiles is done after the question
					WrapUpCurrentPlayersCycle();
					UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for the Rest Tiles round as the number of not owned Tiles < Remaining Players"));
				}
			}
			else
			{
				SetNextGameRound(EGameRound::FightForTerritory);
				CurrentPlayerCounter = -1;
				ConstructPlayerTurnsCycles();
				UpdatePlayersCyclesWidget();
				GetWorld()->GetTimerManager().SetTimer(PauseHandle, this, &ABM_GameStateBase::PassTurnToTheNextPlayer, 3.0f, false);
				UE_LOG(LogBM_GameMode, Display, TEXT("Switch to Fight for Territory round"));
			}
			break;
		}
		case EGameRound::FightForTheRemains:
		{
			int32 NotOwnedTiles = GetNotOwnedTilesCount();
			if (NotOwnedTiles > 0 && PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle) && PlayerTurnsCycles[CurrentPlayerTurnsCycle].PlayersPermutation.Values.IsValidIndex(CurrentPlayerCounter+1))
			{
				// instantly Wrap Up Player Cycle as there is no choice in FightForTheRestTiles round
				WrapUpCurrentPlayersCycle();
				UE_LOG(LogBM_GameMode, Display, TEXT("Continue Fight for Last Tiles round"));
			}
			else
			{
				SetNextGameRound(EGameRound::FightForTerritory);
				CurrentPlayerCounter = -1;
				ConstructPlayerTurnsCycles();
				UpdatePlayersCyclesWidget();
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
			 #1#
			if (bShotQuestionIsNeeded)
			{
				OpenQuestion(EQuestionType::Shot);
				break;
			}
			if (CurrentSiegeTileQuestionCount == 0)
			{
				if (RemainingPlayers < 2 || !PlayerTurnsCycles.IsValidIndex(CurrentPlayerTurnsCycle))
				{
					SetNextGameRound(EGameRound::End);
					FTimerHandle LCountResultsHandle;
					GetWorld()->GetTimerManager().SetTimer(LCountResultsHandle,this, &ABM_GameStateBase::CountResults, 3.0f, false);
				}
				else
				{
					PassTurnToTheNextPlayer();
				}
			}
			else
			{
				OpenNextQuestion();
			}
			break;
		}
		default:
			break;
	}*/
	if (CurrentGameRoundObject)
	{
		CurrentGameRoundObject->PrepareNextTurn();
	}
}

void ABM_GameStateBase::UpdatePlayerTurn()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdatePlayerTurnWidget(CurrentPlayerTurnsCycle, CurrentPlayerCounter);
			PlayerController->UpdateCurrentPlayerNickname(CurrentPlayerIndex);
			Cast<ABM_PlayerState>(PlayerState)->CurrentQuestionAnswerSent = false;
			PlayerController->ResetTurnTimer(Round);
		}
	}
}

void ABM_GameStateBase::StopHUDAnimations()
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->StopHUDAnimations();
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