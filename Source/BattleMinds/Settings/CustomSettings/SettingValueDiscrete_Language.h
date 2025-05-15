// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingValueDiscrete.h"
#include "SettingValueDiscrete_Language.generated.h"

class FText;
class UObject;

UCLASS()
class BATTLEMINDS_API USettingValueDiscrete_Language : public UGameSettingValueDiscrete
{
	GENERATED_BODY()

public:
	
	USettingValueDiscrete_Language();
	
	/** UGameSettingValue */
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
	virtual void OnApply() override;

protected:
	TArray<FString> AvailableCultureNames;
};
