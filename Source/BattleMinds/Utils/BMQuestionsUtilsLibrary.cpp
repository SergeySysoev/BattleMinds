// Battle Minds, 2022. All rights reserved.


#include "Utils/BMQuestionsUtilsLibrary.h"

#include "Core/BM_GameInstance.h"
#include "Core/BM_GameStateBase.h"
#include "Player/BM_PlayerControllerBase.h"
#include "Player/BM_PlayerState.h"

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

EColor UBMQuestionsUtilsLibrary::GetPlayerColorByIndex(int32 PlayerIndex, UObject* WorldContext)
{
	EColor LOutColor = EColor::Undefined;
	if (!IsValid(WorldContext))
	{
		return EColor::Undefined;
	}
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(WorldContext->GetWorld()->GetGameState());
	if (IsValid(LGameState) && LGameState->PlayerArray.IsValidIndex(PlayerIndex))
	{
		return Cast<ABM_PlayerState>(LGameState->PlayerArray[PlayerIndex])->GetPlayerColor();
	}
	return LOutColor;
}
