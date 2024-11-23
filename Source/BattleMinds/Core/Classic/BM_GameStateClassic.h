// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_GameStateBase.h"
#include "BM_GameStateClassic.generated.h"

class ABM_GameModeBase;
class ABM_PlayerState;

UCLASS()
class BATTLEMINDS_API ABM_GameStateClassic : public ABM_GameStateBase
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void AutoAssignTiles();

	UFUNCTION(BlueprintCallable)
	void ForceSetGameRound(EGameRound NewRound);
};
