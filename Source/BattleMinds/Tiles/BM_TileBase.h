// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMinds/Player/BM_PlayerControllerBase.h"
#include "Components/BoxComponent.h"
#include "Core/BM_Types.h"
#include "Net/UnrealNetwork.h"
#include "BM_TileBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_Tile, Display, All);

class UStaticMeshComponent;
class ABM_PlayerControllerBase;
class ABM_PlayerState;

UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	NotOwned UMETA(DisplayName = "Not Owned"),
	Controlled UMETA(DisplayName = "Controlled"),
	Castle UMETA(DisplayName = "Castle")
};

UCLASS()
class BATTLEMINDS_API ABM_TileBase : public AActor
{
	GENERATED_BODY()

public:
	ABM_TileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category="Status")
	bool bIsAttacked = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Neighbours")
	TArray<TObjectPtr<ABM_TileBase>> NeighbourTiles;
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ChangeStatus(ETileStatus NewStatus);
	
	UFUNCTION()
	void OnRep_TileMeshChanged();
	UFUNCTION()
	void OnRep_FlagMeshChanged();
	UFUNCTION()
	void OnRep_CastleMeshChanged();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TileWasChosen(ABM_PlayerState* PlayerState, EGameRound GameRound);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TileWasClicked(FKey ButtonPressed, EGameRound GameRound, ABM_PlayerState* PlayerState);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void AddTileToPlayerTerritory(ABM_PlayerState* PlayerState);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RemoveTileFromPlayerTerritory(ABM_PlayerState* PlayerState);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void DecreaseCastleHP();
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void CancelAttack();
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MC_RemoveHighlighting();
	
	UFUNCTION(BlueprintCallable)
	FString GetOwningPlayerNickname(){ return OwnerPlayerNickname; };
	
	UFUNCTION(BlueprintCallable)
	int32 GetOwningPlayerID(){return OwnerPlayerID;};
	
	UFUNCTION(BlueprintCallable)
	ETileStatus GetStatus() { return Status; };

	UFUNCTION(BlueprintCallable)
	float GetPoints() {return Points;};

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="Visuals")
	void MC_RemoveSelection();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Visuals")
	void TurnOnHighlight(UMaterialInterface* NeighborMaterial);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Visuals")
	void TurnOffHighlight();
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="Visuals")
	void MC_ShowEdges(bool bVisibility, FColor PlayerColor);


protected:
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_FlagMeshChanged, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* FlagMesh;
    UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_CastleMeshChanged, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* CastleMesh;
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_CastleMeshChanged, BlueprintReadWrite, Category = "Components")
	UBoxComponent* EdgesBox;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Points")
	float Points = 200.0f;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	ETileStatus Status;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	bool bIsArtillery;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	bool bIsFortified;
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	int32 TileHP = 1;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> CurrentMaterial;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_FlagMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialAttacked;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialOwned;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CastleMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialCastle;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
	FString OwnerPlayerNickname;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
	int32 OwnerPlayerID;

	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
