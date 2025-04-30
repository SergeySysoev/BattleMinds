// Battle Minds, 2022. All rights reserved.


#include "Player/BM_PlayerPawn.h"

#include "BM_PlayerControllerBase.h"
#include "EnhancedInputSubsystems.h"
#include "Core/BM_GameStateBase.h"

DEFINE_LOG_CATEGORY(LogBM_PlayerPawn);

ABM_PlayerPawn::ABM_PlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABM_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocallyControlled())
	{
		DefaultCameraProperties = StaticCameraProperties;
		PlayerTurnCameraProperties = StaticCameraProperties;
		APlayerController* LPlayerController = Cast<APlayerController>(GetController());
		if (LPlayerController && LPlayerController->IsLocalController())
		{
			ULocalPlayer* LLocalPlayer = LPlayerController->GetLocalPlayer();
			if (IsValid(LLocalPlayer))
			{
				if (UEnhancedInputLocalPlayerSubsystem* LInputSystem = LLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					if (!DynamicCameraIMC.IsNull())
					{
						LInputSystem->AddMappingContext(DynamicCameraIMC.LoadSynchronous(), 0);
					}
				}
			}
		}
	}
}

void ABM_PlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABM_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABM_PlayerPawn::CC_SetPlayerCameraPivotLocation_Implementation(FVector InPivotLocation)
{
	RotationPivotLocation = InPivotLocation;
}

void ABM_PlayerPawn::CC_SetInputEnabled_Implementation(bool IsEnabled)
{
	APlayerController* LPlayerController = Cast<APlayerController>(GetController());
	if (LPlayerController && LPlayerController->IsLocalController())
	{
		ULocalPlayer* LLocalPlayer = LPlayerController->GetLocalPlayer();
		if (IsValid(LLocalPlayer))
		{
			UEnhancedInputLocalPlayerSubsystem* LInputSystem = LLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (IsValid(LInputSystem))
			{
				if (IsEnabled)
				{
					UInputMappingContext* LLoadedIMC = DynamicCameraIMC.LoadSynchronous();
					if (IsValid(LLoadedIMC) && !LInputSystem->HasMappingContext(LLoadedIMC))
					{
						LInputSystem->AddMappingContext(LLoadedIMC, 1);
					}
				}
				else
				{
					UInputMappingContext* LLoadedIMC = DynamicCameraIMC.LoadSynchronous();
					if (IsValid(LLoadedIMC) && LInputSystem->HasMappingContext(LLoadedIMC))
					{
						LInputSystem->RemoveMappingContext(DynamicCameraIMC.LoadSynchronous());
					}
				}
			}
		}
	}
}

void ABM_PlayerPawn::CC_SetDefaultCameraProperties_Implementation(const FUniversalCameraPositionSaveFormat& InDefaultProperties)
{
	if (bEnableDynamicCamera)
	{
		DefaultCameraProperties = InDefaultProperties;
	}
}

void ABM_PlayerPawn::CC_SetPlayerTurnCameraProperties_Implementation(const FUniversalCameraPositionSaveFormat& InDefaultProperties)
{
	if (bEnableDynamicCamera)
	{
		PlayerTurnCameraProperties = InDefaultProperties;
	}
}

void ABM_PlayerPawn::CallCheckPrePlayerTurnPhaseHandled()
{
	ABM_PlayerControllerBase* LOwningController = Cast<ABM_PlayerControllerBase>(GetOwner());
	if (IsValid(LOwningController))
	{
		LOwningController->CheckPrePlayerTurnPhaseHandled();	
	}
	else
	{
		UE_LOG(LogBM_PlayerPawn, Error, TEXT("Can't get PlayerController on CallCheckPrePlayerTurnPhaseHandled"));
	}
}

void ABM_PlayerPawn::CallCheckPostQuestionPhaseHandled()
{
	ABM_PlayerControllerBase* LOwningController = Cast<ABM_PlayerControllerBase>(GetOwner());
	if (IsValid(LOwningController))
	{
		LOwningController->CheckPostQuestionPhaseHandled();	
	}
	else
	{
		UE_LOG(LogBM_PlayerPawn, Error, TEXT("Can't get PlayerController on CallCheckPostQuestionPhaseHandled"));
	}
}

void ABM_PlayerPawn::CacheCameraProperties()
{
	CachedCameraProperties.DesiredLocation = DesiredLocation;
	CachedCameraProperties.DesiredRotation = DesiredRotation;
	CachedCameraProperties.DesiredZoom = DesiredZoom;
	CachedCameraProperties.DesiredRotationOffset = DesiredRotationOffset;
	CachedCameraProperties.DesiredSocketOffset = DesiredSocketOffset;
	CachedCameraProperties.DesiredTargetOffset = DesiredTargetOffset;
	UE_LOG(LogBM_PlayerPawn, Display, TEXT("Cached properties: Location: %s, Rotation: %s, Zoom: %f"),
		*CachedCameraProperties.DesiredLocation.ToString(), *CachedCameraProperties.DesiredRotation.ToString(), CachedCameraProperties.DesiredZoom);
}

void ABM_PlayerPawn::CC_TravelCameraToPlayerTurn_Implementation()
{
	if (bEnableDynamicCamera)
	{
		TravelCamera(true);
	}
	else
	{
		CallCheckPrePlayerTurnPhaseHandled();
	}
}

void ABM_PlayerPawn::CC_TravelCameraToDefault_Implementation()
{
	if (bEnableDynamicCamera)
	{
		TravelCamera(false);
	}
	else
	{
		CallCheckPostQuestionPhaseHandled();
	}
}

void ABM_PlayerPawn::CC_RestoreCameraPropertiesFromCache_Implementation()
{
	SetDesiredLocation(CachedCameraProperties.DesiredLocation, false, true);
	SetDesiredRotation(CachedCameraProperties.DesiredRotation, false, true);
	SetDesiredZoom(CachedCameraProperties.DesiredZoom, false, true);
}

void ABM_PlayerPawn::CC_ZoomIntoClickedTile_Implementation(FVector ZoomLocation, FRotator ZoomRotation, float Zoom, bool IgnoreLag, bool IgnoreRestrictions)
{
	if (bEnableDynamicCamera)
	{
		CacheCameraProperties();
		UE_LOG(LogBM_PlayerPawn, Display, TEXT("New camera pos: Location: %s, Rotation: %s, Zoom: %f"),
		*ZoomLocation.ToString(), *ZoomRotation.ToString(), Zoom);
		SetDesiredLocation(ZoomLocation, IgnoreLag, IgnoreRestrictions);
		SetDesiredRotation(ZoomRotation, IgnoreLag, IgnoreRestrictions);
		SetDesiredZoom(Zoom, IgnoreLag, IgnoreRestrictions);
	}
}

