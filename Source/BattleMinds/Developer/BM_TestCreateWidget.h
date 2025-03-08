// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BM_TestCreateWidget.generated.h"


UCLASS()
class BATTLEMINDS_API UBM_TestCreateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void CreateWidgetsList(FString Path);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UUserWidget* CreateChosenWidget();

private:

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess=true))
	TMap<FString, TSubclassOf<UUserWidget>> WidgetClassMap;
};
