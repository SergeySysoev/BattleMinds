// Battle Minds, 2022. All rights reserved.

#include "Core/Classic/BM_GameModeClassic.h"
#include "Core/BM_GameInstance.h"
#include "Core/BM_GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"
#include "Player/BM_PlayerStateBase.h"
#include "Tiles/BM_TileManager.h"

ABM_GameModeClassic::ABM_GameModeClassic()
{
	MaxNumberOfActivePlayers = 3;
}

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
	ABM_GameStateBase* LGameStateBase = GetGameState<ABM_GameStateBase>();
	if (IsValid(LGameStateBase))
	{
		LGameStateBase->InitGameState();
		TArray<FPlayerInfo> LPlayersHUDInfo;
		for (const auto LPlayerState: GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			ABM_PlayerState* LPS = Cast<ABM_PlayerState>(LPlayerState);
			if (IsValid(LPS))
			{
				FPlayerInfo LNewPlayerInfo;
				LNewPlayerInfo.Nickname = LPS->GetPlayerNickname();
				LNewPlayerInfo.Color = LPS->GetPlayerColor();
				LNewPlayerInfo.PlayerID = LPS->GetPlayerIndex();
				LPlayersHUDInfo.Add(LNewPlayerInfo);
			}
		}
		for (const auto LPlayerState: GetGameState<ABM_GameStateBase>()->PlayerArray)
		{
			if (ABM_PlayerControllerBase* PlayerController = Cast<ABM_PlayerControllerBase>(LPlayerState->GetPlayerController()))
			{
				PlayerController->CC_InitPlayerHUD(LPlayersHUDInfo);
				PlayerController->CC_SetGameLength(SelectedGameLength);
			}
		}
		AActor* LFoundManager = UGameplayStatics::GetActorOfClass(GetWorld(), ABM_TileManager::StaticClass());
		if (IsValid(LFoundManager))
		{
			ABM_TileManager* LTileManager = Cast<ABM_TileManager>(LFoundManager);
			LTileManager->GenerateMap(2);
		}
		//StartPlayerTurnTimer(0);
	}
}

void ABM_GameModeClassic::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	if (NumberOfActivePlayers == Cast<UBM_GameInstance>(GetWorld()->GetGameInstance())->NumberOfPlayers)
	{
		GetWorld()->GetTimerManager().SetTimer(StartGameTimer, this, &ABM_GameModeClassic::StartGame, 5.0, false, 1.0f);
	}
}
