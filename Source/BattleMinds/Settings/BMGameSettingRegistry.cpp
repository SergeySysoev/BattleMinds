// Battle Minds, 2022. All rights reserved.

#include "BMGameSettingRegistry.h"
#include "BMSettingsLocal.h"
#include "BMSettingsShared.h"
#include "GameSettingCollection.h"
#include "Player/BMLocalPlayer.h"

DEFINE_LOG_CATEGORY(LogBMGameSettingRegistry)

#define LOCTEXT_NAMESPACE "BattleMinds"

UBMGameSettingRegistry::UBMGameSettingRegistry() {}

UBMGameSettingRegistry* UBMGameSettingRegistry::Get(UBMLocalPlayer* InLocalPlayer)
{
	UBMGameSettingRegistry* Registry = FindObject<UBMGameSettingRegistry>(InLocalPlayer, TEXT("BMGameSettingRegistry"), true);
	if (Registry == nullptr)
	{
		Registry = NewObject<UBMGameSettingRegistry>(InLocalPlayer, TEXT("BMGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

void UBMGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();
	
	if (UBMLocalPlayer* LocalPlayer = Cast<UBMLocalPlayer>(OwningLocalPlayer))
	{
		// Game user settings need to be applied to handle things like resolution, this saves indirectly
		LocalPlayer->GetLocalSettings()->ApplySettings(false);
		
		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}

void UBMGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	UBMLocalPlayer* BMLocalPlayer = Cast<UBMLocalPlayer>(InLocalPlayer);

	VideoSettings = InitializeVideoSettings(BMLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, BMLocalPlayer);
	RegisterSetting(VideoSettings);

	/*AudioSettings = InitializeAudioSettings(BMLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(BMLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(BMLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(BMLocalPlayer);
	RegisterSetting(GamepadSettings);*/
}

bool UBMGameSettingRegistry::IsFinishedInitializing() const
{
	if (Super::IsFinishedInitializing())
	{
		if (UBMLocalPlayer* LocalPlayer = Cast<UBMLocalPlayer>(OwningLocalPlayer))
		{
			if (LocalPlayer->GetSharedSettings() == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE