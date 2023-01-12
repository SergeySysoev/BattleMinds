// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BM_UWPlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API UBM_UWPlayerHUD : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePlayersInfo(const TArray<APlayerState*>& Players);
};
