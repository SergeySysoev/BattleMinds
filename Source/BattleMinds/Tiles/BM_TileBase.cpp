// Battle Minds, 2022. All rights reserved.


#include "BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Core/BM_GameInstance.h"
#include "Core/BM_GameStateBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogBM_Tile);

ABM_TileBase::ABM_TileBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;

	TileSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TileSpringArm"));
	TileSpringArm->SetupAttachment(Root);
	TileCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TileCamera"));
	TileCamera->SetupAttachment(TileSpringArm);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetIsReplicated(true);

	BorderStaticMesh  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Border Mesh"));
	BorderStaticMesh->SetupAttachment(StaticMesh);
	BorderStaticMesh->SetIsReplicated(true);
	
	BannerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Banner Mesh"));
	BannerMesh->SetupAttachment(RootComponent);
	BannerMesh->SetIsReplicated(true);

	BannerPreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Banner Preview Mesh"));
	BannerPreviewMesh->SetupAttachment(RootComponent);
	
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Mesh"));
	CastleMesh->SetupAttachment(RootComponent);
	CastleMesh->SetIsReplicated(true);

	CastlePreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Preview Mesh"));
	CastlePreviewMesh->SetupAttachment(RootComponent);

	FirstTowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("First Tower Mesh"));
	FirstTowerMesh->SetupAttachment(CastleMesh);
	FirstTowerMesh->SetIsReplicated(true);
	SecondTowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Second Tower Mesh"));
	SecondTowerMesh->SetupAttachment(CastleMesh);
	SecondTowerMesh->SetIsReplicated(true);

	PointsWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Points Widget"));
	PointsWidget->SetupAttachment(RootComponent);
	PointsWidget->SetHiddenInGame(true);
	PointsWidget->SetIsReplicated(true);
	
	TileMeshMaterial = MaterialOwned;

	CurrentQuestionArray.Add(EQuestionType::Choose);
}

void ABM_TileBase::SC_ChangeStatus_Implementation(ETileStatus NewStatus)
{
	Status = NewStatus;
	switch (Status)
	{
		case ETileStatus::Castle:
			SpawnCastleMesh();
			MC_SetCastleVisibility(true);
			CurrentQuestionArray.Add(EQuestionType::Choose);
			CurrentQuestionArray.Add(EQuestionType::Choose);
			TileHP = 3;
			break;
		case ETileStatus::Controlled:
			TileHP = 1;
			break;
		default: break;
	}
	UE_LOG(LogBM_Tile, Warning, TEXT("Tile %s, New owner: %d"), *GetName(), OwnerPlayerIndex);
}

bool ABM_TileBase::SC_ChangeStatus_Validate(ETileStatus NewStatus)
{
	return true;
}

void ABM_TileBase::SC_RevertStatus_Implementation()
{
	Status = CachedStatus;
}

void ABM_TileBase::SetTileEdgesColor(EColor NewColor)
{
	if (HasAuthority())
	{
		BorderColor = NewColor;
		OnRep_BorderColor();
	}
}

void ABM_TileBase::MC_SetBorderVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(BorderStaticMesh))
	{
		BorderStaticMesh->SetVisibility(bIsVisible);
	}
}

void ABM_TileBase::SetPointsWidgetVisibility(bool bIsVisible)
{
	if (IsValid(PointsWidget))
	{
		PointsWidget->SetHiddenInGame(!bIsVisible);
	}
}

void ABM_TileBase::MC_SetPointsWidgetVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(PointsWidget))
	{
		PointsWidget->SetHiddenInGame(!bIsVisible);
	}
}

void ABM_TileBase::MC_SetBannerVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(BannerMesh))
	{
		BannerMesh->SetVisibility(bIsVisible);
	}
}

void ABM_TileBase::MC_SetCastleVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(CastleMesh) && IsValid(FirstTowerMesh) && IsValid(SecondTowerMesh))
	{
		CastleMesh->SetVisibility(bIsVisible);
		FirstTowerMesh->SetVisibility(bIsVisible);
		SecondTowerMesh->SetVisibility(bIsVisible);
	}
}

int32 ABM_TileBase::GetTileQuestionCount() const
{
	return TileHP;
}

EQuestionType ABM_TileBase::GetTileNextQuestionType() const
{
	int32 LQuestionIndex = FMath::Abs(CurrentQuestionArray.Num() - TileHP);
	if (CurrentQuestionArray.IsValidIndex(LQuestionIndex))
	{
		return CurrentQuestionArray[LQuestionIndex];
	}
	return EQuestionType::Choose;
}

void ABM_TileBase::SpawnBannerMesh_Implementation()
{
	OnBannerMeshSpawnedNative.Broadcast();
}

void ABM_TileBase::SpawnCastleMesh_Implementation()
{
}

void ABM_TileBase::SC_AttackTile_Implementation(EColor InPlayerColor)
{
	if (HasAuthority())
	{
		CachedStatus = Status;
		bIsAttacked = true;
		SC_ChangeStatus(ETileStatus::Attacked);
		BorderColor = InPlayerColor;
		OnRep_BorderColor();
		MC_SetBannerVisibility(true);
		SpawnBannerMesh();
	}
}

void ABM_TileBase::SC_RemoveTileFromPlayerTerritory_Implementation()
{
	bIsAttacked = false;
	// TODO? reset Material/OwnerID/OwnerNickname
}

void ABM_TileBase::SC_AddTileToPlayerTerritory_Implementation(ETileStatus InStatus, int32 InPlayerID, EColor InPlayerColor, EGameRound CurrentGameRound)
{
	if (HasAuthority())
	{
		UE_LOG(LogBM_Tile, Warning, TEXT("Tile %d;%d was added to the Player #%d territory with status %s"),
			Axial.X, Axial.Y, InPlayerID, *UEnum::GetValueAsString(InStatus));
		bIsAttacked = false;
		OwnerPlayerIndex = InPlayerID;
		SC_ChangeStatus(InStatus);
		TileColor = InPlayerColor;
		OnRep_TileColor();
		MC_SetBannerVisibility(false);
		AnnexedRound = CurrentGameRound;	
	}
	OnBannerMeshSpawned.Clear();
}

void ABM_TileBase::MC_RemoveSelection_Implementation()
{
	BannerMesh->SetVisibility(false);
}

void ABM_TileBase::SC_CancelAttack_Implementation()
{
	bIsAttacked = false;
	MC_SetBannerVisibility(false);
	OnBannerMeshSpawned.Clear();
	SC_RevertStatus();
}

void ABM_TileBase::OnRep_TileColor()
{
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(LGameInstance))
	{
		CastleMaterial = LGameInstance->CastleMaterials.FindRef(TileColor);
		if (IsValid(CastleMaterial))
		{
			CastleMesh->SetMaterial(0, CastleMaterial);
			FirstTowerMesh->SetMaterial(0, CastleMaterial);
			SecondTowerMesh->SetMaterial(0, CastleMaterial);
		}
		TileMeshMaterial = LGameInstance->TileMeshMaterials.FindRef(TileColor);
		if (IsValid(TileMeshMaterial))
		{
			OnRep_TileColorMeshChangeOR();
		}
	}
}

void ABM_TileBase::OnRep_ShowPreviewMesh()
{
	UE_LOG(LogBM_Tile, Display, TEXT(" On Rep Show preview mesh"));
}

void ABM_TileBase::OnRep_TileColorMeshChangeOR_Implementation()
{
	StaticMesh->SetMaterial(0, TileMeshMaterial);
}

void ABM_TileBase::OnRep_BorderColor()
{
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(LGameInstance))
	{
		EdgesMaterial = LGameInstance->BorderMeshMaterials.FindRef(BorderColor);
		if (IsValid(EdgesMaterial))
		{
			BorderStaticMesh->SetMaterial(0, EdgesMaterial);
		}
		else
		{
			UE_LOG(LogBM_Tile, Error, TEXT("ABM_TileBase::OnRep_BorderColor: Not Valid Material!"));
		}
		BannerMaterial = LGameInstance->BannerMaterials.FindRef(BorderColor);
		if (IsValid(BannerMaterial))
		{
			BannerMesh->SetMaterial(0, BannerMaterial);
		}
		else
		{
			UE_LOG(LogBM_Tile, Warning, TEXT("ABM_TileBase::OnRep_BorderColor, Banner Material is not valid!"));
		}
	}
}

void ABM_TileBase::BeginPlay()
{
	Super::BeginPlay();
	OnBeginCursorOver.AddUniqueDynamic(this, &ThisClass::ShowPreviewMesh);
	OnEndCursorOver.AddUniqueDynamic(this, &ThisClass::HidePreviewMesh);
}

void ABM_TileBase::ShowPreviewMesh_Implementation(AActor* HoveredActor)
{
	UE_LOG(LogBM_Tile, Warning, TEXT("OnBeginMouseOver delegate actor: %s"), *HoveredActor->GetName());
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(UGameplayStatics::GetPlayerController(this,0));
	if (!IsValid(LPlayerController))
	{
		return;
	}
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(LPlayerController->PlayerState);
	if (!IsValid(LPlayerState))
	{
		return;
	}
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (!IsValid(LGameInstance))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("bShow : %s, IsPlayerTurn: %s"), bShowPreviewMesh?TEXT("true"):TEXT("false"),
		LPlayerState->IsPlayerTurn()?TEXT("true"):TEXT("false"));
	if (bShowPreviewMesh && LPlayerState->IsPlayerTurn() && IsValid(LGameState))
	{
		if (IsValid(LPlayerController->GetCachedTileWithPreview()))
		{
			LPlayerController->GetCachedTileWithPreview()->HidePreviewMesh(nullptr);
		}
		UMaterialInterface* LCastlePreviewMaterial = nullptr;
		UMaterialInterface* LBannerPreviewMaterial = nullptr;
		switch (LGameState->GetCurrentRound())
		{
			case EGameRound::ChooseCastle:
				LCastlePreviewMaterial = LGameInstance->CastleMaterials.FindRef(LPlayerState->GetPlayerColor());
				CastlePreviewMesh->SetMaterial(0, LCastlePreviewMaterial);
				CastlePreviewMesh->SetVisibility(true);
				UE_LOG(LogTemp, Warning, TEXT("Showing Castle Preview Mesh"));
			break;
			default:
				LBannerPreviewMaterial = LGameInstance->BannerMaterials.FindRef(LPlayerState->GetPlayerColor());
				BannerPreviewMesh->SetMaterial(0, LBannerPreviewMaterial);
				BannerPreviewMesh->SetVisibility(true);
			break;
		}
	}
}

void ABM_TileBase::HidePreviewMesh_Implementation(AActor* HoveredActor)
{
	CastlePreviewMesh->SetVisibility(false);
	BannerPreviewMesh->SetVisibility(false);
}

void ABM_TileBase::HidePreviewMeshOnClick_Implementation(AActor* HoveredActor, FKey PressedButton)
{
	CastlePreviewMesh->SetVisibility(false);
	BannerPreviewMesh->SetVisibility(false);
}

void ABM_TileBase::SC_ApplyDamage(int32 InDamageAmount)
{
	TileHP -= InDamageAmount;
	if (TileHP <= 0 && CachedStatus == ETileStatus::Castle)
	{
		OnCastleDestroyed.Broadcast(OwnerPlayerIndex);
	}
}

void ABM_TileBase::ForceShowPreviewMesh_Implementation(AActor* HoveredActor)
{
	UE_LOG(LogBM_Tile, Warning, TEXT("OnBeginMouseOver delegate actor: %s"), *HoveredActor->GetName());
	ABM_GameStateBase* LGameState = Cast<ABM_GameStateBase>(GetWorld()->GetGameState());
	ABM_PlayerControllerBase* LPlayerController = Cast<ABM_PlayerControllerBase>(UGameplayStatics::GetPlayerController(this,0));
	if (!IsValid(LPlayerController))
	{
		return;
	}
	ABM_PlayerState* LPlayerState = Cast<ABM_PlayerState>(LPlayerController->PlayerState);
	if (!IsValid(LPlayerState))
	{
		return;
	}
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (!IsValid(LGameInstance))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Force Show Preview, bShow : %s, IsPlayerTurn: %s"), bShowPreviewMesh?TEXT("true"):TEXT("false"),
		LPlayerState->IsPlayerTurn()?TEXT("true"):TEXT("false"));
	if (IsValid(LGameState))
	{
		if (IsValid(LPlayerController->GetCachedTileWithPreview()))
		{
			LPlayerController->GetCachedTileWithPreview()->HidePreviewMesh(nullptr);
		}
		UMaterialInterface* LCastlePreviewMaterial = nullptr;
		UMaterialInterface* LBannerPreviewMaterial = nullptr;
		switch (LGameState->GetCurrentRound())
		{
			case EGameRound::ChooseCastle:
				LCastlePreviewMaterial = LGameInstance->CastleMaterials.FindRef(LPlayerState->GetPlayerColor());
			CastlePreviewMesh->SetMaterial(0, LCastlePreviewMaterial);
			CastlePreviewMesh->SetVisibility(true);
			UE_LOG(LogTemp, Warning, TEXT("Showing Castle Preview Mesh"));
			break;
			default:
				LBannerPreviewMaterial = LGameInstance->BannerMaterials.FindRef(LPlayerState->GetPlayerColor());
			BannerPreviewMesh->SetMaterial(0, LBannerPreviewMaterial);
			BannerPreviewMesh->SetVisibility(true);
			break;
		}
	}
}

void ABM_TileBase::SC_ToggleShowPreviewMesh_Implementation(bool bShow)
{
	bShowPreviewMesh = bShow;
}

void ABM_TileBase::PlayTileDamageAnimation_Implementation() {}

void ABM_TileBase::MC_DamageTile_Implementation()
{
	PlayTileDamageAnimation();
}

void ABM_TileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABM_TileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_TileBase, Status);
	DOREPLIFETIME(ABM_TileBase, bIsAttacked);
	DOREPLIFETIME(ABM_TileBase, TileHP);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerIndex);
	DOREPLIFETIME(ABM_TileBase, Axial);
	DOREPLIFETIME(ABM_TileBase, CachedStatus);
	DOREPLIFETIME(ABM_TileBase, bShowPreviewMesh);
	DOREPLIFETIME_CONDITION(ABM_TileBase, TileColor, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ABM_TileBase, BorderColor, COND_SimulatedOnly);
}
