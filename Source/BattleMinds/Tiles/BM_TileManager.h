// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/Actor.h"
#include "Core/BM_Types.h"
#include "BM_TileManager.generated.h"

class ABM_PlayerControllerBase;
class ABM_TileBase;
class ABM_GameStateBase;

DECLARE_DELEGATE(FOnMapGenerated);

UCLASS()
class BATTLEMINDS_API ABM_TileManager : public AActor
{
	GENERATED_BODY()

public:
	ABM_TileManager();
	
	FOnMapGenerated OnMapGenerated;
	
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
	TArray<FIntPoint> GetCurrentPlayerAvailableTilesAxials(int32 PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Tile operations")
	void UnhighlightTiles();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tile operations")
	TArray<FIntPoint> GetUncontrolledTiles();

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetTilesCount() const { return Tiles.Num(); }

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile visuals")
	void SC_HighlightAvailableTiles(EGameRound GameRound, int32 PlayerID, EColor InPlayerColor);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_SetTileOwner(FIntPoint TileAxials, ETileStatus NewStatus, int32 NewOwnerID, EColor NewOwnerColor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_AttackTile(FIntPoint TileAxials, EColor InPlayerColor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_AddClickedTileToTheTerritory(int32 PlayerID, ETileStatus NewStatus, EColor NewColor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_CancelAttackOnClickedTile(int32 PlayerID);
	
	UFUNCTION()
	void BindGameStateToTileCastleMeshSpawned(FIntPoint TileAxials);
	
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

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Tile operations")
	void HighlightTilesForPlayer(TArray<FIntPoint> TilesToHighlight, EColor InPlayerColor);

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