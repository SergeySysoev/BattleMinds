// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BM_UWPlayerInfo.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API UBM_UWPlayerInfo : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdatePlayerPoints(float NewPoints);
};
