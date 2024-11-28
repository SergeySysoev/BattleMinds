// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BM_UWOptionSwitcherBase.generated.h"


UCLASS()
class BATTLEMINDS_API UBM_UWOptionSwitcherBase : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Setup();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SwitchOption(bool bDecrease);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetOptionButtonsVisibility(bool bShowOptionButtons);

	UFUNCTION(BlueprintPure)
	int32 GetNewOptionIndex(int32 CurrentOptionIndex, int32 MaxOptions, bool bDecrease) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn = "true"), Category="Option Switcher Settings")
	bool bAllowOptionsLooping = true;
};
