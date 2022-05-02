// Battle Minds, 2021. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMinds/Player/BM_PlayerControllerBase.h"
#include "Net/UnrealNetwork.h"
#include "BM_TileBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_Tile, Display, All);

class UStaticMeshComponent;
class ABM_PlayerControllerBase;

UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	NotOwned		UMETA(DisplayName = "Not Owned"),
	Controlled		UMETA(DisplayName = "Controlled"),
	Castle			UMETA(DisplayName = "Castle"),
	Fortified		UMETA(DisplayName = "Fortified"),
};

UCLASS()
class BATTLEMINDS_API ABM_TileBase : public AActor
{
	GENERATED_BODY()

public:
	ABM_TileBase();

	UFUNCTION(NetMulticast, reliable, WithValidation, BlueprintCallable)
		void ChangeStatus(ETileStatus NewStatus);

	UFUNCTION(BlueprintCallable)
		void Highlight(AActor* TouchedActor);

	UFUNCTION(BlueprintCallable)
		void Unhighlight(AActor* TouchedActor);

	UFUNCTION(BlueprintCallable)
		void TileWasChosen(FKey ButtonPressed, AActor* TouchedActor, ABM_PlayerControllerBase* Player);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Points")
		float Points = 200.0f;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
		ETileStatus Status;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Territory")
		bool bIsArtillery;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Visuals")
		UMaterial* Material;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Visuals")
		UMaterial* OriginalMaterial;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
		ABM_PlayerControllerBase* LastClickedPlayer;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Player")
		ABM_PlayerControllerBase* OwnerPlayer;

	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
