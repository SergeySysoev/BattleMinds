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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings | Classic")
	FTimerHandle CastleTurnTimer;
	
	UFUNCTION(BlueprintCallable)
	void PostLogin(APlayerController* NewPlayer) override;
	UFUNCTION(BlueprintCallable)
	void StartChooseCastleRound();
	UFUNCTION(BlueprintCallable)
	void StartChooseCastleTimer();
	UFUNCTION(BlueprintCallable)
	void StartSetTerritoryRound();
	UFUNCTION(BlueprintCallable)
	void UpdateChooseCastleTimer();
	UFUNCTION(BlueprintCallable)
	void ChooseFirstAvailableCastle();
};
