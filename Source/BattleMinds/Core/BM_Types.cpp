// Battle Minds, 2022. All rights reserved.


#include "Core/BM_Types.h"

EColor UBM_Types::GetColorStringAsEnum(FString InColor)
{
	int32 LEnumValue = FCString::Atoi((*InColor));
	EColor LStringColor = static_cast<EColor>(static_cast<uint8>(LEnumValue));
	return LStringColor;
}

int32 UBM_Types::GetGameLengthAsInt(EGameLength GameLength)
{
	switch (GameLength)
	{
		case EGameLength::Long:
			return 6;
		case EGameLength::Short:
			return 4;
		default: return 0;
	}
}