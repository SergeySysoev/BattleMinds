// Battle Minds, 2022. All rights reserved.


#include "Player/BM_PlayerPawn.h"
#include "Core/BM_GameStateBase.h"
#include "Tiles/BM_TileBase.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerPawn);

ABM_PlayerPawn::ABM_PlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABM_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->OnQuestionCompleted.AddUniqueDynamic(this, &ThisClass::HandlePostQuestionPhase);
		LGameState->OnPrePlayerTurnPhaseStarted.AddUniqueDynamic(this, &ThisClass::HandlePrePlayerTurn);
	}
}

void ABM_PlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABM_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void ABM_PlayerPawn::PostQuestionPhaseFightForTheRestTiles(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	/*TArray<ABM_TileBase*> LClickedTiles;
	PostQuestionPhaseInfo.PlayerClickedTiles.GenerateValueArray(LClickedTiles);
	ABM_TileBase* LClickedTile = LClickedTiles[0];
	if (IsValid(LClickedTile))
	{
		CC_ZoomIntoClickedTile(LClickedTile->GetZoomCameraLocation(), LClickedTile->GetZoomCameraRotation(),0.f);
	}*/
	CheckPostQuestionPhaseHandled();
}

void ABM_PlayerPawn::PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	TArray<ABM_TileBase*> LClickedTiles;
	PostQuestionPhaseInfo.PlayerClickedTiles.GenerateValueArray(LClickedTiles);
	ABM_TileBase* LClickedTile = LClickedTiles[0];
	if (IsValid(LClickedTile))
	{
		CC_ZoomIntoClickedTile(LClickedTile->GetZoomCameraLocation(), LClickedTile->GetZoomCameraRotation(),0.f, true, false);
	}
	CheckPostQuestionPhaseHandled();
}

void ABM_PlayerPawn::CheckPostQuestionPhaseHandled()
{
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->NotifyPostQuestionPhaseReady();
	}
}

void ABM_PlayerPawn::PrePlayerTurnFightForTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CC_RestoreCameraPropertiesFromCache();
	CheckPrePlayerTurnPhaseHandled();
}

void ABM_PlayerPawn::CheckPrePlayerTurnPhase_Implementation()
{
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	if (IsValid(LGameState))
	{
		LGameState->NotifyPlayerTurnPhaseReady();
	}
}


void ABM_PlayerPawn::CachedCameraProperties()
{
	CachedLocation = DesiredLocation;
	CachedRotation = DesiredRotation;
	CachedZoom = DesiredZoom;
	UE_LOG(LogBM_PlayerPawn, Display, TEXT("Cached properties: Location: %s, Rotation: %s, Zoom: %f"),
		*CachedLocation.ToString(), *CachedRotation.ToString(), CachedZoom);
}

void ABM_PlayerPawn::CC_RestoreCameraPropertiesFromCache_Implementation()
{
	SetDesiredLocation(CachedLocation, false, true);
	SetDesiredRotation(CachedRotation, false, true);
	SetDesiredZoom(CachedZoom, false, true);
}

void ABM_PlayerPawn::CC_ZoomIntoClickedTile_Implementation(FVector ZoomLocation, FRotator ZoomRotation, float Zoom, bool IgnoreLag, bool IgnoreRestrictions)
{
	CachedCameraProperties();
	UE_LOG(LogBM_PlayerPawn, Display, TEXT("New camera pos: Location: %s, Rotation: %s, Zoom: %f"),
	*ZoomLocation.ToString(), *ZoomRotation.ToString(), Zoom);
	SetDesiredLocation(ZoomLocation, IgnoreLag, IgnoreRestrictions);
	SetDesiredRotation(ZoomRotation, IgnoreLag, IgnoreRestrictions);
	SetDesiredZoom(Zoom, IgnoreLag, IgnoreRestrictions);
}

