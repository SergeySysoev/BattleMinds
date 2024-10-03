// Battle Minds, 2022. All rights reserved.


#include "UI/BM_UWTurnTimer.h"

void UBM_UWTurnTimer::StartTimer()
{
	TimerCurrent = 0.0;
	UpdateTimer();
}
void UBM_UWTurnTimer::ResetTimer()
{
	if (IsValid(TimerMaterialDynamic))
	{
		TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), 0.0);
	}
	TimerCurrent = 0.0;	
}
void UBM_UWTurnTimer::UpdateTimer()
{
	if (IsValid(TimerMaterialDynamic))
	{
		TimerMaterialDynamic->SetScalarParameterValue(FName(TEXT("Percent")), TimerCurrent);
	}
	TimerCurrent += TimerTotal/100.0;
}
