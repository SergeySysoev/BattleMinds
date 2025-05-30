﻿// Battle Minds, 2022. All rights reserved.

#include "BM_PlayerStateBase.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerStateBase);

void ABM_PlayerStateBase::SetPlayerColor_Implementation(EColor NewColor)
{
	PlayerColor = NewColor;
}

void ABM_PlayerStateBase::SetPlayerAvatar(UTexture2D* NewAvatar)
{
	if (IsValid(NewAvatar))
	{
		PlayerAvatar = NewAvatar;
		bAvatarInitialized = true;
	}
	else
	{
		bAvatarInitialized = false;
	}
}

void ABM_PlayerStateBase::SetPlayerNickname(FString NewNickname)
{
	PlayerNickname = NewNickname;
	bNicknameInitialized = true;
}

void ABM_PlayerStateBase::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (ABM_PlayerStateBase* NewPlayerState = Cast<ABM_PlayerStateBase>(PlayerState))
	{
		NewPlayerState->PlayerColor = PlayerColor;
		NewPlayerState->PlayerAvatar = PlayerAvatar;
		NewPlayerState->PlayerNickname = PlayerNickname;
	}
}

void ABM_PlayerStateBase::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	if (ABM_PlayerStateBase* OldPlayerState = Cast<ABM_PlayerStateBase>(PlayerState))
	{
		SetPlayerAvatar(OldPlayerState->GetPlayerAvatar());
		SetPlayerNickname(OldPlayerState->GetPlayerNickname());
		SetPlayerColor(OldPlayerState->GetPlayerColor());
	}
}

void ABM_PlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_PlayerStateBase, PlayerColor);
	DOREPLIFETIME(ABM_PlayerStateBase, PlayerAvatar);
	DOREPLIFETIME(ABM_PlayerStateBase, PlayerNickname);
	DOREPLIFETIME(ABM_PlayerStateBase, bNicknameInitialized);
	DOREPLIFETIME(ABM_PlayerStateBase, bAvatarInitialized);
}

void ABM_PlayerStateBase::OnRep_Color()
{
	PlayerColorChanged();
}

void ABM_PlayerStateBase::PlayerColorChanged() {}