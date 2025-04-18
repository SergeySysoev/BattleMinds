// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "Data/DA_QuestionCategories.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Player/BM_PlayerControllerBase.h"
#include "BMQuestionsUtilsLibrary.generated.h"


UCLASS()
class BATTLEMINDS_API UBMQuestionsUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "BMQuestions")
	static void GetQuestionCategoryStruct(EQuestionCategories Category, UDA_QuestionCategories* QuestionCategoriesDataAsset, FCategory& OutCategoryStruct);

	UFUNCTION(BlueprintPure, Category = "Players")
	static EColor GetPlayerColorByIndex(int32 PlayerIndex, UObject* WorldContext);
};
