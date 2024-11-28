// Battle Minds, 2022. All rights reserved.


#include "Core/BM_Types.h"

EColor UBM_Types::GetColorStringAsEnum(FString InColor)
{
	int32 LEnumValue = FCString::Atoi((*InColor));
	EColor LStringColor = static_cast<EColor>(static_cast<uint8>(LEnumValue));
	return LStringColor;
}