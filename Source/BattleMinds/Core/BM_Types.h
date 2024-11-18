// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InstancedStruct.h"
#include "BM_Types.generated.h"

UENUM(BlueprintType)
enum class EQuestionCategories : uint8
{
	Science UMETA(DisplayedName = "Science"),
	History UMETA(DisplayedName = "History"),
	Literature UMETA(DisplayedName = "Literature"),
	Sports UMETA(DisplayedName = "Sports"),
	Movies UMETA(DisplayedName = "Movies"),
	ComputerGames UMETA(DisplayedName = "Computer Games"),
	Music UMETA(DisplayedName = "Music"),
	Technologies UMETA(DisplayedName = "Technologies"),
	TwentyFirstCentury UMETA(DisplayedName = "XXI century")
};

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

USTRUCT(Blueprintable)
struct FPlayerChoice
{
	GENERATED_BODY()

	/* BMPlayerID of the player whose choice this is*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choice settings")
	int32 PlayerID = 0;

	/* Index of the Question in UsedQuestion set located in GameMode*/
	UPROPERTY(BlueprintReadWrite, Category="Choice setting")
	int32 QuestionID = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choice settings")
	FTimespan ElapsedTime = FTimespan::Zero();

	FPlayerChoice() {}
	FPlayerChoice(int32 inPlayerID, FTimespan inElapsedTime)
	{
		PlayerID = inPlayerID;
		ElapsedTime = inElapsedTime;
	}

	FORCEINLINE bool operator<(const FPlayerChoice &Other) const
	{
		if(ElapsedTime < Other.ElapsedTime)
		{
			return true;	
		}
		return false;
	}
};

USTRUCT(BlueprintType)
struct FPlayerChoiceChoose: public FPlayerChoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Choose choice")
	int32 AnswerID = -1;

	FPlayerChoiceChoose() {}
	
	FPlayerChoiceChoose(int32 inPlayerID, FTimespan inElapsedTime, int32 inAnswerID)
	{
		FPlayerChoice(inPlayerID, inElapsedTime);
		AnswerID = inAnswerID;
		
	}
	
	FORCEINLINE bool operator<(const FPlayerChoice &Other) const
	{
		if(ElapsedTime < Other.ElapsedTime)
		{
			return true;	
		}
		return false;
	}
};

USTRUCT(BlueprintType)
struct FPlayerChoiceShot: public FPlayerChoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Shot choice")
	int32 Answer = 0;

	UPROPERTY(BlueprintReadWrite, Category="Shot choice")
	int32 Difference = 0;
	
	FPlayerChoiceShot() {}

	FPlayerChoiceShot(int32 inPlayerID, FTimespan inElapsedTime, int32 inAnswer)
	{
		FPlayerChoice(inPlayerID, inElapsedTime);
		Answer = inAnswer;
	}
	
	FORCEINLINE bool operator<(const FPlayerChoiceShot &Other) const
	{
		if(Difference < Other.Difference)
		{
			return true;
		}
		
		if(Difference > Other.Difference)
		{
			return false;
		}
		
		if (Difference == Other.Difference)
		{
			if(ElapsedTime < Other.ElapsedTime)
			{
				return true;
			}
			if (ElapsedTime >= Other.ElapsedTime)
			{
				return false;
			}
		}
		return false;
	}
};

USTRUCT(BlueprintType)
struct FCategory
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Category settings")
	EQuestionCategories Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Category settings")
	FText DisplayableName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Category settings")
	TObjectPtr<UTexture2D> Thumbnail;
};

USTRUCT(Blueprintable, BlueprintType)
struct FQuestion : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	FText Question;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	FCategory Category;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	EQuestionType Type;

	/* Additional multiplicative modifier for this specific question*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	int32 PointsModifier = 1;

	UPROPERTY(BlueprintReadOnly, Category="General Question")
	int32 QuestionID = MAX_int32;

	FORCEINLINE FText GetQuestionText() const { return Question;}
	FORCEINLINE FCategory GetCategory() const { return Category;}
	FORCEINLINE EQuestionType GetType() const { return Type;}
	FORCEINLINE int32 GetPointsModifier() const { return PointsModifier;}
};

USTRUCT(BlueprintType)
struct FQuestionChooseText: public FQuestion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Choose Text Question")
	TArray<FText> Answers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Choose Text Question")
	int32 RightAnswer;
};

USTRUCT(BlueprintType)
struct FQuestionChooseImage: public FQuestion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Choose Image Question")
	TArray<UTexture2D*> Answers;
};

USTRUCT(BlueprintType)
struct FQuestionShot: public FQuestion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot Question")
	int32 Answer;
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

USTRUCT(BlueprintType)
struct FTooltip
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tooltip Settings")
	bool bIsEnabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tooltip Settings")
	TObjectPtr<UTexture2D> TooltipImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tooltip Settings")
	FText Title;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tooltip Settings")
	FText Description;
};

USTRUCT(BlueprintType)
struct FGameModeAvailableSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Settings")
	FString SettingName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Settings")
	TArray<FString> Options;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Settings")
	FTooltip Tooltip;
};

USTRUCT(BlueprintType)
struct FGameModeSetting : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Setting")
	FString SettingName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Setting")
	FString Option;
};

USTRUCT(BlueprintType)
struct FGameModeDescription : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Description")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Description")
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Description")
	TArray<FName> AvailableMaps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Mode Description")
	TArray<FGameModeAvailableSettings> AvailableSettings;
};

USTRUCT(BlueprintType)
struct FPlayerConnectionInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Connection Info")
	bool bIsHost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Connection Info")
	bool bIsReady;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Connection Info")
	int32 PlayerID;
};

USTRUCT(BlueprintType)
struct FPlayerProfile
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Profile Info")
	FString Nickname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Profile Info")
	TObjectPtr<UTexture2D> Avatar;
};

USTRUCT(BlueprintType)
struct FPlayerCastle
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Castle Info")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Castle Info")
	TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Castle Info")
	TObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Castle Info")
	int32 CastleID;
};

USTRUCT(BlueprintType)
struct FPlayerKnight
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Knight Info")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Knight Info")
	TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Knight Info")
	TObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Knight Info")
	int32 KnightID;
};

USTRUCT(BlueprintType)
struct FPlayerFigures
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Figures Info")
	FPlayerCastle Castle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Figures Info")
	FPlayerKnight Knight;
};

USTRUCT(BlueprintType)
struct FConnectedPlayer
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Info")
	FPlayerConnectionInfo ConnectionInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Info")
	FPlayerProfile Profile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Info")
	FPlayerFigures Figures;
};


UCLASS()
class BATTLEMINDS_API UBM_Types : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
