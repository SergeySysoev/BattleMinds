// Battle Minds, 2022. All rights reserved.


#include "Utils/BMQuestionsUtilsLibrary.h"

void UBMQuestionsUtilsLibrary::GetQuestionCategoryStruct(EQuestionCategories Category, UDA_QuestionCategories* QuestionCategoriesDataAsset, FCategory& OutCategoryStruct)
{
	if (!IsValid(QuestionCategoriesDataAsset))
	{
		return ;
	}
	for (const auto LCategory : QuestionCategoriesDataAsset->QuestionCategories)
	{
		if (LCategory.Name == Category)
		{
			OutCategoryStruct = LCategory;
			return;
		}
	}
}