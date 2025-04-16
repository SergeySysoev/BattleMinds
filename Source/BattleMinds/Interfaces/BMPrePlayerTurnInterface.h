// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/BM_Types.h"
#include "BMPrePlayerTurnInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UBMPrePlayerTurnInterface : public UInterface
{
	GENERATED_BODY()
};

class BATTLEMINDS_API IBMPrePlayerTurnInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void HandlePrePlayerTurn(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	virtual void HandlePrePlayerTurn_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void PrePlayerTurnSetTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	virtual void PrePlayerTurnSetTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void PrePlayerTurnFightForTheRestTiles(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	virtual void PrePlayerTurnFightForTheRestTiles_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void PrePlayerTurnFightForTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	virtual void PrePlayerTurnFightForTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void CheckPrePlayerTurnPhaseHandled();
	virtual void CheckPrePlayerTurnPhase_Implementation();

};
