// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/BM_Types.h"
#include "TimerManager.h"
#include "Player/BM_PlayerPawn.h"
#include "Tiles/BM_TileBase.h"
#include "InstancedStruct.h"
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
	FORCEINLINE int32 GetNumberOfPlayerTurns() const { return NumberOfPlayerTurns;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumberOfActivePlayers() const { return NumberOfActivePlayers;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumberOfTotalTurns() const { return NumberOfTotalTurns;}

	UFUNCTION(BlueprintPure)
	UDataTable* GetQuestionsDataTable(EQuestionType QuestionType) const;
	
protected:
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float QuestionTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	float TurnTimer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfActivePlayers = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfPlayerTurns = 6;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game settings")
	int32 NumberOfTerritoryTurns = 3;
	
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
	TArray<UDataTable*> QuestionTablesChoose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game settings | Questions")
	TArray<UDataTable*> QuestionTablesShot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Players settings")
	TSubclassOf<ABM_PlayerPawn> PawnClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ChooseQuestionCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game settings | Cameras")
	TObjectPtr<ACameraActor> ShotQuestionCamera;

	UPROPERTY()
	int32 NumberOfTotalTurns = 0;
	
	UFUNCTION(BlueprintCallable)
	void InitPlayer(APlayerController* NewPlayer);
};
