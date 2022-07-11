// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/BM_PlayerControllerBase.h"
#include "TimerManager.h"
#include "BM_UWTurnTimer.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API UBM_UWTurnTimer : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget settings")
	UMaterialInstanceDynamic* TimerMaterialDynamic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer settings")
	float TimerTotal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer settings")
	float TimerCurrent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer settings")
	ABM_PlayerControllerBase* OwningPlayerController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer settings")
	FTimerHandle TimerHandle;

	UFUNCTION(BlueprintCallable)
	void StartTimer();
	UFUNCTION(BlueprintCallable)
	void ResetTimer();
	UFUNCTION(BlueprintCallable)
	void UpdateTimer();
};
