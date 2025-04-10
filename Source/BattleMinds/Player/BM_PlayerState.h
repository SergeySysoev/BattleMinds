// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BM_PlayerStateBase.h"
#include "Core/BM_Types.h"
#include "Tiles/BM_TileBase.h"
#include "BM_PlayerState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerState, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPointsChanged, int32, PlayerID, float, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleConquered);

class ABM_PlayerControllerBase;

UCLASS()
class BATTLEMINDS_API ABM_PlayerState : public ABM_PlayerStateBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Questions")
	bool CurrentQuestionAnswerSent = false;

	/* Based on how many points Player received, calculated in GameState */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Questions")
	int32 TotalPlace;

	/* bool to check if the Player clicked on any tile during his turn*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Stats")
	bool bHasChosenTile;

	/* Array of Player choices*/
	// TODO: rewrite using FFastArraySerializer?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> QuestionChoices;

	/* Array of Player Question Results*/
	// TODO: rewrite using FFastArraySerializer?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	TArray<FQuestionResult> QuestionResults;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleConquered OnCastleConquered;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPointsChanged OnPointsChanged;
	
	UFUNCTION(BlueprintPure)
	float GetPoints() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetPlayerIndex() const { return BMPlayerIndex; }

	UFUNCTION(Server, Reliable)
	void SC_SetPlayerIndex(int32 NewPlayerIndex);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_ChangePoints(int32 IncrementPoints);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AddTileToTerritory();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RemoveTileFromTerritory(ABM_TileBase* InTile);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerTurn();
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerTurn(bool inTurn);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasArtillery();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetCorrectAnswersNumber();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWrongAnswersNumber();
	
	UFUNCTION()
	void OnRep_Points();
	
	UFUNCTION()
	TSet<ABM_TileBase*> GetNeighbors();
	
protected:

	virtual void BeginPlay() override;

	/* Player index in PlayerArray of GameState*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Replicated, Category= "Player Info")
	int32 BMPlayerIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Points, Category = "Player Info")
	float Points;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasArtillery;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasTurn;

	virtual void PlayerColorChanged() override;
	
};
