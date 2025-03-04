// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "Engine/DataAsset.h"
#include "DA_QuestionTables.generated.h"

UCLASS()
class BATTLEMINDS_API UDA_QuestionTables : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<EQuestionCategories, UDataTable*> QuestionTables;

	UFUNCTION(BlueprintPure)
	void GetTablesOfCategories(TArray<EQuestionCategories> Categories, TArray<UDataTable*>& OutTables) const;
	
};
