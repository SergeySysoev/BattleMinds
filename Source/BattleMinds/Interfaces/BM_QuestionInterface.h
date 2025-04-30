// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BM_QuestionInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UBM_QuestionInterface : public UInterface
{
	GENERATED_BODY()
};

class BATTLEMINDS_API IBM_QuestionInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Debuff")
	void AddBombs();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Debuff")
	void AddIce();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Debuff")
	void AddChopLetters();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Buff")
	void ApplyFiftyFifty();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Buff")
	void ApplyShield();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Buff")
	void ShowAnswerRange();
};
