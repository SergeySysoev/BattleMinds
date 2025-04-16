// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UniversalCamera.h"
#include "Interfaces/BMPostQuestionPhase.h"
#include "Interfaces/BMPrePlayerTurnInterface.h"
#include "BM_PlayerPawn.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerPawn, Display, All);

class ABM_GameStateBase;
class ABM_TileBase;

UCLASS()
class BATTLEMINDS_API ABM_PlayerPawn : public AUniversalCamera, public IBMPostQuestionPhase, public IBMPrePlayerTurnInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	FVector CachedLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	FRotator CachedRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	float CachedZoom = 1.0f;
	

public:
	ABM_PlayerPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IBMPostQuestionPhase interface
	virtual void PostQuestionPhaseFightForTheRestTiles(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo) override;
	virtual void PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo) override;
	virtual void CheckPostQuestionPhaseHandled() override;
	//~ IBMPostQuestionPhase interface

	// IBMPrePlayerTurn Interface
	virtual void PrePlayerTurnFightForTerritory_Implementation(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo) override;
	virtual void CheckPrePlayerTurnPhase_Implementation() override;
	
	//~IBMPrePlayerTurn interface
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Camera")
	void CC_ZoomIntoClickedTile(FVector ZoomLocation, FRotator ZoomRotation, float Zoom = 0.f, bool IgnoreLag = true, bool IgnoreRestrictions = true);

	UFUNCTION()
	void CachedCameraProperties();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Camera")
	void CC_RestoreCameraPropertiesFromCache();
};