// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/BM_QuestionInterface.h"
#include "BM_UWQuestion.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API UBM_UWQuestion : public UUserWidget, public IBM_QuestionInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Question settings")
	FName QuestionName;
};
