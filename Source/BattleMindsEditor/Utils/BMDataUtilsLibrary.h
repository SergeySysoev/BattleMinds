// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BMDataUtilsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMEditorDataUtils, Display, Display);

UCLASS()
class BATTLEMINDSEDITOR_API UBMDataUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Data tables")
	static void CreateQuestionsDataTable(FName QuestionsStringTableID,
		FName AnswersStringTableID,
		EQuestionType QuestionType,
		EQuestionCategories QuestionsCategory,
		FString InTableName);
};