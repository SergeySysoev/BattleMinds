// Battle Minds, 2022. All rights reserved.


#include "UI/BM_UWTurnTimer.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UBM_UWTurnTimer::StartTimer()
{
	TimerCurrent = 0.0;
	UpdateTimer();
	if (IsValid(TimerSound))
	{
		SpawnedTimerSound = UGameplayStatics::SpawnSound2D(GetOwningPlayer(), TimerSound);
	}
	GetOwningPlayer()->GetWorldTimerManager().SetTimer(TimerHandle, this, &UBM_UWTurnTimer::UpdateTimer, 1.0, true);
}
void UBM_UWTurnTimer::ResetTimer()
{
	StopTimer();
	if (IsValid(TimerMaterialDynamic))
	{
		TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), 0.0);
	}
	StartTimer();
}
void UBM_UWTurnTimer::UpdateTimer()
{
	TimerCurrent += TimerTotal/100.0;
	if (IsValid(TimerMaterialDynamic))
	{
		TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), TimerCurrent);
	}
}

void UBM_UWTurnTimer::StopTimer()
{
	if (IsValid(SpawnedTimerSound))
	{
		SpawnedTimerSound->Stop();
	}
	TimerCurrent = 0.0;
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(TimerHandle);
}
