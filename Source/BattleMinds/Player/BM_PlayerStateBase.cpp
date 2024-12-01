// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerStateBase.h"

void ABM_PlayerStateBase::SetPlayerColor_Implementation(EColor NewColor)
{
	PlayerColor = NewColor;
}

void ABM_PlayerStateBase::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (ABM_PlayerStateBase* NewPlayerState = Cast<ABM_PlayerStateBase>(PlayerState))
	{
		NewPlayerState->PlayerColor = PlayerColor;
	}
}

void ABM_PlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_PlayerStateBase, PlayerColor);
}