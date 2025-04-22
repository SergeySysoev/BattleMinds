// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "UObject/Interface.h"
#include "BMPostQuestionPhase.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, NotBlueprintable)
class UBMPostQuestionPhase : public UInterface
{
	GENERATED_BODY()
};

class BATTLEMINDS_API IBMPostQuestionPhase
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(Category = "BMPostQuestionPhase")
	virtual void HandlePostQuestionPhase(FPostQuestionPhaseInfo PostQuestionPhaseInfo);

	UFUNCTION(Category = "BMPostQuestionPhase")
	virtual void PostQuestionPhaseChooseCastle(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);
	
	UFUNCTION(Category = "BMPostQuestionPhase")
	virtual void PostQuestionPhaseSetTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);

	UFUNCTION(Category = "BMPostQuestionPhase")
	virtual void PostQuestionPhaseFightForTheRestTiles(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);

	UFUNCTION(Category = "BMPostQuestionPhase")
	virtual void PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);

	UFUNCTION(BlueprintCallable, Category = "BMPostQuestionPhase")
	virtual void CheckPostQuestionPhaseHandled();
};
