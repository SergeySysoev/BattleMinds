// Battle Minds, 2022. All rights reserved.


#include "Core/Classic/BM_GameModeClassic.h"

#include "Core/BM_GameInstance.h"
#include "Core/BM_GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"
#include "Tiles/BM_TileBase.h"

void ABM_GameModeClassic::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (NumberOfActivePlayers == Cast<UBM_GameInstance>(GetWorld()->GetGameInstance())->NumberOfPlayers)
	{
		GetWorld()->GetTimerManager().SetTimer(StartGameTimer, this, &ABM_GameModeClassic::StartGame, 5.0, false, 1.0f);
	}
}

void ABM_GameModeClassic::StartGame()
{
	TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
	TArray<AActor*> FoundTiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, FoundTiles);
	Tiles.Append(FoundTiles);
	Round = EGameRound::ChooseCastle;
	//TODO: initialize Players HUD
	for (const auto PlayerState: GetGameState<ABM_GameStateBase>()->PlayerArray)
	{
		if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController()))
		{
			PlayerController->CC_InitPlayerHUD(GetGameState<ABM_GameStateBase>()->PlayerArray);
		}
	}
	//StartPlayerTurnTimer(0);
}
