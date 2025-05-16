// Battle Minds, 2022. All rights reserved.

#include "Core/BM_GameInstance.h"

#include "CommonLocalPlayer.h"
#include "GameUIManagerSubsystem.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Online/CoreOnline.h"
#include "SaveGame/BMGraphicsSaveGame.h"

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

void UBM_GameInstance::Init()
{
	Super::Init();
	InitGraphicsSettings();
}

int32 UBM_GameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogBM_GameInstance, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}
		
		GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(NewPlayer));
	}
	
	return ReturnVal;
}

bool UBM_GameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: do we want to fall back to another player?
		PrimaryPlayer.Reset();
		UE_LOG(LogBM_GameInstance, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UCommonLocalPlayer>(ExistingPlayer));

	return Super::RemoveLocalPlayer(ExistingPlayer);
}

void UBM_GameInstance::ReturnToMainMenu()
{
	Super::ReturnToMainMenu();
}

void UBM_GameInstance::InitGraphicsSettings() const
{
	UBMGraphicsSaveGame* LSaveData = Cast<UBMGraphicsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(GraphicsSaveSlotName, GraphicsSaveSlotIndex)
	);

	bool bShouldRunBenchmark = false;

	if (!LSaveData)
	{
		LSaveData = Cast<UBMGraphicsSaveGame>(UGameplayStatics::CreateSaveGameObject(UBMGraphicsSaveGame::StaticClass()));
		bShouldRunBenchmark = true;
	}
	else if (!LSaveData->bIsFirstLaunchDone)
	{
		bShouldRunBenchmark = true;
	}

	if (bShouldRunBenchmark)
	{
		UE_LOG(LogBM_GameInstance, Log, TEXT("First launch benchmark"));

		if (UGameUserSettings* LSettings = GEngine->GetGameUserSettings())
		{
			LSettings->RunHardwareBenchmark();
			LSettings->ApplyHardwareBenchmarkResults();
			LSettings->ApplySettings(false);
			LSettings->SaveSettings();
		}

		LSaveData->bIsFirstLaunchDone = true;
		UGameplayStatics::SaveGameToSlot(LSaveData, GraphicsSaveSlotName, GraphicsSaveSlotIndex);
	}
	else
	{
		UE_LOG(LogBM_GameInstance, Log, TEXT("Graphic settings were initialized earlier"));
	}
}
