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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings | Classic")
	FTimerHandle TerritoryTurnTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings | Classic")
	FTimerHandle BattleTurnTimer;
	UFUNCTION(BlueprintCallable)
	void PostLogin(APlayerController* NewPlayer) override;
	UFUNCTION(BlueprintCallable, Category= "Choose Castle round")
	void StartChooseCastleRound();
	UFUNCTION(BlueprintCallable, Category= "Choose Castle round")
	void StartChooseCastleTimer();
	UFUNCTION(BlueprintCallable, Category= "Choose Castle round")
	void UpdateChooseCastleTimer();
	UFUNCTION(BlueprintCallable, Category= "Choose Castle round")
	void ChooseFirstAvailableCastle();
	UFUNCTION(BlueprintCallable, Category= "Set Territory round")
	void StartSetTerritoryRound();
	UFUNCTION(BlueprintCallable, Category= "Set Territory round")
	void StartSetTerritoryTimer();
	UFUNCTION(BlueprintCallable, Category= "Set Territory round")
	void UpdateSetTerritoryTimer();
	UFUNCTION(BlueprintCallable, Category= "Set Territory round")
	void ChooseFirstAvailableTile();
};
