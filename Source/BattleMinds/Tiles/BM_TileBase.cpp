// Battle Minds, 2022. All rights reserved.


#include "BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Core/BM_GameInstance.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogBM_Tile);

ABM_TileBase::ABM_TileBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetIsReplicated(true);

	BorderStaticMesh  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Border Mesh"));
	BorderStaticMesh->SetupAttachment(StaticMesh);
	BorderStaticMesh->SetIsReplicated(true);
	
	BannerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Banner Mesh"));
	BannerMesh->SetupAttachment(RootComponent);
	BannerMesh->SetIsReplicated(true);
	
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Mesh"));
	CastleMesh->SetupAttachment(RootComponent);
	CastleMesh->SetIsReplicated(true);
	
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
	UE_LOG(LogBM_Tile, Warning, TEXT("Tile %s, New owner: %d"), *GetName(), OwnerPlayerID);
}

bool ABM_TileBase::SC_ChangeStatus_Validate(ETileStatus NewStatus)
{
	return true;
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

void ABM_TileBase::MC_SetBannerVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(BannerMesh))
	{
		BannerMesh->SetVisibility(bIsVisible);
	}
}

void ABM_TileBase::MC_SetCastleVisibility_Implementation(bool bIsVisible)
{
	if (IsValid(CastleMesh))
	{
		CastleMesh->SetVisibility(bIsVisible);
	}
}

EQuestionType ABM_TileBase::GetTileNextQuestionType() const
{
	if (CurrentQuestionArray.IsValidIndex(FMath::Abs(CurrentQuestionArray.Num() - TileHP)))
	{
		return CurrentQuestionArray[TileHP-1];
	}
	return EQuestionType::Undefined;
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
		bIsAttacked = true;
		SC_ChangeStatus(ETileStatus::Attacked);
		BorderColor = InPlayerColor;
		OnRep_BorderColor();
		MC_SetBannerVisibility(true);
		SpawnBannerMesh();
	}
}

void ABM_TileBase::SC_ChangeTileHP_Implementation(int32 HPIncrement)
{
	TileHP+= HPIncrement;
}

void ABM_TileBase::SC_RemoveTileFromPlayerTerritory_Implementation()
{
	bIsAttacked = false;
	// TODO? reset Material/OwnerID/OwnerNickname
}

void ABM_TileBase::SC_AddTileToPlayerTerritory_Implementation(ETileStatus InStatus, int32 InPlayerID, EColor InPlayerColor)
{
	if (HasAuthority())
	{
		UE_LOG(LogBM_Tile, Warning, TEXT("Tile %d;%d was added to the Player #%d territory with status %s"),
			Axial.X, Axial.Y, InPlayerID, *UEnum::GetValueAsString(InStatus));
		bIsAttacked = false;
		OwnerPlayerID = InPlayerID;
		SC_ChangeStatus(InStatus);
		TileColor = InPlayerColor;
		OnRep_TileColor();
		MC_SetBannerVisibility(false);
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
	if (OwnerPlayerID >= 0)
	{
		SC_ChangeStatus(ETileStatus::Controlled);
	}
	else
	{
		SC_ChangeStatus(ETileStatus::NotOwned);
	}
}


void ABM_TileBase::OnRep_TileColor()
{
	/*switch (Status)
	{
		case ETileStatus::NotOwned:
			StaticMesh->SetMaterial(0, TileMeshDefaultMaterial);
			break;
		case ETileStatus::Castle:
			CastleMesh->SetMaterial(0, CastleMaterial);
			//CastleMesh->SetVisibility(true);
		case ETileStatus::Controlled:
			StaticMesh->SetMaterial(0, TileMeshMaterial);
			//StaticMesh->SetVisibility(true);
			break;
		case ETileStatus::Attacked:
			BannerMesh->SetMaterial(0, BannerMaterial);
		default:break;
	}*/
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(LGameInstance))
	{
		CastleMaterial = LGameInstance->CastleMaterials.FindRef(TileColor);
		if (IsValid(CastleMaterial))
		{
			CastleMesh->SetMaterial(0, CastleMaterial);
		}
		TileMeshMaterial = LGameInstance->TileMeshMaterials.FindRef(TileColor);
		if (IsValid(TileMeshMaterial))
		{
			StaticMesh->SetMaterial(0, TileMeshMaterial);
		}
	}
}

void ABM_TileBase::OnRep_BorderColor()
{
	UBM_GameInstance* LGameInstance = Cast<UBM_GameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(LGameInstance))
	{
		EdgesMaterial = LGameInstance->BorderMeshMaterials.FindRef(BorderColor);
		if (IsValid(EdgesMaterial))
		{
			//UE_LOG(LogBM_Tile, Warning, TEXT("ABM_TileBase::OnRep_BorderColor, Border Material: %s"), *EdgesMaterial->GetName());
			BorderStaticMesh->SetMaterial(0, EdgesMaterial);
		}
		else
		{
			UE_LOG(LogBM_Tile, Error, TEXT("ABM_TileBase::OnRep_BorderColor: Not Valid Material!"));
		}
		BannerMaterial = LGameInstance->BannerMaterials.FindRef(BorderColor);
		if (IsValid(BannerMaterial))
		{
			//UE_LOG(LogBM_Tile, Warning, TEXT("ABM_TileBase::OnRep_BorderColor, Banner Material: %s"), *BannerMaterial->GetName());
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
}

void ABM_TileBase::SC_ApplyDamage(int32 InDamageAmount)
{
	TileHP -= InDamageAmount;
	if (TileHP <= 0)
	{
		OnCastleDestroyed.Broadcast();
	}
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
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerID);
	DOREPLIFETIME(ABM_TileBase, Axial);
	DOREPLIFETIME_CONDITION(ABM_TileBase, TileColor, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ABM_TileBase, BorderColor, COND_SimulatedOnly);
}
