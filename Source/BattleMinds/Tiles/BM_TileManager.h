﻿// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/Actor.h"
#include "Core/BM_Types.h"
#include "BM_TileManager.generated.h"

class ABM_PlayerControllerBase;
class ABM_TileBase;
class ABM_GameStateBase;

DECLARE_MULTICAST_DELEGATE(FOnMapGeneratedNative);

UCLASS()
class BATTLEMINDS_API ABM_TileManager : public AActor
{
	GENERATED_BODY()

public:
	ABM_TileManager();
	
	FOnMapGeneratedNative OnMapGeneratedNative;
	
	virtual void Tick(float DeltaTime) override;

	/* Generates map with hexes and call OnMapGenerated delegate*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GenerateMap(int32 NumberOfPlayers);

	UFUNCTION(BlueprintCallable)
	void HandleClickedTile(int32 PlayerID, FIntPoint ClickedTileAxials);

	UFUNCTION(BlueprintPure)
	int32 GetTileOwnerID(FIntPoint TileAxials) const;
	
	UFUNCTION(BlueprintPure, Category = "Tile operations")
	TArray<ABM_TileBase*> GetCurrentPlayerAvailableTiles(int32 PlayerID);

	UFUNCTION(BlueprintPure, Category = "Tile operations")
	TArray<FIntPoint> GetCurrentPlayerAvailableTilesAxials(EGameRound CurrentRound, int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Tile operations")
	void UnhighlightTiles();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tile operations")
	TArray<FIntPoint> GetUncontrolledTiles();

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetTilesCount() const { return Tiles.Num(); }

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile visuals")
	void SC_HighlightAvailableTiles(EGameRound GameRound, int32 PlayerID, EColor InPlayerColor);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_SetTileOwner(FIntPoint TileAxials, ETileStatus NewStatus, int32 NewOwnerID, EColor NewOwnerColor, EGameRound CurrentGameRound);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_AttackTile(FIntPoint TileAxials, EColor InPlayerColor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_AddClickedTileToTheTerritory(int32 PlayerID, ETileStatus NewStatus, EColor NewColor, EGameRound CurrentGameRound);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_CancelAttackOnClickedTile(int32 PlayerID);
	
	UFUNCTION()
	void BindPassTurnToTileCastleMeshSpawned(FIntPoint TileAxials);
	
	void BindGameStateToTileBannerMeshSpawned(FIntPoint TileAxials, FunctionVoidPtr FunctionPointer);

	UFUNCTION()
	void UnbindAllOnBannerSpawnedDelegates();

	/*
	 * Randomly assign Tiles to Number of players
	 * bool bAssignCastles : true - Manager will assign castles randomly,
	 * false - Castles should be chosen by Players previously
	 * PlayerColors - map of <PlayerID,EColor> to properly set corresponding properties of BM_Tile
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Tile operations")
	void AutoAssignTerritory(const bool bAssignCastles, const TMap<int32, EColor>& PlayerColors);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Tile operations")
	void AutoAssignTerritoryWithEmptyTiles(const bool bAssignCastles, const TMap<int32, EColor>& PlayerColors, const int32 AmountOfEmptyTiles);

	UFUNCTION(BlueprintPure)
	EGameRound GetTileAnnexedRound(FIntPoint TileAxial) const;

	UFUNCTION(BlueprintPure)
	int32 GetTileQuestionsCount(FIntPoint TileAxials) const;

	/* Get Tile AnnexedRound and found corresponding value in TilePoints map of BM_GameModeBase*/
	UFUNCTION(BlueprintPure)
	int32 GetPointsOfTile(FIntPoint TileAxials) const;

	/* Get Tile clicked by a player with PlayerIndex and call on it GetPointsOfTile(Axials)*/
	UFUNCTION(BlueprintPure)
	int32 GetPointsOfCurrentClickedTile(int32 PlayerIndex);

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Tile operations")
	void HighlightTilesForPlayer(TArray<FIntPoint> TilesToHighlight, EColor InPlayerColor, EGameRound CurrentRound);

	UFUNCTION(BlueprintPure, Category = "Tile operations")
	EQuestionType GetTileNextQuestionType(FIntPoint TileAxials);

	UFUNCTION(BlueprintPure, Category = "Tile operations")
	TArray<FIntPoint> GetShapeBorder(TArray<FIntPoint> Shape);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tiles")
	TMap<FIntPoint, ABM_TileBase*> Tiles;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tile operations")
	TArray<FIntPoint> GetTilesAvailableForCastle();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tile operations")
	TArray<FIntPoint> GetTilesAvailableForExpansion(int32 PlayerID);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tile operations")
	TArray<FIntPoint> GetTilesAvailableForAttack(int32 PlayerID);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile operations")
	TObjectPtr<ABM_TileBase> CurrentClickedTile;

	/*
	 * Tiles that has been clicked
	 * Key - PlayerID
	 * Value - Tile axials
	 */
	UPROPERTY()
	TMap<int32, FIntPoint> ClickedTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile operations")
	TArray<FIntPoint> CurrentPlayerAvailableTiles;
};