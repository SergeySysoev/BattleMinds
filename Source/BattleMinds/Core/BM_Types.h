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

USTRUCT(BlueprintType)
struct FAnswer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	EAnswerType AnswerType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseText"))
	FText AnswerText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseImage"))
	TObjectPtr<UTexture2D> AnswerPicture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings", meta=(EditCondition = "AnswerType == EAnswerType::ChooseNumeric"))
	float AnswerNumber;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	bool IsCorrect;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Question settings")
	bool bWasChosen;
};

USTRUCT(BlueprintType)
struct FQuestion : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	FText Question;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	TArray<FAnswer> Answers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Question settings")
	int32 PointsModifier = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Question settings")
	bool bWasAnswered;
};

UENUM(BlueprintType)
enum class EGameRound : uint8
{
	ChooseCastle,
	SetTerritory,
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
