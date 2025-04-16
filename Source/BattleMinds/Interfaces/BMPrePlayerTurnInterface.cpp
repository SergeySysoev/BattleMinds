// Battle Minds, 2022. All rights reserved.


#include "BMPrePlayerTurnInterface.h"

void IBMPrePlayerTurnInterface::HandlePrePlayerTurn_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	switch (PrePlayerTurnPhaseInfo.GameRound)
	{
		case EGameRound::SetTerritory:
			PrePlayerTurnSetTerritory(PrePlayerTurnPhaseInfo);
		break;
		case EGameRound::FightForTheRestTiles:
			PrePlayerTurnFightForTheRestTiles(PrePlayerTurnPhaseInfo);
		break;
		case EGameRound::FightForTerritory:
			PrePlayerTurnFightForTerritory(PrePlayerTurnPhaseInfo);
		default:break;
	}
}

void IBMPrePlayerTurnInterface::PrePlayerTurnSetTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::PrePlayerTurnFightForTheRestTiles_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::PrePlayerTurnFightForTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::CheckPrePlayerTurnPhase_Implementation() {}