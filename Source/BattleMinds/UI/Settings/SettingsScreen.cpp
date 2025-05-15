// Battle Minds, 2022. All rights reserved.


#include "SettingsScreen.h"
#include "Input/CommonUIInputTypes.h"
#include "Player/BMLocalPlayer.h"
#include "Settings/BMGameSettingRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsScreen)

class UGameSettingRegistry;

void USettingsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction)));
	ApplyHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	CancelChangesHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelChangesInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCancelChangesAction)));

}

UGameSettingRegistry* USettingsScreen::CreateRegistry()
{
	UBMGameSettingRegistry* NewRegistry = NewObject<UBMGameSettingRegistry>();

	if (ULocalPlayer* LocalPlayer = CastChecked<ULocalPlayer>(GetOwningLocalPlayer()))
	{
		NewRegistry->Initialize(LocalPlayer);
	}

	return NewRegistry;
}

void USettingsScreen::HandleBackAction()
{
	if (AttemptToPopNavigation())
	{
		return;
	}

	ApplyChanges();

	DeactivateWidget();
}

void USettingsScreen::HandleApplyAction()
{
	ApplyChanges();
}

void USettingsScreen::HandleCancelChangesAction()
{
	CancelChanges();
}

void USettingsScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyHandle))
		{
			AddActionBinding(ApplyHandle);
		}
		if (!GetActionBindings().Contains(CancelChangesHandle))
		{
			AddActionBinding(CancelChangesHandle);
		}
	}
	else
	{
		RemoveActionBinding(ApplyHandle);
		RemoveActionBinding(CancelChangesHandle);
	}
}