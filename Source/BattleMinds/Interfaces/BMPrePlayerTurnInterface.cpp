// Battle Minds, 2022. All rights reserved.

#include "BMPrePlayerTurnInterface.h"

void IBMPrePlayerTurnInterface::HandlePrePlayerTurn(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	switch (PrePlayerTurnPhaseInfo.GameRound)
	{
		case EGameRound::ChooseCastle:
			PrePlayerTurnChooseCastle(PrePlayerTurnPhaseInfo);
		break;
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

void IBMPrePlayerTurnInterface::PrePlayerTurnChooseCastle(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::PrePlayerTurnSetTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::PrePlayerTurnFightForTheRestTiles(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::PrePlayerTurnFightForTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo)
{
	CheckPrePlayerTurnPhaseHandled();
}

void IBMPrePlayerTurnInterface::CheckPrePlayerTurnPhaseHandled() {}