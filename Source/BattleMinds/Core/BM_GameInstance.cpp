// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameInstance.h"
#include "Online/CoreOnline.h"

DEFINE_LOG_CATEGORY(LogBM_GameInstance)

void UBM_GameInstance::SetLocalPlayerNickname(const FString& InLocalNickname)
{
	LocalNickname = InLocalNickname;
}

void UBM_GameInstance::SetLocalPlayerUniqueNetId(const FUniqueNetIdRepl& InUniqueNetId)
{
	UniqueNetIdRepl = InUniqueNetId;
}

void UBM_GameInstance::AddPlayerAvatar(FUniqueNetIdRepl PlayerUniqueNetId, EAvatarSize AvatarSize, UTexture2D* Avatar)
{
	FPlayerAvatars PlayerAvatarsRef = PlayerAvatars.FindRef(PlayerUniqueNetId);
	switch (AvatarSize)
	{
		case EAvatarSize::Small:
			PlayerAvatarsRef.AvatarSmall = Avatar;
			break;
		case EAvatarSize::Medium:
			PlayerAvatarsRef.AvatarMedium = Avatar;
			break;
		case EAvatarSize::Large:
			PlayerAvatarsRef.AvatarLarge = Avatar;
			break;
		default:break;
	}
	PlayerAvatars.Add(PlayerUniqueNetId, PlayerAvatarsRef);
}

UTexture2D* UBM_GameInstance::GetPlayerAvatarOfSize(FUniqueNetIdRepl PlayerUniqueNetId, EAvatarSize AvatarSize)
{
	FPlayerAvatars PlayerAvatarsRef = PlayerAvatars.FindRef(PlayerUniqueNetId);
	switch (AvatarSize)
	{
		case EAvatarSize::Small:
			return PlayerAvatarsRef.AvatarSmall;
		case EAvatarSize::Medium:
			return PlayerAvatarsRef.AvatarMedium;
		case EAvatarSize::Large:
			return PlayerAvatarsRef.AvatarLarge;
		default: return nullptr;
	}
}

void UBM_GameInstance::ClearPlayerAvatarsMap()
{
	PlayerAvatars.Empty();
}
