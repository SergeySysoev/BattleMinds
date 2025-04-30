// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BMGameUtilsLibrary.generated.h"


UCLASS()
class BATTLEMINDS_API UBMGameUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "BMGameUtils")
	static bool IsEditorWorldContext(UObject* InWorldContext);
};
