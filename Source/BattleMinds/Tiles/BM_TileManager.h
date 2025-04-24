// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SharedStructs.h"
#include "Core/BM_Types.h"
#include "GameFramework/Actor.h"
#include "Core/BM_Types.h"
#include "BM_TileManager.generated.h"

class ABM_PlayerControllerBase;
class ABM_TileBase;
class ABM_GameStateBase;

DECLARE_MULTICAST_DELEGATE(FOnMapGeneratedNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerCastleDestroyed, int32, PlayerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClickedTileMaterialChanged);
DECLARE_DELEGATE(FOnTileAddedNative);

UCLASS()
class BATTLEMINDS_API ABM_TileManager : public AActor
{
	GENERATED_BODY()

public:
	ABM_TileManager();
	
	FOnMapGeneratedNative OnMapGeneratedNative;

	FOnTileAddedNative OnTileAddedNative;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "TileManager")
	FOnPlayerCastleDestroyed OnPlayerCastleDestroyed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "TileManager")
	FOnClickedTileMaterialChanged OnClickedTileMaterialChanged;

	virtual void Tick(float DeltaTime) override;

	/* Generates map with hexes and call OnMapGenerated delegate*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GenerateMap(int32 NumberOfPlayers);

	UFUNCTION(BlueprintCallable)
	void HandleClickedTile(int32 PlayerID, FIntPoint ClickedTileAxials);

	UFUNCTION(BlueprintCallable)
	void ToggleShowPreviewMeshOnTiles(bool bShow);
	
	UFUNCTION(BlueprintPure)
	int32 GetTileOwnerID(FIntPoint TileAxials) const;

	UFUNCTION(BlueprintPure)
	ABM_TileBase* GetTileByAxials(FIntPoint TileAxials);
	
	UFUNCTION(BlueprintPure, Category = "Tile operations")
	TArray<ABM_TileBase*> GetCurrentPlayerAvailableTiles();

	UFUNCTION(BlueprintPure, Category = "Tile operations")
	TArray<FIntPoint> GetCurrentPlayerAvailableTilesAxials(EGameRound CurrentRound, int32 PlayerIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Tile operations")
	TArray<ABM_TileBase*> GetPlayerOwnedTiles(int32 PlayerID);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Tile operations")
	TArray<FIntPoint> GetPlayerOwnedTilesAxials(int32 PlayerID);
	
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

	/*
	 * Damage could be applied only to CurrentClickedTile by each player
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Tile operations")
	void SC_ApplyDamageToTile(int32 PlayerIndex, int32 InDamage);

	UFUNCTION(BlueprintCallable)
	void GetClickedTileCastleCameraProperties(const FIntPoint& TileAxials, FUniversalCameraPositionSaveFormat& CameraProperties);

	UFUNCTION(BlueprintCallable)
	void GetClickedTilePlayerTurnCameraProperties(const FIntPoint& TileAxials, FUniversalCameraPositionSaveFormat& CameraProperties);

	UFUNCTION(BlueprintCallable)
	void GetClickedTileCastleCameraPropertiesByPlayerId(const int PlayerId, FUniversalCameraPositionSaveFormat& CameraProperties);

	UFUNCTION(BlueprintCallable)
	void GetClickedTilePlayerTurnCameraPropertiesByPlayerId(const int PlayerId, FUniversalCameraPositionSaveFormat& CameraProperties);
	
	UFUNCTION()
	void BindPassTurnToTileCastleMeshSpawned(FIntPoint TileAxials);
	
	void BindGameStateToTileBannerMeshSpawned(FIntPoint TileAxials, FunctionVoidPtr FunctionPointer);

	void BindRoundToTileBannerMeshSpawned(FIntPoint TileAxials, RoundFunctionVoidPtr FunctionPointer);

	UFUNCTION()
	void UnbindAllOnBannerSpawnedDelegates();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Tile operations")
	void AutoAssignCastles(const TMap<int32, EColor>& PlayerColors);
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
	EQuestionType GetNextQuestionTypeOfClickedTile() const;

	UFUNCTION(BlueprintPure)
	int32 GetTileQuestionsCount(FIntPoint TileAxials) const;

	/* Get Tile AnnexedRound and found corresponding value in TilePoints map of BM_GameModeBase*/
	UFUNCTION(BlueprintPure)
	int32 GetPointsOfTile(FIntPoint TileAxials) const;

	/* Get Tile clicked by a player with PlayerIndex and call on it GetPointsOfTile(Axials)*/
	UFUNCTION(BlueprintPure)
	int32 GetPointsOfCurrentClickedTile(int32 PlayerIndex);

	UFUNCTION(BlueprintPure)
	ETileStatus GetStatusOfCurrentClickedTile(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable)
	void SwitchToNextRound(EGameRound NewRound);
	
	UFUNCTION(BlueprintPure)
	bool IsTileAvailable(FIntPoint TileAxials) const;

	UFUNCTION(BlueprintPure)
	TMap<int32, ABM_TileBase*> GetClickedTiles();

	UFUNCTION(BlueprintPure)
	FORCEINLINE ABM_TileBase* GetFirstAvailableTile() { return FirstAvailableTile; }

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Tile operations")
	void SC_SetCastleRotationToCenter(ABM_TileBase* TileToRotate);

	UFUNCTION(BlueprintPure)
	ABM_TileBase* GetFirstAvailableForPlayerTile();

	UFUNCTION(BlueprintPure)
	ABM_TileBase* GetTileFromClientsMap(FIntPoint TileAxials);

	UFUNCTION(BlueprintPure)
	FORCEINLINE FIntPoint GetFirstAvailableTileAxials() { return FirstAvailableTileAxials; };

	UFUNCTION(Server, Unreliable, Category = "Tile operations")
	void SC_ResetFirstAvailableTile();

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

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Tiles")
	TMap<FIntPoint, ABM_TileBase*> ClientTilesMap;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Tiles")
	ABM_TileBase* FirstAvailableTile = nullptr;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Tiles")
	FIntPoint FirstAvailableTileAxials;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile operations")
	TArray<FIntPoint> CurrentPlayerAvailableTilesAxials;
	
	UFUNCTION(BlueprintCallable)
	void HandlePostQuestionPhase(FPostQuestionPhaseInfo PostQuestionPhaseInfo);

	UFUNCTION()
	void PostQuestionPhaseSetTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);

	UFUNCTION()
	void PostQuestionPhaseFightForTheRestTiles(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);

	UFUNCTION()
	void PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo);
	
	UFUNCTION(BlueprintCallable)
	void SwitchTilesMaterial();

	UFUNCTION(BlueprintCallable)
	void SC_TransferTerritory();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tile operations")
	void OnCastleDestroyed(int32 OwnerPlayerIndex);

	UFUNCTION(NetMulticast, Unreliable, Category = "Tiles")
	void MC_OnMapGenerated();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION()
	void CheckForTilePostQuestionHandled();
	
	TPair<int32, EColor> GetCurrentPlayerInfo();

	UPROPERTY()
	TArray<FIntPoint> TilesToSwitchMaterial;

	UPROPERTY()
	int32 CurrentTilesToHandlePostQuestion = 0;

	UPROPERTY()
	int32 ExpectedTilesToHandlePostQuestion = 0;

	UPROPERTY()
	int32 CastlePreviousOwnerIndex = -1;

	UPROPERTY()
	bool bCastleCaptured = false;
};