// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/BM_Types.h"
#include "BM_TileBase.generated.h"

class USpringArmComponent;
DECLARE_LOG_CATEGORY_EXTERN(LogBM_Tile, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleMeshSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBannerMeshSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCastleDestroyed, int32, OwnerPlayerIndex);
DECLARE_MULTICAST_DELEGATE(FOnBannerMeshSpawnedNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTileMaterialSwitched);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleTowerDamaged);

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

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleMeshSpawned OnCastleMeshSpawned;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnBannerMeshSpawned OnBannerMeshSpawned;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleDestroyed OnCastleDestroyed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCastleTowerDamaged OnCastleTowerDamaged;
	
	FOnBannerMeshSpawnedNative OnBannerMeshSpawnedNative;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTileMaterialSwitched OnTileMaterialSwitched;
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_ChangeStatus(ETileStatus NewStatus);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RevertStatus();

	UFUNCTION(BlueprintCallable)
	void SetTileEdgesColor(EColor NewColor);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPointsWidgetValue(int32 Points);

	UFUNCTION(BlueprintCallable)
	void SetPointsWidgetVisibility(bool bIsVisible);

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category="Visuals")
	void MC_SetPointsWidgetVisibility(bool bIsVisible);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AttackTile(EColor InPlayerColor);
	
	UFUNCTION(BlueprintCallable)
	int32 GetOwningPlayerIndex(){return OwnerPlayerIndex;};
	
	UFUNCTION(BlueprintPure)
	ETileStatus GetStatus() { return Status; };
	
	UFUNCTION(BlueprintPure)
	int32 GetTileQuestionCount() const;

	UFUNCTION(BlueprintPure)
	EQuestionType GetTileNextQuestionType() const;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="Visuals")
	void MC_RemoveSelection();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_CancelAttack();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AddTileToPlayerTerritory(ETileStatus InStatus, int32 InPlayerID, EColor InPlayerColor, EGameRound CurrentGameRound);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RemoveTileFromPlayerTerritory();
	
	UFUNCTION(BlueprintNativeEvent)
	void SpawnCastleMesh();

	UFUNCTION(BlueprintNativeEvent)
	void SpawnBannerMesh();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MC_SetBorderVisibility(bool bIsVisible);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MC_SetBannerVisibility(bool bIsVisible);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MC_SetCastleVisibility(bool bIsVisible);

	UFUNCTION(BlueprintCallable)
	void SC_ApplyDamage(int32 InDamageAmount);

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable)
	void MC_DamageTile();

	UFUNCTION(BlueprintNativeEvent)
	void PlayTileDamageAnimation();

	UFUNCTION(BlueprintPure)
	FORCEINLINE FIntPoint GetAxial() const { return Axial; };

	UFUNCTION(BlueprintPure)
	FORCEINLINE EGameRound GetAnnexedRound() const { return AnnexedRound; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetOwnerPlayerIndex() const { return OwnerPlayerIndex; };

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SwitchTileMeshMaterialColor();

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void SC_ToggleShowPreviewMesh(bool bShow);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowPreviewMesh(AActor* HoveredActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ForceShowPreviewMesh(AActor* HoveredActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HidePreviewMesh(AActor* HoveredActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HidePreviewMeshOnClick(AActor* HoveredActor, FKey PressedButton);
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* TileSpringArm = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* TileCamera = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh = nullptr;

	/* Static mesh that is the same form but slightly bigger
	 * Used for highlight events
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* BorderStaticMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* BannerMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* BannerPreviewMesh = nullptr;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* CastleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* CastlePreviewMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* FirstTowerMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* SecondTowerMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UWidgetComponent* PointsWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> TileMeshMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> TileMeshDefaultMaterial;
	
	UPROPERTY(BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> EdgesMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialOwned;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> CastleMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> BannerMaterial;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	ETileStatus Status = ETileStatus::NotOwned;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
	int32 TileHP = 1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_TileColor, BlueprintReadWrite, Category="Territory")
	EColor TileColor = EColor::Undefined;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_BorderColor, BlueprintReadWrite, Category="Territory")
	EColor BorderColor = EColor::Undefined;

	/*
	 * Game round in which this tile was annexed to a Player's territory
	 * Depending on this value different amount of points will be incremented to Player's Score
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Territory")
	EGameRound AnnexedRound = EGameRound::ChooseCastle;
	 /*
	  * XY coordinate of the Hex tile for any Hex related functions
	  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Territory", meta = (AllowPrivateAccess = "true", ExposeOnSpawn = true))
	FIntPoint Axial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Territory")
	TArray<EQuestionType> CurrentQuestionArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Territory")
	TArray<EQuestionType> DefaultQuestionArray;
	
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
	int32 OwnerPlayerIndex = -1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ShowPreviewMesh, BlueprintReadWrite, Category = "Player")
	bool bShowPreviewMesh = false;
	
	UFUNCTION()
	void OnRep_TileColor();

	UFUNCTION()
	void OnRep_ShowPreviewMesh();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnRep_TileColorMeshChangeOR();

	UFUNCTION()
	void OnRep_BorderColor();
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(BlueprintReadOnly, Replicated, meta = (AllowPrivateAccess = "true"))
	ETileStatus CachedStatus;
};
