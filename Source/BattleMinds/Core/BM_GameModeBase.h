// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "Player/BM_PlayerPawn.h"
#include "Tiles/BM_TileBase.h"
#include "InstancedStruct.h"
#include "Data/DA_QuestionTables.h"
#include "BM_GameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_GameMode, Display, All);

/*
 *	General game loop:
 *	1) Choose game round: Set Castle, Set Territory, Fight For Territory, Count Results
 *	2) based on the round, assign Player's turn
 *	3) when all Players have made their turn, open a Question
 *	4) wait until all Players submit their choices or until timer ends
 *	5) collect all answers
 *	6) verify collected answers
 *	7) assign points and territory based on the answers
 *	8) repeat until all Players have no more turns
 *	9) count results and assign places
 */
UCLASS()
class BATTLEMINDS_API ABM_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	ACameraActor* GetQuestionCamera(EQuestionType QuestionType) const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetQuestionTimer() const { return QuestionTimer;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetTurnTimer() const { return TurnTimer;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumberOfActivePlayers() const { return NumberOfActivePlayers;}

	UFUNCTION(BlueprintPure)
	UDataTable* GetQuestionsDataTable(EQuestionType QuestionType) const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetPointsOfTile(const EGameRound TileStatus) const { return TilePoints.FindRef(TileStatus); }
	
protected:

	ABM_GameModeBase();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void BeginPlay() override;
	
	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float QuestionTimer = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float TurnTimer = 10.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game settings")
	int32 MaxNumberOfActivePlayers = 3;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfActivePlayers = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialAttackMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> CastleMaterialMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, UMaterialInterface*> MaterialNeighborMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, FColor> ColorMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players settings")
	TMap<int32, FString> NicknameMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game settings | Questions")
	TObjectPtr<UDA_QuestionTables> ChooseTables;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game settings | Questions")
	TObjectPtr<UDA_QuestionTables> ShotTables;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Game settings | Questions")
	TArray<UDataTable*> QuestionTablesChoose;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Game settings | Questions")
	TArray<UDataTable*> QuestionTablesShot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Players settings")
	TSubclassOf<ABM_PlayerPawn> PawnClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ChooseQuestionCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ShotQuestionCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game settings")
	EGameLength SelectedGameLength = EGameLength::Long;

	/*
	 * How many points will be given to Player if he adds the tile with the specified status
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game settings | Tile Points")
	TMap<EGameRound, int32> TilePoints;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitPlayer(APlayerController* NewPlayer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitGameSettings();
};
