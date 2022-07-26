// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_GameModeBase.h"
#include "BM_GameModeClassic.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API ABM_GameModeClassic : public ABM_GameModeBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void PostLogin(APlayerController* NewPlayer) override;
};
