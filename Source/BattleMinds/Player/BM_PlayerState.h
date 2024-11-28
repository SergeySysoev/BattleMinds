// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Core/BM_Types.h"
#include "Tiles/BM_TileBase.h"
#include "BM_PlayerState.generated.h"

class ABM_PlayerControllerBase;

UCLASS()
class BATTLEMINDS_API ABM_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/* Player index*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Replicated, Category= "Player Info")
	int32 BMPlayerID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	FString Nickname;

	/* Material used to apply to Player's tiles meshes*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
	UMaterialInterface* MaterialTile;

	/* Material used to apply to Player's castle meshe*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
	UMaterialInterface* MaterialCastle;

	/* Material used to apply to map mesh Player is attacking*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
	UMaterialInterface* MaterialAttack;

	/* Material used to apply to neighbours of the Player's tiles*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
	UMaterialInterface* MaterialNeighbour;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	FColor PlayerColor;
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Questions")
	bool CurrentQuestionAnswerSent;

	/* Number of turns this player will make in the game, calculated based on number of players in lobby*/
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Questions")
	int32 NumberOfTurns;
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Questions")
	int32 WinnerPosition;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Stats")
	TArray<ABM_TileBase*> OwnedTiles;

	/* bool to check if the Player clicked on any tile during his turn*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Stats")
	bool bHasChosenTile;

	/* Array of Player choices*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info", meta=(BaseStruct="PlayerChoice"))
	TArray<FInstancedStruct> QuestionChoices;

	/* Array of Player Question Results*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	TArray<FQuestionResult> QuestionResults;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetPoints() const;
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void AddPoints(int32 inPoints);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AddTileToTerritory(ABM_TileBase* InTile, ETileStatus InTileStatus);

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
	
	UFUNCTION(BlueprintNativeEvent)
	void SetPointsInWidget();
	
	UFUNCTION()
	TSet<ABM_TileBase*> GetNeighbors();
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=SetPointsInWidget, Category = "Player Info")
	float Points;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasArtillery;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	bool bHasTurn;
	
};
