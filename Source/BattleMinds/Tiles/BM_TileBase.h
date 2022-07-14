// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMinds/Player/BM_PlayerControllerBase.h"
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

	UFUNCTION(BlueprintCallable)
	void Highlight(AActor* TouchedActor);

	UFUNCTION(BlueprintCallable)
	void Unhighlight(AActor* TouchedActor);
	
	UFUNCTION()
	void OnRep_TileChanged();

	UFUNCTION(Server, reliable, WithValidation, BlueprintCallable)
	void TileWasChosen(const FString& PlayerNick, UMaterialInterface* PlayerMaterial);

	UFUNCTION(Server, reliable, WithValidation, BlueprintCallable)
	void TileWasClicked(FKey ButtonPressed, const FString& PlayerNick, UMaterialInterface* PlayerMaterial, EGameRound GameRound);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TileWasAttacked(UMaterialInterface* PlayerMaterialAttack);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void CancelAttack();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void BindHighlightEvents();
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void UnbindHighlightEvents();
	
	UFUNCTION(BlueprintCallable)
	FString GetOwningPlayerNickname(){ return OwnerPlayerNickname; };
	
	UFUNCTION(BlueprintCallable)
	ETileStatus GetStatus() { return Status; };

protected:
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Points")
	float Points = 200.0f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Territory")
	ETileStatus Status;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Territory")
	bool bIsArtillery;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Territory")
	bool bIsFortified;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UMaterialInterface> OriginalMaterial;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_TileChanged, BlueprintReadWrite, Category = "Player")
	FString OwnerPlayerNickname;

	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
