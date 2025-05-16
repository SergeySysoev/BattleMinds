// Battle Minds, 2022. All rights reserved.


#include "BMLocalPlayer.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Settings/BMSettingsLocal.h"
#include "Settings/BMSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BMLocalPlayer)

UBMLocalPlayer::UBMLocalPlayer() {}

void UBMLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (UBMSettingsLocal* LocalSettings = GetLocalSettings())
	{
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &UBMLocalPlayer::OnAudioOutputDeviceChanged);
	}
}

void UBMLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UBMLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UBMLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

UBMSettingsLocal* UBMLocalPlayer::GetLocalSettings() const
{
	return UBMSettingsLocal::Get();
}

UBMSettingsShared* UBMLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// On PC it's okay to use the sync load because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
		
		if (bCanLoadBeforeLogin)
		{
			SharedSettings = UBMSettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// We need to wait for user login to get the real settings so return temp ones
			SharedSettings = UBMSettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}

void UBMLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
	if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings)
	{
		// Already loaded once, don't reload
		return;
	}

	ensure(UBMSettingsShared::AsyncLoadOrCreateSettings(this, UBMSettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &UBMLocalPlayer::OnSharedSettingsLoaded)));
}

void UBMLocalPlayer::OnSharedSettingsLoaded(UBMSettingsShared* LoadedOrCreatedSettings)
{
	// The settings are applied before it gets here
	if (ensure(LoadedOrCreatedSettings))
	{
		// This will replace the temporary or previously loaded object which will GC out normally
		SharedSettings = LoadedOrCreatedSettings;

		NetIdForSharedSettings = GetCachedUniqueNetId();
	}
}

void UBMLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UBMLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult) {}

void UBMLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController) {}