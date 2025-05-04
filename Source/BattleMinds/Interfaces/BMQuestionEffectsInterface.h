// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BMQuestionEffectsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UBMQuestionEffectsInterface : public UInterface
{
	GENERATED_BODY()
};

/* Interface to be implemented on Actors that should react on different applied bonuses during Questions*/
class BATTLEMINDS_API IBMQuestionEffectsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Question Effects | Reaction | Bombs")
	void ReactToBombExplosion(FVector Location, FVector WorldDirection);
};
