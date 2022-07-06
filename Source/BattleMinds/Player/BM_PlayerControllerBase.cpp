// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerControllerBase.h"
#include "BattleMinds/Tiles/BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerController);

ABM_PlayerControllerBase::ABM_PlayerControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABM_PlayerControllerBase::SC_TryClickTheTile_Implementation(ABM_TileBase* TargetTile)
{
	ABM_PlayerState* BM_PlayerState = Cast<ABM_PlayerState>(this->PlayerState);
	if (TargetTile->GetOwningPlayerNickname() == BM_PlayerState->Nickname || TargetTile->GetStatus() == ETileStatus::NotOwned)
		TargetTile->TileWasClicked(EKeys::LeftMouseButton, BM_PlayerState->Nickname, BM_PlayerState->Material);
	else
		UE_LOG(LogBM_PlayerController, Warning, TEXT("You have clicked a tile that is already owned"));
	//MC_TryClickTheTile_Implementation(TargetTile);
}

bool ABM_PlayerControllerBase::SC_TryClickTheTile_Validate(ABM_TileBase* TargetTile)
{
	return true;
}