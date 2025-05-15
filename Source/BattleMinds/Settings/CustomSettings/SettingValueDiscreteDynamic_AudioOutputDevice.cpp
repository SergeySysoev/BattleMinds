// Battle Minds, 2022. All rights reserved.


#include "SettingValueDiscreteDynamic_AudioOutputDevice.h"
#include "AudioDeviceNotificationSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingValueDiscreteDynamic_AudioOutputDevice)

#define LOCTEXT_NAMESPACE "BMSettings"

void USettingValueDiscreteDynamic_AudioOutputDevice::OnInitialized()
{
	Super::OnInitialized();

	DevicesObtainedCallback.BindUFunction(this, FName("OnAudioOutputDevicesObtained"));
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedDeviceSwap"));

	if (UAudioDeviceNotificationSubsystem* AudioDeviceNotifSubsystem = UAudioDeviceNotificationSubsystem::Get())
	{
		AudioDeviceNotifSubsystem->DeviceAddedNative.AddUObject(this, &USettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved);
		AudioDeviceNotifSubsystem->DeviceRemovedNative.AddUObject(this, &USettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved);
		//AudioDeviceNotifSubsystem->DeviceSwitchedNative.AddUObject(this, &USettingValueDiscreteDynamic_AudioOutputDevice::DeviceSwitched);
		AudioDeviceNotifSubsystem->DefaultRenderDeviceChangedNative.AddUObject(this, &USettingValueDiscreteDynamic_AudioOutputDevice::DefaultDeviceChanged);
	}

	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void USettingValueDiscreteDynamic_AudioOutputDevice::OnAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
	int32 NewSize = AvailableDevices.Num();
	OutputDevices.Reset(NewSize++);
	OutputDevices.Append(AvailableDevices);

	OptionValues.Reset(NewSize);
	OptionDisplayTexts.Reset(NewSize);

	// Placeholder - needs to be first option so we can format the default device string later
	AddDynamicOption(TEXT(""), FText::GetEmpty());
	FString SystemDefaultDeviceName;

	for (const FAudioOutputDeviceInfo& DeviceInfo : OutputDevices)
	{
		if (!DeviceInfo.DeviceId.IsEmpty() && !DeviceInfo.Name.IsEmpty())
		{
			// System Default 
			if (DeviceInfo.bIsSystemDefault)
			{
				SystemDefaultDeviceId = DeviceInfo.DeviceId;
				SystemDefaultDeviceName = DeviceInfo.Name;
			}

			// Current Device
			if (DeviceInfo.bIsCurrentDevice)
			{
				CurrentDeviceId = DeviceInfo.DeviceId;
			}

			// Add the menu option
			AddDynamicOption(DeviceInfo.DeviceId, FText::FromString(DeviceInfo.Name));
		}
	}

	OptionDisplayTexts[0] = FText::Format(LOCTEXT("DefaultAudioOutputDevice", "Default Output - {0}"), FText::FromString(SystemDefaultDeviceName));
	SetDefaultValueFromString(TEXT(""));
	RefreshEditableState();
	
}

void USettingValueDiscreteDynamic_AudioOutputDevice::OnCompletedDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{

}

void USettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved(FString DeviceId)
{
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void USettingValueDiscreteDynamic_AudioOutputDevice::DefaultDeviceChanged(EAudioDeviceChangedRole InRole, FString DeviceId)
{
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void USettingValueDiscreteDynamic_AudioOutputDevice::SetDiscreteOptionByIndex(int32 Index)
{
	Super::SetDiscreteOptionByIndex(Index);
}


#undef LOCTEXT_NAMESPACE
