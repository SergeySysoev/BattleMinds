// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Core/BM_Types.h"
#include "BM_UWPlayerHUD.generated.h"


class UBM_UWPlayerInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTurnUpdated);

UCLASS()
class BATTLEMINDS_API UBM_UWPlayerHUD : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlayerTurnUpdated OnPlayerTurnUpdated;
	
	UFUNCTION(BlueprintImplementableEvent)
	void InitPlayersInfo(const TArray<FPlayerInfo>& PlayersHUDInfo);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void ShowWarningPopup(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void HideAllExceptForPlayerInfo();

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void ShowAllWidgets();

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void SetGameLength(EGameLength GameLength);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void SetTurnTimerLength(float NewLength);

protected:

	/* Map of WBP_PlayerInfo widgets for each Player*/
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, UBM_UWPlayerInfo*> PlayerInfos;
};
