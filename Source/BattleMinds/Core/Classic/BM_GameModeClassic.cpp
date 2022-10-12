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
		TSubclassOf<ABM_TileBase> TileClass = ABM_TileBase::StaticClass();
		TArray<AActor*> FoundTiles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TileClass, FoundTiles);
		Tiles.Append(FoundTiles);
		Round = EGameRound::ChooseCastle;
		StartPlayerTurnTimer(0);
	}
}
