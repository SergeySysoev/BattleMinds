// Universal Camera Plugin - Mathieu Jacq 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SharedStructs.h"
#include "UniversalCameraSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALCAMERAPLUGIN_API UUniversalCameraSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FUniversalCameraPositionSaveFormat SavedPosition;
	UPROPERTY()
	bool bIsValidSavePosition = false;

	UPROPERTY()
	TArray<uint8> SavedSettings;
	UPROPERTY()
	bool bIsValidSaveSettings = false;


};
