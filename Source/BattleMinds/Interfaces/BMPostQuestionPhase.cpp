// Battle Minds, 2022. All rights reserved.

#include "Interfaces/BMPostQuestionPhase.h"

void IBMPostQuestionPhase::HandlePostQuestionPhase(FPostQuestionPhaseInfo PostQuestionPhaseInfo)
{
	switch (PostQuestionPhaseInfo.GameRound)
	{
		case EGameRound::ChooseCastle:
			PostQuestionPhaseChooseCastle(PostQuestionPhaseInfo);
		break;
		case EGameRound::SetTerritory:
			PostQuestionPhaseSetTerritory(PostQuestionPhaseInfo);
		break;
		case EGameRound::FightForTheRestTiles:
			PostQuestionPhaseFightForTheRestTiles(PostQuestionPhaseInfo);
		break;
		case EGameRound::FightForTerritory:
			PostQuestionPhaseFightForTerritory(PostQuestionPhaseInfo);
		default:break;
	}
}

void IBMPostQuestionPhase::PostQuestionPhaseChooseCastle(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	CheckPostQuestionPhaseHandled();
}

void IBMPostQuestionPhase::PostQuestionPhaseSetTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	CheckPostQuestionPhaseHandled();
}

void IBMPostQuestionPhase::PostQuestionPhaseFightForTheRestTiles(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	CheckPostQuestionPhaseHandled();
}

void IBMPostQuestionPhase::PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo)
{
	CheckPostQuestionPhaseHandled();
}

void IBMPostQuestionPhase::CheckPostQuestionPhaseHandled()
{
	
}