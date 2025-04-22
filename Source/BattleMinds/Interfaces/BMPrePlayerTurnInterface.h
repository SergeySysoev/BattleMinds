// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/BM_Types.h"
#include "BMPrePlayerTurnInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBMPrePlayerTurnInterface : public UInterface
{
	GENERATED_BODY()
};

class BATTLEMINDS_API IBMPrePlayerTurnInterface
{
	GENERATED_BODY()

public:

	UFUNCTION()
	virtual void HandlePrePlayerTurn(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	UFUNCTION()
	virtual void PrePlayerTurnChooseCastle(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	UFUNCTION()
	virtual void PrePlayerTurnSetTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	UFUNCTION()
	virtual void PrePlayerTurnFightForTheRestTiles(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	UFUNCTION()
	virtual void PrePlayerTurnFightForTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo);
	
	virtual void CheckPrePlayerTurnPhaseHandled();

};
