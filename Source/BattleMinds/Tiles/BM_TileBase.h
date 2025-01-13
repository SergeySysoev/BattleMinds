// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Core/BM_Types.h"
#include "BM_TileBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_Tile, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleMeshSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBannerMeshSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleDestroyed);

class UStaticMeshComponent;
class ABM_PlayerControllerBase;
class ABM_PlayerState;

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

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleMeshSpawned OnCastleMeshSpawned;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnBannerMeshSpawned OnBannerMeshSpawned;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleDestroyed OnCastleDestroyed;
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ChangeStatus(ETileStatus NewStatus);
	
	UFUNCTION()
	void OnRep_TileMeshChanged();
	UFUNCTION()
	void OnRep_FlagMeshChanged();
	UFUNCTION()
	void OnRep_CastleMeshChanged();
	UFUNCTION()
	void OnRep_AttackingColor();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_SiegeTile(EColor InPlayerColor);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_SetDisputedAppearance();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RestoreCastleHP();
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_DecreaseCastleHP();
	
	UFUNCTION(BlueprintCallable)
	FString GetOwningPlayerNickname(){ return OwnerPlayerNickname; };
	
	UFUNCTION(BlueprintCallable)
	int32 GetOwningPlayerID(){return OwnerPlayerID;};
	
	UFUNCTION(BlueprintCallable)
	ETileStatus GetStatus() { return Status; };

	UFUNCTION(BlueprintCallable)
	float GetPoints() {return Points;};

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetTileQuestionCount() const { return CurrentQuestionArray.Num(); };

	UFUNCTION(BlueprintPure)
	EQuestionType GetTileNextQuestionType() const;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="Visuals")
	void MC_RemoveSelection();
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="Visuals")
	void MC_ShowEdges(bool bVisibility, EColor InPlayerColor = EColor::Black);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Visuals")
	void SC_SetAttackingColorForTileEdges(EColor AttackingPlayerColor);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void CancelAttack();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void AddTileToPlayerTerritory(ETileStatus InStatus, int32 InPlayerID, const FString& InPlayerNickname, EColor InPlayerColor);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RemoveTileFromPlayerTerritory();

	UFUNCTION(BlueprintCallable)
	void SetInGameTTileMaterials(TMap<EColor, FTileMaterials> InGameMaterials);

	UFUNCTION(BlueprintNativeEvent)
	void SpawnCastleMesh();

	UFUNCTION(BlueprintNativeEvent)
	void SpawnBannerMesh();

	UFUNCTION(BlueprintCallable)
	void SC_ApplyDamage(int32 InDamageAmount);
	
protected:
	
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh = nullptr;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_FlagMeshChanged, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* BannerMesh = nullptr;
	
    UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_CastleMeshChanged, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* CastleMesh = nullptr;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Components")
	UBoxComponent* EdgesBox = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadWrite, Category = "Points")
	float Points = 200.0f;

	UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadWrite, Category = "Points")
	float SuccessfulDefencePoints = 100.f;

	UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadWrite, Category = "Points")
	float ConqueredPoints = 400.f;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	ETileStatus Status = ETileStatus::NotOwned;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	bool bIsArtillery;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	bool bIsFortified;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	int32 TileHP = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Territory")
	TArray<EQuestionType> CurrentQuestionArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Territory")
	TArray<EQuestionType> DefaultQuestionArray;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> TileMeshMaterial;
	
	UPROPERTY(ReplicatedUsing = OnRep_FlagMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialAttacked;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_FlagMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> DisputedTerritoryMaterial;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialOwned;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CastleMeshChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialCastle;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AttackingColor, BlueprintReadWrite, Category = "Visuals")
	FColor AttackingColor = FColor::Black;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
	FString OwnerPlayerNickname;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
	int32 OwnerPlayerID;

	UPROPERTY()
	TMap<EColor, FTileMaterials> TileMaterials;

	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
