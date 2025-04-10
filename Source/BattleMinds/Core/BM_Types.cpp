// Battle Minds, 2022. All rights reserved.


#include "Core/BM_Types.h"

bool FPostQuestionPhaseInfo::ContainsResultType(EQuestionResult ResultTypeToCheck) const
{
	for (const auto LQuestionResult : QuestionResultsPerPlayer)
	{
		if (LQuestionResult.Value == ResultTypeToCheck)
		{
			return true;
		}
	}
	return false;
}

EColor UBM_Types::GetColorStringAsEnum(FString InColor)
{
	int32 LEnumValue = FCString::Atoi((*InColor));
	EColor LStringColor = static_cast<EColor>(static_cast<uint8>(LEnumValue));
	return LStringColor;
}

int32 UBM_Types::GetGameLengthAsInt(EGameLength GameLength)
{
	switch (GameLength)
	{
		case EGameLength::Long:
			return 6;
		case EGameLength::Short:
			return 4;
		default: return 0;
	}
}

TArray<FPermutation> UBM_Types::GenerateAllPermutations(TArray<int32>& ElementsOfPermutations)
{
	TArray<FPermutation> LResults;
	TArray<int32> LCurrent;
	Backtrack(LCurrent, ElementsOfPermutations, LResults);
	return LResults;
}

TArray<FPermutation> UBM_Types::GenerateNumberOfPermutations(TArray<int32>& ElementsOfPermutations, int32 NumberOfPermutations)
{
	TArray<FPermutation> LAllPermutations;
	TArray<FPermutation> LResults;
	TArray<int32> LCurrent;
	Backtrack(LCurrent, ElementsOfPermutations, LAllPermutations);
	if (LAllPermutations.Num() < NumberOfPermutations)
	{
		LResults.Append(LAllPermutations);
		int32 LRemaining = NumberOfPermutations - LAllPermutations.Num();
		int32 LFullPermutationsArrayNumAdditional = LRemaining / LAllPermutations.Num();
		int32 LRemainingPermutationsNum = LRemaining % LAllPermutations.Num();
		for (int32 i = 0; i < LFullPermutationsArrayNumAdditional; i++)
		{
			LResults.Append(LAllPermutations);
		}
	}
	return LResults;
}

void UBM_Types::Backtrack(TArray<int32>& Current, TArray<int32>& Elements, TArray<FPermutation>& Results)
{
	if (Elements.Num() == 0)
	{
		FPermutation LNewPerm;
		LNewPerm.Values = Current;
		Results.Add(LNewPerm);
		return;
	}

	for (int32 i = 0; i < Elements.Num(); i++)
	{
		int32 Chosen = Elements[i];

		// Добавляем выбранный элемент в текущую перестановку
		Current.Add(Chosen);
		Elements.RemoveAt(i);

		// Рекурсивный вызов
		Backtrack(Current, Elements, Results);

		// Откат изменений (Backtracking)
		Elements.Insert(Chosen, i);
		Current.Pop();
	}
}