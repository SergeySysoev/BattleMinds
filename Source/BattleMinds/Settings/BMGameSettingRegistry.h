// Battle Minds, 2022. All rights reserved.

#pragma once

#include "DataSource/GameSettingDataSourceDynamic.h"
#include "GameSettingRegistry.h"
#include "Settings/BMSettingsLocal.h"
#include "BMGameSettingRegistry.generated.h"

class ULocalPlayer;
class UObject;
class UGameSettingCollection;
class UBMLocalPlayer;

//--------------------------------------
// UGameSettingRegistry implementation
//--------------------------------------

DECLARE_LOG_CATEGORY_EXTERN(LogBMGameSettingRegistry, Log, Log);

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
GET_FUNCTION_NAME_STRING_CHECKED(UBMLocalPlayer, GetSharedSettings),				\
GET_FUNCTION_NAME_STRING_CHECKED(UBMSettingsShared, FunctionOrPropertyName)		\
}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
GET_FUNCTION_NAME_STRING_CHECKED(UBMLocalPlayer, GetLocalSettings),				\
GET_FUNCTION_NAME_STRING_CHECKED(UBMSettingsLocal, FunctionOrPropertyName)		\
}))

UCLASS()
class BATTLEMINDS_API UBMGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	UBMGameSettingRegistry();

	static UBMGameSettingRegistry* Get(UBMLocalPlayer* InLocalPlayer);
	
	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(UBMLocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, UBMLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeAudioSettings(UBMLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGameplaySettings(UBMLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeMouseAndKeyboardSettings(UBMLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGamepadSettings(UBMLocalPlayer* InLocalPlayer);

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> VideoSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GamepadSettings;
};
