// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameStateBase.h"
#include "Camera/CameraActor.h"
#include "BM_GameInstance.h"
#include "BM_GameModeBase.h"
#include "GameRounds/BuildCastleRound.h"
#include "GameRounds/FightForTerritoryRound.h"
#include "GameRounds/FightForTheRemainsRound.h"
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
}

int32 ABM_GameStateBase::GetPointsOfTile(EGameRound GameRound) const
{
	if (IsValid(BMGameMode))
	{
		return BMGameMode->GetPointsOfTile(GameRound);
	}
	return 0;
}

void ABM_GameStateBase::StartPostCastleChosenPhase()
{
	verify(CurrentGameRoundObject);
	int32 LCurrentPlayerIndex = CurrentGameRoundObject->GetCurrentPlayerIndex();
	ABM_PlayerControllerBase* LCurrentPlayerController = GetPlayerController(LCurrentPlayerIndex);
	FUniversalCameraPositionSaveFormat LCastleCameraProperties;
	FUniversalCameraPositionSaveFormat LPlayerTurnCameraProperties;
	TileManager->GetClickedTileCastleCameraPropertiesByPlayerId(LCurrentPlayerIndex, LCastleCameraProperties);
	TileManager->GetClickedTilePlayerTurnCameraPropertiesByPlayerId(LCurrentPlayerIndex, LPlayerTurnCameraProperties);
	LCurrentPlayerController->SC_SetCameraDefaultProperties(LCastleCameraProperties);
	LCurrentPlayerController->SC_SetCameraPlayerTurnProperties(LPlayerTurnCameraProperties);
	StartPostQuestionPhase(false);
}

void ABM_GameStateBase::BeginPlay()
{
	Super::BeginPlay();
	BMGameMode = Cast<ABM_GameModeBase>(GetWorld()->GetAuthGameMode());
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

int32 ABM_GameStateBase::GetCurrentPlayerIndex()
{
	if (CurrentGameRoundObject)
	{
		return CurrentGameRoundObject->GetCurrentPlayerIndex();
	}
	return -1;
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

void ABM_GameStateBase::ChangePlayerPoints(TMap<int32, int32>& NewPlayersPoints)
{
	for (const auto LPlayerIndex : NewPlayersPoints)
	{
		if (PlayerArray.IsValidIndex(LPlayerIndex.Key))
		{
			ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerArray[LPlayerIndex.Key]);
			if (IsValid(LPlayerState))
			{
				float LNewScore = LPlayerState->GetPoints() + LPlayerIndex.Value;
				LPlayerState->SC_ChangePoints(LPlayerIndex.Value);
				MC_UpdatePoints(LPlayerIndex.Key, LNewScore);
			}
		}
	}
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

void ABM_GameStateBase::MC_UpdatePoints_Implementation(int32 PlayerID, float NewScore)
{
	OnPlayerPointsChanged.Broadcast(PlayerID, NewScore);
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
			case EGameRound::FightForTheRemains:
			{
				SwitchGameRound<UFightForTheRemainsRound>();
			}
			break;
			case EGameRound::FightForTerritory:
			{
				SwitchGameRound<UFightForTerritoryRound>();
			}
			break;
			case EGameRound::End:
			{
				CountResults();
			}
			default:
				break;
		}
	});
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, LNextRoundDelegate, 3.0f, false);
	
}

void ABM_GameStateBase::StopAllTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	GetWorld()->GetTimerManager().ClearTimer(QuestionTimerHandle);
}

void ABM_GameStateBase::HandleClickedTile(FIntPoint InClickedTile)
{
	check(HasAuthority());
	if (CurrentGameRoundObject)
	{
		int32 LCurrentPlayerIndex = CurrentGameRoundObject->GetCurrentPlayerIndex();
		ABM_PlayerControllerBase* LCurrentPlayerController = GetPlayerController(LCurrentPlayerIndex);
		ABM_PlayerState* LCurrentPlayerState = LCurrentPlayerController->GetPlayerState<ABM_PlayerState>();
		LCurrentPlayerController->CC_SetInputEnabled(false);
		LCurrentPlayerState->SetPlayerTurn(false);
		TileManager->HandleClickedTile(LCurrentPlayerIndex, InClickedTile);
		DisableTileEdgesHighlight();
		StopPlayerTurnTimer();
		CurrentGameRoundObject->HandleClickedTile(InClickedTile, LCurrentPlayerState);
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

void ABM_GameStateBase::NotifyPrePlayerTurnPhaseReady()
{
	++CurrentPrePlayerTurnReadyActors;
	UE_LOG(LogBM_GameStateBase, Display, TEXT("PrePlayerTurnListener confirmed (%d/%d)"), CurrentPrePlayerTurnReadyActors, ExpectedPrePlayerTurnReadyActors);
	CheckPrePlayerTurnPhaseCompleted();
}

void ABM_GameStateBase::CheckPrePlayerTurnPhaseCompleted()
{
	if (CurrentPrePlayerTurnReadyActors >= ExpectedPrePlayerTurnReadyActors)
	{
		UE_LOG(LogTemp, Log, TEXT("All listeners of PrePlayerTurn Phase ready. Proceeding logic to PassTurnToTheNextPlayer."));
		CurrentPrePlayerTurnReadyActors = 0;
		ExpectedPrePlayerTurnReadyActors = 0;
		// PassTurnToTheNextPlayer after 1 second
		GetWorld()->GetTimerManager().SetTimer(PauseHandle,this, &ABM_GameStateBase::StartPlayerTurnTimer, 1.0f, false);
	}
}

void ABM_GameStateBase::StartPlayerTurnTimer()
{
	check(HasAuthority());
	verify(CurrentGameRoundObject != nullptr);
	int32 LCurrentPlayerIndex = CurrentGameRoundObject->GetCurrentPlayerIndex();
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	CurrentTurnTimer = BMGameMode->GetTurnTimer()+1; // to be able to see timer widget fades away
	HighlightAvailableTiles(LCurrentPlayerIndex);
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->ResetTurnTimer(Round);
		}
	}
	ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LCurrentPlayerIndex]);
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
	LAutoChooseTileDelegate.BindUObject(this, &ABM_GameStateBase::ForceChooseAvailableTile, LCurrentPlayerIndex);
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
	verify(CurrentGameRoundObject != nullptr);
	int32 LCurrentPlayerIndex = CurrentGameRoundObject->GetCurrentPlayerIndex();
	ABM_PlayerState* LCurrentPlayerState = Cast<ABM_PlayerState>(PlayerArray[LCurrentPlayerIndex]);
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

void ABM_GameStateBase::RequestToOpenQuestion(EQuestionType QuestionType, float PauseBeforeOpeningQuestion)
{
	GetWorld()->GetTimerManager().ClearTimer(PlayerTurnHandle);
	QuestionDelegate.BindUFunction(this, FName("OpenQuestion"), QuestionType);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, QuestionDelegate, PauseBeforeOpeningQuestion, false);
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

void ABM_GameStateBase::AssignAnsweringPlayers()
{
	if (CurrentGameRoundObject)
	{
		CurrentGameRoundObject->AssignAnsweringPlayers(AnsweringPlayers);
	}
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
	if (CurrentGameRoundObject)
	{
		CurrentGameRoundObject->GatherPlayerAnswers();
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	GetWorld()->GetTimerManager().SetTimer(PauseHandle,this, &ABM_GameStateBase::VerifyAnswers, 1.7f, false);
}

void ABM_GameStateBase::VerifyAnswers()
{
	PlayersToUpdatePoints.Empty();
	UnbindAllOnBannerSpawnedTiles();
	QuestionResults.Empty();
	if (LastQuestion.GetPtr<FQuestion>())
	{
		switch (LastQuestion.GetPtr<FQuestion>()->Type)
		{
			case EQuestionType::Choose:
			{
				QuestionResults = CurrentGameRoundObject->VerifyChooseAnswers(
					LastQuestion,
					UsedQuestions.Num());
				break;
			}
			case EQuestionType::Shot:
			{
				QuestionResults = CurrentGameRoundObject->VerifyShotAnswers(
					LastQuestion,
					UsedQuestions.Num());
				break;
			}
			default:
				break;
		}
		//Show Correct answers after 1 second
		//GetWorld()->GetTimerManager().SetTimer(PauseHandle,this,&ABM_GameStateBase::ShowPlayerChoicesAndCorrectAnswer, 1.0f, false);
		ShowPlayerChoicesAndCorrectAnswer(CurrentGameRoundObject->GetPlayerChoices());
	}
}

void ABM_GameStateBase::ShowPlayerChoicesAndCorrectAnswer(TArray<FInstancedStruct>& CurrentPlayersChoices)
{
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_ShowCorrectAnswers(CurrentPlayersChoices);
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(PauseHandle);
	FTimerDelegate LPostQuestionDelegate;
	
	LPostQuestionDelegate.BindUObject(this, &ThisClass::StartPostQuestionPhase, CurrentGameRoundObject->IsShotQuestionNeeded());
	GetWorld()->GetTimerManager().SetTimer(PauseHandle, LPostQuestionDelegate, 3.f, false);	
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
		CurrentGameRoundObject->ShouldSkipToPostQuestionComplete())
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
		CurrentGameRoundObject->OnStartPostQuestion(QuestionResults);
		if (ExpectedPostQuestionReadyActors == 0)
		{
			CheckPostQuestionPhaseComplete();
		}
	}
}

void ABM_GameStateBase::NotifyPostQuestionPhaseReady(UObject* PostQuestionListener)
{
	++CurrentPostQuestionReadyActors;
	UE_LOG(LogBM_GameStateBase, Display, TEXT("PostQuestionListener %s confirmed (%d/%d)"), *PostQuestionListener->GetName(),CurrentPostQuestionReadyActors, ExpectedPostQuestionReadyActors);
	CheckPostQuestionPhaseComplete();
}

void ABM_GameStateBase::SetPlayersCamerasToDefault()
{
	for (auto LPlayerState : PlayerArray)
	{
		ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController());
		if (IsValid(LPlayerController))
		{
			ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(LPlayerController->GetPawn());
			if (IsValid(LPlayerPawn))
			{
				LPlayerPawn->CC_TravelCameraToDefault();
			}
		}
	}
}

void ABM_GameStateBase::CheckPostQuestionPhaseComplete()
{
	if (CurrentPostQuestionReadyActors >= ExpectedPostQuestionReadyActors)
	{
		UE_LOG(LogTemp, Log, TEXT("All listeners ready. Proceeding logic."));
		CurrentPostQuestionReadyActors = 0;
		ExpectedPostQuestionReadyActors = 0;
		TArray<EQuestionResult> LQuestionResultsArray;
		QuestionResults.GenerateValueArray(LQuestionResultsArray);
		if (!LQuestionResultsArray.Contains(EQuestionResult::TileDamaged))
		{
			SetPlayersCamerasToDefault();
		}
		// Process to the next turn after 3 seconds
		GetWorld()->GetTimerManager().SetTimer(PauseHandle,this, &ABM_GameStateBase::PrepareNextTurn, 3.0f, false);
	}
}

void ABM_GameStateBase::UpdatePlayersCyclesWidget()
{
	TArray<FPlayersCycleUI> LPlayerCyclesUI;
	verify(CurrentGameRoundObject);
	ConvertPlayerCyclesToPlayerCyclesUI(LPlayerCyclesUI, CurrentGameRoundObject->GetPlayersCycles());
	for (const auto PlayerState : PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->UpdatePlayerTurnsAmount(LPlayerCyclesUI);
		}
	}
}

void ABM_GameStateBase::ConvertPlayerCyclesToPlayerCyclesUI(TArray<FPlayersCycleUI>& OutPlayersCyclesUI, TArray<FPlayersCycle> InPlayersCycles)
{
	TMap<int32, FUniqueNetIdRepl> PlayerIdsMap;
	TMap<int32, EColor> PlayerColorsMap;
	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		PlayerIdsMap.Add(i, PlayerArray[i]->GetUniqueId());
		PlayerColorsMap.Add(i, Cast<ABM_PlayerStateBase>(PlayerArray[i])->GetPlayerColor());
	}
	for (const auto LPlayerCycle : InPlayersCycles)
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

void ABM_GameStateBase::PrepareNextTurn()
{
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
			PlayerController->UpdatePlayerTurnWidget(CurrentGameRoundObject->GetCurrentCycle(), CurrentGameRoundObject->GetCurrentPlayerCounter());
			PlayerController->UpdateCurrentPlayerNickname(CurrentGameRoundObject->GetCurrentPlayerIndex());
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