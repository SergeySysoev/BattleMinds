// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"
#include "BM_UWQuestion.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Core/BM_GameInstance.h"
#include "Core/BM_GameModeBase.h"
#include "Core/BM_GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Tiles/BM_TileManager.h"
#include "EnhancedInputSubsystems.h"
#include "UI/BM_UWResults.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerController);

ABM_PlayerControllerBase::ABM_PlayerControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABM_PlayerControllerBase::CC_SetViewTargetWithBlend_Implementation(AActor* NewViewTarget, float BlendTime)
{
	SetViewTargetWithBlend(NewViewTarget, BlendTime);
}

void ABM_PlayerControllerBase::CC_SetGameLength_Implementation(const EGameLength GameLength)
{
	if (IsValid(PlayerHUD))
	{
		PlayerHUD->SetGameLength(GameLength);
	}
}

void ABM_PlayerControllerBase::SC_SetPlayerInfo_Implementation(const FString& InPlayerNickname)
{
	ABM_PlayerStateBase* LPlayerStateBase = GetPlayerState<ABM_PlayerStateBase>();
	if (IsValid(LPlayerStateBase))
	{
		LPlayerStateBase->SetPlayerNickname(InPlayerNickname);
	}
}

void ABM_PlayerControllerBase::CC_CheckForTileUnderCursor_Implementation(const FIntPoint& FirstTileAxials)
{
	FHitResult LHit;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));
	GetHitResultUnderCursorForObjects(TraceObjectTypes, false, LHit);
	if (Cast<ABM_TileBase>(LHit.GetActor()))
	{
		ABM_TileBase* LTileUnderCursor = Cast<ABM_TileBase>(LHit.GetActor());
		LTileUnderCursor->ShowPreviewMesh(LTileUnderCursor);
		CachedTileWithPreview = LTileUnderCursor;
	}
	else
	{
		TSubclassOf<ABM_TileManager> LTileManagerClass = ABM_TileManager::StaticClass();
		ABM_TileManager* LTileManager = Cast<ABM_TileManager>(UGameplayStatics::GetActorOfClass(GetWorld(), LTileManagerClass));
		if (IsValid(LTileManager))
		{
			ABM_TileBase* LFirstAvailableTile = LTileManager->GetTileFromClientsMap(FirstTileAxials);
			if (IsValid(LFirstAvailableTile))
			{
				UE_LOG(LogBM_PlayerController, Warning, TEXT("First available tile axials: %d,%d "),
					LFirstAvailableTile->GetAxial().X, LFirstAvailableTile->GetAxial().Y);
				LFirstAvailableTile->ForceShowPreviewMesh(LFirstAvailableTile);
				CachedTileWithPreview = LFirstAvailableTile;
			}
		}
	}
}

void ABM_PlayerControllerBase::SetPlayerInfoFromGI()
{
	if (IsLocalController())
	{
		UBM_GameInstance* LGameInstance = GetGameInstance<UBM_GameInstance>();
		if (IsValid(LGameInstance))
		{
			SC_SetPlayerInfo(LGameInstance->GetLocalPlayerName());
		}
	}
}

void ABM_PlayerControllerBase::SetInputEnabled_Implementation(bool bIsEnabled)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (bIsEnabled)
			{
				InputSystem->AddMappingContext(ClassicIMC.LoadSynchronous(), 1);
				if (IsValid(LPlayerPawn))
				{
				//	LPlayerPawn->CC_SetInputEnabled(bIsEnabled);
				}
			}
			else
			{
				UInputMappingContext* LLoadedClassicIMC = ClassicIMC.LoadSynchronous();
				if (InputSystem->HasMappingContext(LLoadedClassicIMC))
				{
					InputSystem->RemoveMappingContext(LLoadedClassicIMC);
				}
				if (IsValid(LPlayerPawn))
				{
				//	LPlayerPawn->CC_SetInputEnabled(bIsEnabled);
				}
			}
		}
	}
}

void ABM_PlayerControllerBase::CC_SetInputEnabled_Implementation(bool bIsEnabled)
{
	SetInputEnabled(bIsEnabled);
}

void ABM_PlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!EssentialIMC.IsNull())
			{
				InputSystem->AddMappingContext(EssentialIMC.LoadSynchronous(), 0);
			}
		}
	}
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->OnQuestionCompleted.AddUniqueDynamic(this, &ThisClass::HandlePostQuestionPhase);
		LGameState->OnPrePlayerTurnPhaseStarted.AddUniqueDynamic(this, &ThisClass::HandlePrePlayerTurn);
	}
}

FLinearColor ABM_PlayerControllerBase::GetPlayerColorByID(int32 PlayerID) const
{
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		return LGameState->GetPlayerLinearColorByIndex(PlayerID);
	}
	return FLinearColor::White;
}

FUniqueNetIdRepl ABM_PlayerControllerBase::GetPlayerUniqueNetIdByPlayerId(int32 PlayerID) const
{
	if(ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState()))
	{
		return LGameState->PlayerArray[PlayerID]->GetUniqueId();
	}
	return FUniqueNetIdRepl();
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
		LGameState->OnAnswerSent.Broadcast(BM_PlayerState->GetPlayerIndex());
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

void ABM_PlayerControllerBase::CC_RemoveQuestionWidget_Implementation(bool bSwitchViewTargetBackToTiles, AActor* NewViewTarget)
{
	if (QuestionWidget)
	{
		QuestionWidget->RemoveFromParent();
	}
	if (bSwitchViewTargetBackToTiles)
	{
		PlayerHUD->ShowAllWidgets();
		SetViewTargetWithBlend(GetPawn(), 0.f);
	}
}

void ABM_PlayerControllerBase::CC_OpenQuestionWidget_Implementation(FInstancedStruct LastQuestion, const TArray<int32>& AnsweringPlayers, AActor* NewViewTarget, float QuestionTimerLength)
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
		SetViewTargetWithBlend(NewViewTarget, 0.f);	
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
	QuestionWidget->SetQuestionTimerLength(QuestionTimerLength);
	if (QuestionWidget)
	{
		QuestionWidget->AddToViewport(0);
	}
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(FIntPoint TargetTile)
{
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	TSubclassOf<ABM_TileManager> LTileManagerClass = ABM_TileManager::StaticClass();
	ABM_TileManager* LTileManager = Cast<ABM_TileManager>(UGameplayStatics::GetActorOfClass(GetWorld(), LTileManagerClass));
	if (!IsValid(LPlayerState) || !IsValid(LGameState))
	{
		return;
	}
	if (LPlayerState->IsPlayerTurn())
	{
		if (LTileManager->IsTileAvailable(TargetTile))
		{
			LGameState->HandleClickedTile(TargetTile);
		}
		else
		{
			//TODO: add option to attack any tile once per game
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

bool ABM_PlayerControllerBase::SC_TryClickTheTile_Validate(FIntPoint TargetTile)
{
	return true;
}

void ABM_PlayerControllerBase::PostQuestionPhaseChooseCastle(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	/*ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerState);
	if (IsValid(LPlayerPawn) && IsValid(LPlayerState))
	{
		if (LPlayerState->IsPlayerTurn())
		{
			LPlayerPawn->CC_TravelCameraToDefault();
		}
		else
		{
			
		}
	}*/
	CheckPostQuestionPhaseHandled();
}

void ABM_PlayerControllerBase::PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	TArray<ABM_TileBase*> LClickedTiles;
	PostQuestionPhaseInfo.PlayerClickedTiles.GenerateValueArray(LClickedTiles);
	ABM_TileBase* LClickedTile = LClickedTiles[0];
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	if (IsValid(LClickedTile) && IsValid(LPlayerPawn))
	{
		LPlayerPawn->CC_ZoomIntoClickedTile(LClickedTile->GetZoomCameraLocation(), LClickedTile->GetZoomCameraRotation(),0.f, true, false);
	}
	// Immediately call check because zooming is instant
	CheckPostQuestionPhaseHandled();
}

void ABM_PlayerControllerBase::CheckPostQuestionPhaseHandled()
{
	SC_CheckPostQuestionPhaseHandled();
}

void ABM_PlayerControllerBase::SC_CheckPostQuestionPhaseHandled_Implementation()
{
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->NotifyPostQuestionPhaseReady();
	}
}

void ABM_PlayerControllerBase::PrePlayerTurnChooseCastle(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerState);
	if (IsValid(LPlayerPawn) && IsValid(LPlayerState) && LPlayerState->IsPlayerTurn())
	{
		LPlayerPawn->CC_TravelCameraToPlayerTurn();
	}
	else
	{
		CheckPrePlayerTurnPhaseHandled();
	}
}

void ABM_PlayerControllerBase::PrePlayerTurnSetTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerState);
	if (IsValid(LPlayerPawn) && IsValid(LPlayerState) && LPlayerState->IsPlayerTurn())
	{
		LPlayerPawn->CC_TravelCameraToPlayerTurn();
	}
	else
	{
		CheckPrePlayerTurnPhaseHandled();
	}
}

void ABM_PlayerControllerBase::PrePlayerTurnFightForTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(PlayerState);
	if (IsValid(LPlayerPawn) && IsValid(LPlayerState) && LPlayerState->IsPlayerTurn())
	{
		LPlayerPawn->CC_TravelCameraToPlayerTurn();
	}
	else
	{
		CheckPrePlayerTurnPhaseHandled();
	}
}

void ABM_PlayerControllerBase::SC_SetCameraDefaultProperties_Implementation(const FUniversalCameraPositionSaveFormat& InDefaultProperties)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	if (IsValid(LPlayerPawn))
	{
		LPlayerPawn->CC_SetDefaultCameraProperties(InDefaultProperties);
	}
}

void ABM_PlayerControllerBase::SC_SetCameraPlayerTurnProperties_Implementation(const FUniversalCameraPositionSaveFormat& InDefaultProperties)
{
	ABM_PlayerPawn* LPlayerPawn = Cast<ABM_PlayerPawn>(GetPawn());
	if (IsValid(LPlayerPawn))
	{
		LPlayerPawn->CC_SetPlayerTurnCameraProperties(InDefaultProperties);
	}
}

void ABM_PlayerControllerBase::CheckPrePlayerTurnPhaseHandled()
{
	SC_CheckPrePlayerTurnPhaseHandled();
}

void ABM_PlayerControllerBase::SC_CheckPrePlayerTurnPhaseHandled_Implementation()
{
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->NotifyPrePlayerTurnPhaseReady();
	}
}
