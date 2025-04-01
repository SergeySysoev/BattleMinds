// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BMGraphicsSaveGame.generated.h"

UCLASS()
class BATTLEMINDS_API UBMGraphicsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bIsFirstLaunchDone = false;
};
