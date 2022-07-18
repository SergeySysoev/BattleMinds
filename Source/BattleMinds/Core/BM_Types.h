// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BM_Types.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAnswerType : uint8
{
	ChooseText UMETA(DisplayName = "Choose Text"),
	ChooseImage UMETA(DisplayName = "Choose Image"),
	ChooseNumeric UMETA(DisplayName = "Choose Numeric"),
	Shot UMETA(DisplayName = "Shot")
};
UENUM(BlueprintType)
enum class EQuestionType : uint8
{
	Choose UMETA(DisplayName = "Choose Answer"),
	Shot UMETA(DisplayName = "Shot")
};

USTRUCT(BlueprintType)
struct FAnswer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings")
	EAnswerType AnswerType = EAnswerType::ChooseText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseText"))
	FText AnswerText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseImage"))
	TObjectPtr<UTexture2D> AnswerPicture = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseNumeric || AnswerType == EAnswerType::Shot"))
	float AnswerNumber = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings")
	bool IsCorrect = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Answer settings")
	bool bWasChosen = false;
};
USTRUCT(BlueprintType)
struct FAnswerShot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Answer settings")
	int32 CorrectAnswer = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Answer settings")
	int32 PlayerAnswer = 0;
	UPROPERTY(BlueprintReadWrite, Category="Answer settings")
	int32 Difference = MAX_int32;
};

USTRUCT(BlueprintType)
struct FQuestion : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	FText Question;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	EQuestionType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings", meta = (EditCondition = "Type == EQuestionType::Choose"))
	TArray<FAnswer> Answers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings", meta = (EditCondition = "Type == EQuestionType::Shot"))
	FAnswerShot AnswerShot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	int32 PointsModifier = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Question settings", meta = (EditCondition = "Type == EQuestionType::Choose"))
	bool bWasAnswered;
	UPROPERTY(BlueprintReadWrite, Category="Question settings")
	int32 PlayerID;
	UPROPERTY(BlueprintReadWrite, Category="Question settings|Shot")
	bool bExactAnswer;
	UPROPERTY(BlueprintReadWrite, Category="Question settings|Shot")
	FTimespan ElapsedTime;

	FORCEINLINE bool operator<(const FQuestion &Other) const
	{
		return ElapsedTime < Other.ElapsedTime;
	}
};

UENUM(BlueprintType)
enum class EGameRound : uint8
{
	ChooseCastle,
	SetTerritory,
	FightForTheRestTiles,
	FightForTerritory,
	End
};

UENUM(BlueprintType)
enum class EClassicRound : uint8
{
	ChooseCastle,
	SetTerritory,
	FightForTerritory,
	CountResults
};

UCLASS()
class BATTLEMINDS_API UBM_Types : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
