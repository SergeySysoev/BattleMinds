// Battle Minds, 2022. All rights reserved.


#include "UI/BM_UWTurnTimer.h"

void UBM_UWTurnTimer::StartTimer()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBM_UWTurnTimer::ResetTimer, 1.0, true);
}
void UBM_UWTurnTimer::ResetTimer()
{
	TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), 0.0);
	if (TimerHandle.IsValid())
		TimerHandle.Invalidate();
	StartTimer();
}
void UBM_UWTurnTimer::UpdateTimer()
{
	TimerCurrent += TimerTotal/100.0;
	TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), TimerCurrent);
}
