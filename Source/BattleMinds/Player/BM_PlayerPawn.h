// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "UniversalCamera.h"
#include "BM_PlayerPawn.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerPawn, Display, All);

class ABM_GameStateBase;
class ABM_TileBase;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class BATTLEMINDS_API ABM_PlayerPawn : public AUniversalCamera
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	FUniversalCameraPositionSaveFormat CachedCameraProperties;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	FUniversalCameraPositionSaveFormat StaticCameraProperties;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	FUniversalCameraPositionSaveFormat DefaultCameraProperties;

	/* Calculated based on the Default Castle Camera properties*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	FUniversalCameraPositionSaveFormat PlayerTurnCameraProperties;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	float PlayerTurnCameraZoom = 2500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	float PlayerTurnCameraPitch = -72.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	FUniversalCameraPositionSaveFormat ZoomTileCameraProperties;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inputs")
	TSoftObjectPtr<UInputMappingContext> DynamicCameraIMC = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	bool bEnableDynamicCamera = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
	FVector RotationPivotLocation = FVector::ZeroVector;

public:
	ABM_PlayerPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Client, Reliable, Category="Camera")
	void CC_SetPlayerCameraPivotLocation(FVector InPivotLocation);

	UFUNCTION(Client, Reliable, Category="Inputs")
	void CC_SetInputEnabled(bool IsEnabled);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_SetDefaultCameraProperties(const FUniversalCameraPositionSaveFormat& InDefaultProperties);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_SetPlayerTurnCameraProperties(const FUniversalCameraPositionSaveFormat& InDefaultProperties);
	
	UFUNCTION(BlueprintCallable)
	void CallCheckPrePlayerTurnPhaseHandled();

	UFUNCTION(BlueprintCallable)
	void CallCheckPostQuestionPhaseHandled();
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Camera")
	void CC_ZoomIntoClickedTile(FVector ZoomLocation, FRotator ZoomRotation, float Zoom = 0.f, bool IgnoreLag = true, bool IgnoreRestrictions = true);

	UFUNCTION()
	void CacheCameraProperties();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Camera")
	void CC_RestoreCameraPropertiesFromCache();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Camera")
	void CC_TravelCameraToDefault();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Camera")
	void CC_TravelCameraToPlayerTurn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Camera")
	void TravelCamera(bool IsPlayerTurn);
	
};