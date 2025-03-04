// Battle Minds, 2022. All rights reserved.


#include "Data/DA_QuestionTables.h"

void UDA_QuestionTables::GetTablesOfCategories(TArray<EQuestionCategories> Categories, TArray<UDataTable*>& OutTables) const
{
	for (const EQuestionCategories& Category : Categories)
	{
		if (auto LFoundTable = QuestionTables.FindRef(Category); IsValid(LFoundTable))
		{
			OutTables.Add(LFoundTable);
		}
	}
}