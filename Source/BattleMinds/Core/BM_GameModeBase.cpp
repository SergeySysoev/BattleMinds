// Battle Minds, 2021. All rights reserved.


#include "BM_GameModeBase.h"
#include "BM_GameInstance.h"
#include "BM_GameStateBase.h"
#include "Camera/CameraActor.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerPawn.h"
#include "Player/BM_PlayerState.h"

DEFINE_LOG_CATEGORY(LogBM_GameMode);

void ABM_GameModeBase::InitPlayer(APlayerController* NewPlayer)
{
	if (ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(NewPlayer->GetPlayerState<ABM_PlayerState>()))
	{
		PlayerState->BMPlayerID = NumberOfActivePlayers;
		PlayerState->MaterialTile = MaterialMap.FindRef(NumberOfActivePlayers);
		//TODO: get this info from Steam info (loaded into Save game UserProfile)
		PlayerState->Nickname = NicknameMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialCastle = CastleMaterialMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialAttack = MaterialAttackMap.FindRef(NumberOfActivePlayers);
		PlayerState->MaterialNeighbour = MaterialNeighborMap.FindRef(NumberOfActivePlayers);
		PlayerState->PlayerColor = ColorMap.FindRef(NumberOfActivePlayers);
		PlayerState->NumberOfTurns = NumberOfPlayerTurns;
		TotalSetTerritoryTurns += NumberOfPlayerTurns;
		const auto AvailablePlayerStart = FindPlayerStart(NewPlayer, TEXT(""));
		const FVector Location = AvailablePlayerStart->GetActorLocation();
		const FRotator Rotation = AvailablePlayerStart->GetActorRotation();
		ABM_PlayerPawn* SpawnedPawn = GetWorld()->SpawnActor<ABM_PlayerPawn>(PawnClass, Location, Rotation);
		//SpawnedPawn->SetActorRotation(Rotation);
		NewPlayer->Possess(SpawnedPawn);
		NumberOfActivePlayers++;
	}
}

ACameraActor* ABM_GameModeBase::GetQuestionCamera(EQuestionType QuestionType) const
{
	switch (QuestionType)
	{
		case EQuestionType::Choose:
			if (IsValid(ChooseQuestionCamera))
			{
				return ChooseQuestionCamera;
			}
			break;
		case EQuestionType::Shot:
			if (IsValid(ShotQuestionCamera))
			{
				return ShotQuestionCamera;
			}
			break;
		default: break;
	}
	return nullptr;
}

UDataTable* ABM_GameModeBase::GetQuestionsDataTable(EQuestionType QuestionType) const
{
	int32 LTableIndex;
	switch (QuestionType)
	{
		case EQuestionType::Choose:
			LTableIndex = FMath::RandRange(0, QuestionTablesChoose.Num()-1);
			if (IsValid(ChooseQuestionCamera))
			{
				return QuestionTablesChoose[LTableIndex];
			}
		break;
		case EQuestionType::Shot:
			LTableIndex = FMath::RandRange(0, QuestionTablesShot.Num()-1);
			if (IsValid(ShotQuestionCamera))
			{
				return QuestionTablesShot[LTableIndex];
			}
		break;
		default: break;
	}
	return nullptr;
}

void ABM_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	InitPlayer(NewPlayer);
}

void ABM_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	TArray<TObjectPtr<AActor>> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass() ,FoundCameras);
	for (auto Camera:FoundCameras)
	{
		if(Camera->ActorHasTag(TEXT("ChooseQuestion")))
		{
			ChooseQuestionCamera = Cast<ACameraActor>(Camera);
		}
		if(Camera->ActorHasTag(TEXT("ShotQuestion")))
		{
			ShotQuestionCamera = Cast<ACameraActor>(Camera);
		}
	}
}
