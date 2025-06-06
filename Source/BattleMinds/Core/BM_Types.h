// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "BM_Types.generated.h"

class ABM_TileBase;

UENUM(BlueprintType)
enum class EGameLength : uint8
{
	Short,
	Long,
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETilePostQuestionOperation : uint8
{
	None = 0, 
	TileMesh = 1 << 0,
	BannerMesh = 1 << 1,
	CastleMesh = 1 << 2
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAnsweredPlayer : uint8
{
	NoCorrectAnswer = 0, 
	AttackingPlayer = 1 << 0,
	DefendingPlayer = 1 << 1
};

UENUM(BlueprintType)
enum class EQuestionResult : uint8
{
	ShotQuestionNeeded,
	NobodyAnswered,
	WrongAnswer,
	TileDefended,
	TileDamaged,
	TileCaptured
};

UENUM(BlueprintType)
enum class EQuestionUsableItem : uint8
{
	Bombs,
	Ice,
	ChopLetters,
	FiftyFifty,
	Clamp,
	DenyDebuff,
	SecondChance
};

UENUM(BlueprintType)
enum class EGameRound : uint8
{
	BuildCastle,
	SetTerritory,
	FightForTheRemains,
	FightForTerritory,
	End
};

UENUM(BlueprintType)
enum class EColor : uint8
{
	Undefined,
	Red,
	Green,
	Blue,
	Pink,
	Orange,
	White,
	Black,
	Purple,
	Brown,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FPostQuestionPhaseInfo
{
	GENERATED_BODY()

	/*
	 * Key - PlayerIndex
	 * Value - Player Question result
	 */
	UPROPERTY(BlueprintReadOnly)
	TMap<int32, EQuestionResult> QuestionResultsPerPlayer;

	/*
	 * Key - PlayerIndex
	 * Value - Tile that was clicked by the Player
	 */
	UPROPERTY(BlueprintReadOnly)
	TMap<int32, ABM_TileBase*> PlayerClickedTiles;
	
	UPROPERTY(BlueprintReadOnly)
	TMap<int32, EColor> PlayerColors;

	UPROPERTY(BlueprintReadOnly)
	EGameRound GameRound = EGameRound::End;

	FPostQuestionPhaseInfo(){};
	explicit FPostQuestionPhaseInfo(const TMap<int32, EQuestionResult>& InQuestionResultsPerPlayer,
		const TMap<int32,EColor>& InPlayerColors, const EGameRound& InGameRound,
		const TMap<int32, ABM_TileBase*>& InPlayerClickedTiles)
	{
		QuestionResultsPerPlayer = InQuestionResultsPerPlayer;
		PlayerColors = InPlayerColors;
		GameRound = InGameRound;
		PlayerClickedTiles = InPlayerClickedTiles;
	}

	bool ContainsResultType(EQuestionResult ResultTypeToCheck) const;
};

USTRUCT(BlueprintType)
struct FPrePlayerTurnPhaseInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EGameRound GameRound = EGameRound::End;

	FPrePlayerTurnPhaseInfo() {}
	FPrePlayerTurnPhaseInfo(const EGameRound& InGameRound) : GameRound(InGameRound) {}
};

UENUM(BlueprintType)
enum class EQuestionCategories : uint8
{
	Science UMETA(DisplayedName = "Science"), //
	History UMETA(DisplayedName = "History"),
	Literature UMETA(DisplayedName = "Literature"), //
	Sports UMETA(DisplayedName = "Sports"), //
	Movies UMETA(DisplayedName = "Movies"), //
	Videogames UMETA(DisplayedName = "Video Games"), //
	Music UMETA(DisplayedName = "Music"), //
	Technologies UMETA(DisplayedName = "Technologies"),
	Geography UMETA(DisplayedName = "Geography") //
};

UENUM(BlueprintType)
enum class EAnswerType : uint8
{
	ChooseText UMETA(DisplayName = "Choose Text"),
	Shot UMETA(DisplayName = "Shot")
};

UENUM(BlueprintType)
enum class EQuestionType : uint8
{
	Undefined, 
	Choose UMETA(DisplayName = "Choose Answer"),
	Shot UMETA(DisplayName = "Shot")
};

UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	NotOwned UMETA(DisplayName = "Not Owned"),
	Controlled UMETA(DisplayName = "Controlled"),
	Castle UMETA(DisplayName = "Castle"),
	Attacked UMETA(DisplayName = "Attacked")
};

USTRUCT(BlueprintType)
struct FTileQuestionCount
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EQuestionType QuestionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Count;
};


/*
 * Struct representing Answers in UI
 */
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


/*
 * Base struct representing Player choice in Question UI
 * This struct is created and populated when a Player presses the button in Choose Questions
 * or enters the number in Shot Question
 */
USTRUCT(Blueprintable)
struct FPlayerChoice
{
	GENERATED_BODY()

	/* BMPlayerID of the player whose choice this is*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choice settings")
	int32 PlayerID = 0;

	/* BMPlayerID of the player whose choice this is*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choice settings")
	FUniqueNetIdRepl PlayerUniqueNetId;

	/* Index of the Question in UsedQuestion set located in GameMode*/
	UPROPERTY(BlueprintReadWrite, Category="Choice setting")
	int32 QuestionID = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choice settings")
	FTimespan ElapsedTime = FTimespan::Zero();

	FPlayerChoice() {}
	FPlayerChoice(int32 inPlayerID, const FUniqueNetIdRepl& inPlayerUniqueNetId, FTimespan inElapsedTime)
	{
		PlayerID = inPlayerID;
		PlayerUniqueNetId = inPlayerUniqueNetId;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Choose choice")
	int32 AnswerID = -1;

	FPlayerChoiceChoose() {}
	
	FPlayerChoiceChoose(int32 inPlayerID, const FUniqueNetIdRepl& inPlayerUniqueNetId, FTimespan inElapsedTime, int32 inAnswerID)
	{
		FPlayerChoice(inPlayerID, inPlayerUniqueNetId, inElapsedTime);
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shot choice")
	int32 Answer = MAX_int32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shot choice")
	int32 Difference = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shot choice")
	bool bAnswered = false;
	
	FPlayerChoiceShot() {}

	FPlayerChoiceShot(int32 inPlayerID, const FUniqueNetIdRepl& inPlayerUniqueNetId, FTimespan inElapsedTime, int32 inAnswer)
	{
		FPlayerChoice(inPlayerID, inPlayerUniqueNetId, inElapsedTime);
		Answer = inAnswer;
	}
	FPlayerChoiceShot(const FPlayerChoiceShot& Other) : FPlayerChoice(Other), Answer(Other.Answer),
	Difference(Other.Difference), bAnswered(Other.bAnswered) {}
	
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

	bool operator==(const FPlayerChoiceShot &Other) const
	{
		return Equals(Other);
	}
	bool Equals(const FPlayerChoiceShot& Other) const;
};
#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FPlayerChoiceShot& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FPlayerChoiceShot& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FPlayerChoiceShot));
	return Hash;
}
#endif

/*
 * UI only struct for Question Category info
 */
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

/*
 * Base struct for Question
 * used as instanced struct for different types of Question
 */
USTRUCT(Blueprintable, BlueprintType)
struct FQuestion : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	FText Question;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	EQuestionCategories Category;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	EQuestionType Type;

	/* Additional multiplicative modifier for this specific question*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General Question")
	int32 PointsModifier = 1;

	UPROPERTY(BlueprintReadOnly, Category="General Question")
	int32 QuestionID = MAX_int32;

	FORCEINLINE FText GetQuestionText() const { return Question;}
	FORCEINLINE EQuestionCategories GetCategory() const { return Category;}
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

/*
 * Struct for collecting data about Player answers
 * Used in Results UI in the end of the game
 */

USTRUCT(BlueprintType)
struct FQuestionResult
{
	GENERATED_BODY()

	/*
	 * 1) Question number
	 * 2) Question category
	 * 3) Question text
	 * 4) Player's answer
	 * 5) Other players answers
	 * 6) Correct answer
	 * 7) received points
	 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuestionNumber = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BaseStruct = "Question"))
	FInstancedStruct Question;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BaseStruct = "PlayerChoice"))
	TArray<FInstancedStruct> PlayerChoices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasAnswered = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReceivedPoints = 0;
};

USTRUCT(BlueprintType)
struct FTileMaterials
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EColor Color;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> CastleMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> BannerMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> TileMeshMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> TileBorderMeshMaterial;
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
	FUniqueNetIdRepl UniqueNetIdRepl;
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
	int32 CastleID = 0;
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
	int32 KnightID = 0;
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

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Nickname;

	UPROPERTY(BlueprintReadOnly)
	EColor Color = EColor::Undefined;

	/* Index in PlayerArray of GameState*/
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerID = -1;
	
	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl UniqueNetIdRepl;
};

UENUM(BlueprintType)
enum class EAvatarSize : uint8
{
	Small,
	Medium,
	Large
};

USTRUCT(BlueprintType)
struct FPlayerAvatars
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Avatars")
	UTexture2D* AvatarSmall = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Avatars")
	UTexture2D* AvatarMedium = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Avatars")
	UTexture2D* AvatarLarge = nullptr;
};

USTRUCT(BlueprintType)
struct FPermutation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Permutation Info")
	TArray<int32> Values;
};

USTRUCT(BlueprintType)
struct FPermutationUIValue
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="Permutation UI Info")
	FUniqueNetIdRepl UniqueNetId;

	UPROPERTY(BlueprintReadWrite, Category="Permutation UI Info")
	EColor Color;
	
	FPermutationUIValue() : UniqueNetId(FUniqueNetIdRepl()), Color(EColor::Black) {};
	
	FPermutationUIValue(const FUniqueNetIdRepl& InUniqueNetId, EColor InColor)
	{
		UniqueNetId = InUniqueNetId;
		Color = InColor;
	}
};

USTRUCT(BlueprintType)
struct FPermutationUI
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Permutation Info")
	TArray<FPermutationUIValue> Values;
};

USTRUCT(BlueprintType)
struct FPlayersCycle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	int32 CycleNumber = 0;
	
	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	FPermutation PlayersPermutation;

	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	bool bIsCompleted = false;

	FPlayersCycle(){}
	FPlayersCycle(const int32 InCycleNumber, const FPermutation& InPermutation, const bool InIsCompleted)
	{
		CycleNumber = InCycleNumber;
		PlayersPermutation = InPermutation;
		bIsCompleted = InIsCompleted;
	}
};

USTRUCT(BlueprintType)
struct FPlayersCycleUI
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	int32 CycleNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	FPermutationUI PlayersPermutation;

	UPROPERTY(BlueprintReadWrite, Category="Player Cycle Info")
	FLinearColor BorderColor = FLinearColor::Black;

	FPlayersCycleUI() {};
	FPlayersCycleUI(const int32 InCycleNumber, const FPermutationUI& InPermutation)
	{
		CycleNumber = InCycleNumber;
		PlayersPermutation = InPermutation;
	}
};

USTRUCT()
struct FTileBaseItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	ABM_TileBase* TileBase = nullptr;
	
	FTileBaseItem() {}
	FTileBaseItem(ABM_TileBase* InTile) : TileBase(InTile) {}
};

USTRUCT()
struct FTileBaseArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTileBaseItem> Items;

	// Владелец (обязательно, если реплицировать в Actor)
	class UObject* Owner = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FTileBaseItem, FTileBaseArray>(Items, DeltaParams, *this);
	}

	// Вызов при добавлении нового элемента
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
	{
		UE_LOG(LogTemp, Log, TEXT("FTileBaseItem Added"));
	}

	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
	{
		UE_LOG(LogTemp, Log, TEXT("FTileBaseItem Changed"));
	}

	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("FTileBaseItem Removed %d elements"), RemovedIndices.Num());
	}
};

template<>
struct TStructOpsTypeTraits<FTileBaseArray> : public TStructOpsTypeTraitsBase2<FTileBaseArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

class ABM_GameStateBase;
class UGameRound;
typedef void (ABM_GameStateBase::*FunctionVoidPtr)(void);
typedef void (UGameRound::*RoundFunctionVoidPtr)(void);

UCLASS()
class BATTLEMINDS_API UBM_Types : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static EColor GetColorStringAsEnum(FString InColor);

	/* How many rounds on N players should be in the game*/
	UFUNCTION(BlueprintPure)
	static int32 GetGameLengthAsInt(EGameLength GameLength);

	UFUNCTION(BlueprintCallable, Category = "Permutations")
	static TArray<FPermutation> GenerateAllPermutations(TArray<int32>& ElementsOfPermutations);

	UFUNCTION(BlueprintCallable, Category = "Permutations")
	static TArray<FPermutation> GenerateNumberOfPermutations(TArray<int32>& ElementsOfPermutations, int32 NumberOfPermutations);

private:

	UFUNCTION()
	static void Backtrack(TArray<int32>& Current, TArray<int32>& Elements, TArray<FPermutation>& Results);
};
