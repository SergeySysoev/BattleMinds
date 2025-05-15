// Battle Minds, 2022. All rights reserved.


#include "SettingValueDiscrete_MobileFPSType.h"
#include "Settings/BMSettingsLocal.h"
#include "Performance/BMPerformanceSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingValueDiscrete_MobileFPSType)

#define LOCTEXT_NAMESPACE "BMSettings"

USettingValueDiscrete_MobileFPSType::USettingValueDiscrete_MobileFPSType()
{
}

void USettingValueDiscrete_MobileFPSType::OnInitialized()
{
	Super::OnInitialized();

	const UBMPlatformSpecificRenderingSettings* PlatformSettings = UBMPlatformSpecificRenderingSettings::Get();
	const UBMSettingsLocal* UserSettings = UBMSettingsLocal::Get();

	for (int32 TestLimit : PlatformSettings->MobileFrameRateLimits)
	{
		if (UBMSettingsLocal::IsSupportedMobileFramePace(TestLimit))
		{
			FPSOptions.Add(TestLimit, MakeLimitString(TestLimit));
		}
	}

	const int32 FirstFrameRateWithQualityLimit = UserSettings->GetFirstFrameRateWithQualityLimit();
	if (FirstFrameRateWithQualityLimit > 0)
	{
		SetWarningRichText(FText::Format(LOCTEXT("MobileFPSType_Note", "<strong>Note: Changing the framerate setting to {0} or higher might lower your Quality Presets.</>"), MakeLimitString(FirstFrameRateWithQualityLimit)));
	}
}

int32 USettingValueDiscrete_MobileFPSType::GetDefaultFPS() const
{
	return UBMSettingsLocal::GetDefaultMobileFrameRate();
}

FText USettingValueDiscrete_MobileFPSType::MakeLimitString(int32 Number)
{
	return FText::Format(LOCTEXT("MobileFrameRateOption", "{0} FPS"), FText::AsNumber(Number));
}

void USettingValueDiscrete_MobileFPSType::StoreInitial()
{
	InitialValue = GetValue();
}

void USettingValueDiscrete_MobileFPSType::ResetToDefault()
{
	SetValue(GetDefaultFPS(), EGameSettingChangeReason::ResetToDefault);
}

void USettingValueDiscrete_MobileFPSType::RestoreToInitial()
{
	SetValue(InitialValue, EGameSettingChangeReason::RestoreToInitial);
}

void USettingValueDiscrete_MobileFPSType::SetDiscreteOptionByIndex(int32 Index)
{
	TArray<int32> FPSOptionsModes;
	FPSOptions.GenerateKeyArray(FPSOptionsModes);

	int32 NewMode = FPSOptionsModes.IsValidIndex(Index) ? FPSOptionsModes[Index] : GetDefaultFPS();

	SetValue(NewMode, EGameSettingChangeReason::Change);
}

int32 USettingValueDiscrete_MobileFPSType::GetDiscreteOptionIndex() const
{
	TArray<int32> FPSOptionsModes;
	FPSOptions.GenerateKeyArray(FPSOptionsModes);
	return FPSOptionsModes.IndexOfByKey(GetValue());
}

TArray<FText> USettingValueDiscrete_MobileFPSType::GetDiscreteOptions() const
{
	TArray<FText> Options;
	FPSOptions.GenerateValueArray(Options);

	return Options;
}

int32 USettingValueDiscrete_MobileFPSType::GetValue() const
{
	return UBMSettingsLocal::Get()->GetDesiredMobileFrameRateLimit();
}

void USettingValueDiscrete_MobileFPSType::SetValue(int32 NewLimitFPS, EGameSettingChangeReason InReason)
{
	UBMSettingsLocal::Get()->SetDesiredMobileFrameRateLimit(NewLimitFPS);

	NotifySettingChanged(InReason);
}

#undef LOCTEXT_NAMESPACE

