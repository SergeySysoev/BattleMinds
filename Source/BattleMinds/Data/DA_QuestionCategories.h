// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/BM_Types.h"
#include "DA_QuestionCategories.generated.h"


UCLASS()
class BATTLEMINDS_API UDA_QuestionCategories : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestionCategories")
	TArray<FCategory> QuestionCategories;
};
