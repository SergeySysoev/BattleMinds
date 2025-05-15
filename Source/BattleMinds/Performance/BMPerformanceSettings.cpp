// Battle Minds, 2022. All rights reserved.


#include "BMPerformanceSettings.h"

UBMPlatformSpecificRenderingSettings::UBMPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const UBMPlatformSpecificRenderingSettings* UBMPlatformSpecificRenderingSettings::Get()
{
	UBMPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

UBMPerformanceSettings::UBMPerformanceSettings()
{
	PerPlatformSettings.Initialize(UBMPlatformSpecificRenderingSettings::StaticClass());
	CategoryName = TEXT("Game");
	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });
}