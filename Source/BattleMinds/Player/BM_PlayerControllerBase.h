// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerControllerBase.generated.h"

class ABM_TileBase;
class ABM_PlayerState;

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerController, Display, All);

UCLASS()
class BATTLEMINDS_API ABM_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABM_PlayerControllerBase();

protected:
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile);
	
	UFUNCTION(NetMulticast, Reliable, WithValidation, BlueprintCallable)
	void MC_TryClickTheTile(ABM_TileBase* TargetTile);
};
