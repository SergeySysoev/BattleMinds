// Battle Minds, 2022. All rights reserved.


#include "BattleMinds/Player/BM_PlayerState.h"

void ABM_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_PlayerState, Nickname);
	DOREPLIFETIME(ABM_PlayerState, Points);
	DOREPLIFETIME(ABM_PlayerState, MaterialTile);
	DOREPLIFETIME(ABM_PlayerState, bHasArtillery);
	DOREPLIFETIME(ABM_PlayerState, bHasTurn);
	DOREPLIFETIME(ABM_PlayerState, AnsweredQuestions);
	DOREPLIFETIME(ABM_PlayerState, MaterialCastle);
	DOREPLIFETIME(ABM_PlayerState, MaterialAttack);
}

float ABM_PlayerState::GetPoints()
{
	return Points;
}

void ABM_PlayerState::AddPoints(int32 inPoints)
{
	Points += inPoints;
}

bool ABM_PlayerState::IsPlayerTurn()
{
	return bHasTurn;
}

void ABM_PlayerState::SetPlayerTurn(bool inTurn)
{
	bHasTurn = inTurn;
}

bool ABM_PlayerState::HasArtillery()
{
	return bHasArtillery;
}
int32 ABM_PlayerState::GetCorrectAnswersNumber()
{
	int32 Count = 0;
	for (const auto Questions : AnsweredQuestions)
	{
		if (Questions.bWasAnswered)
		{
			Count++;
		}
	}
	return  Count;
}

int32 ABM_PlayerState::GetWrongAnswersNumber()
{
	int32 Count = 0;
	for (const auto Questions : AnsweredQuestions)
	{
		if (!Questions.bWasAnswered)
		{
			Count++;
		}
	}
	return  Count;
}
