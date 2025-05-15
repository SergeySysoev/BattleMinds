// Battle Minds, 2022. All rights reserved.

#pragma once

#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"
#include "SettingAction_SafeZoneEditor.generated.h"

class UGameSetting;
class UObject;


UCLASS()
class USettingValueScalarDynamic_SafeZoneValue : public UGameSettingValueScalarDynamic
{
	GENERATED_BODY()

public:
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
};


UCLASS()
class BATTLEMINDS_API USettingAction_SafeZoneEditor : public UGameSettingAction
{
	GENERATED_BODY()

public:
	USettingAction_SafeZoneEditor();
	virtual TArray<UGameSetting*> GetChildSettings() override;

private:
	UPROPERTY()
	TObjectPtr<USettingValueScalarDynamic_SafeZoneValue> SafeZoneValueSetting;
};
