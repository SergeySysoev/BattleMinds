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
		PlayerState->MaterialTile = MaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->Nickname = NicknameMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialCastle = CastleMaterialMap.FindRef(NumberOfActivePlayers);
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
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			//TODO: Add actual gathering logic
			PlayerController->CC_RemoveQuestionWidget();
		}
	}
}

void ABM_GameModeBase::StartPlayerTurnTimer(int32 PlayerID)
{
	GetWorld()->GetTimerManager().SetTimer(PlayerTurnHandle, this, &ABM_GameModeBase::ChooseFirstAvailableTile, TurnTimer, false);
}

void ABM_GameModeBase::ChooseFirstAvailableTile()
{
	TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, Tiles);
	for (auto Tile: Tiles)
	{
		auto FoundTile = Cast<ABM_TileBase>(Tile);
		if (FoundTile->GetStatus() == ETileStatus::NotOwned)
		{
			OpenQuestion();
			break;
		}
	}
}

void ABM_GameModeBase::NM_StartTurnTimer_Implementation()
{
	for (const auto PlayerState : GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->OnTurnUpdate.Broadcast();
		}
	} 
}

void ABM_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NumberOfActivePlayers++;
	InitPlayer(NewPlayer);
	if (NumberOfActivePlayers == Cast<UBM_GameInstance>(GetWorld()->GetGameInstance())->NumberOfPlayers)
	{
		NM_StartTurnTimer();
		StartPlayerTurnTimer(CurrentPlayerID);
	}
}
