// Battle Minds, 2022. All rights reserved.


#include "BM_UWOptionSwitcherBase.h"

int32 UBM_UWOptionSwitcherBase::GetNewOptionIndex(int32 CurrentOptionIndex, int32 MaxOptions, bool bDecrease) const
{
	int32 LNewOptionIndex;
	int32 LMinOptions = 0;
	if (bDecrease)
	{
		if (CurrentOptionIndex <= LMinOptions)
		{
			if (bAllowOptionsLooping)
			{
				LNewOptionIndex = MaxOptions;
			}
			else
			{
				LNewOptionIndex = LMinOptions;
			}
		}
		else
		{
			LNewOptionIndex = CurrentOptionIndex-1;
		}
	}
	else
	{
		if (CurrentOptionIndex >= MaxOptions)
		{
			if (bAllowOptionsLooping)
			{
				LNewOptionIndex = LMinOptions;
			}
			else
			{
				LNewOptionIndex = MaxOptions;
			}
		}
		else
		{
			LNewOptionIndex = CurrentOptionIndex+1;
		}
	}
	return LNewOptionIndex;
}