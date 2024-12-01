// Battle Minds, 2022. All rights reserved.


#include "BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogBM_Tile);

ABM_TileBase::ABM_TileBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
	StaticMesh->SetIsReplicated(true);
	SetRootComponent(StaticMesh);
	
	BannerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Banner Mesh"));
	BannerMesh->SetupAttachment(StaticMesh);
	BannerMesh->SetIsReplicated(true);
	
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Mesh"));
	CastleMesh->SetupAttachment(StaticMesh);
	CastleMesh->SetIsReplicated(true);
	
	EdgesBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Edges box"));
	EdgesBox->SetIsReplicated(true);
	EdgesBox->SetupAttachment(StaticMesh);
	EdgesBox->SetVisibility(false);
	EdgesBox->SetHiddenInGame(false);
	
	TileMeshMaterial = MaterialOwned;

}

void ABM_TileBase::ChangeStatus_Implementation(ETileStatus NewStatus)
{
	Status = NewStatus;
	switch (Status)
	{
		case ETileStatus::Castle:
			SpawnCastleMesh();
			break;
		case ETileStatus::Controlled:
			break;
		default: break;
	}
	UE_LOG(LogBM_Tile, Warning, TEXT("Tile %s, New owner: %s"), *GetName(), *OwnerPlayerNickname);
}

bool ABM_TileBase::ChangeStatus_Validate(ETileStatus NewStatus)
{
	return true;
}

void ABM_TileBase::OnRep_TileMeshChanged()
{
	StaticMesh->SetMaterial(0, TileMeshMaterial);
}

void ABM_TileBase::OnRep_FlagMeshChanged()
{
	BannerMesh->SetMaterial(0, MaterialAttacked);
}

void ABM_TileBase::OnRep_CastleMeshChanged()
{
	CastleMesh->SetMaterial(0, MaterialCastle);
}

void ABM_TileBase::OnRep_AttackingColor()
{
	UE_LOG(LogBM_Tile, Display, TEXT("Current color: %s"), *AttackingColor.ToString());
	if (AttackingColor == FColor::Transparent)
	{
		EdgesBox->ShapeColor = FColor::Transparent;
		EdgesBox->SetVisibility(false);
	}
	else
	{
		EdgesBox->ShapeColor = AttackingColor;
		EdgesBox->SetVisibility(true);
	}
	
}

void ABM_TileBase::SC_SetAttackingColorForTileEdges_Implementation(EColor AttackingPlayerColor)
{
	AttackingColor = TileMaterials.FindRef(AttackingPlayerColor).TileEdgesColor;
	if (HasAuthority())
	{
		OnRep_AttackingColor();
	}
}

void ABM_TileBase::SetInGameTTileMaterials(TMap<EColor, FTileMaterials> InGameMaterials)
{
	check(HasAuthority());
	TileMaterials = InGameMaterials;
}

void ABM_TileBase::SpawnBannerMesh_Implementation()
{
	BannerMesh->SetMaterial(0, MaterialAttacked);
	BannerMesh->SetVisibility(true);
}

void ABM_TileBase::SpawnCastleMesh_Implementation()
{
	CastleMesh->SetMaterial(0, MaterialCastle);
	CastleMesh->SetVisibility(true);
}

void ABM_TileBase::SC_SiegeTile_Implementation(EColor InPlayerColor)
{
	bIsAttacked = true;
	MaterialAttacked = TileMaterials.FindRef(InPlayerColor).BannerMaterial;
	StaticMesh->SetMaterial(0 , MaterialOwned);
	SC_SetAttackingColorForTileEdges(EColor::Undefined);
	SpawnBannerMesh();
}

void ABM_TileBase::SC_SetDisputedAppearance_Implementation()
{
	BannerMesh->SetVisibility(true);
	MaterialAttacked = DisputedTerritoryMaterial;
	BannerMesh->SetMaterial(0, DisputedTerritoryMaterial);
}

void ABM_TileBase::SC_RestoreCastleHP_Implementation()
{
	TileHP++;
}

void ABM_TileBase::SC_DecreaseCastleHP_Implementation()
{
	TileHP--;
}

void ABM_TileBase::RemoveTileFromPlayerTerritory_Implementation()
{
	bIsAttacked = false;
	// TODO? reset Material/OwnerID/OwnerNickname
}

void ABM_TileBase::AddTileToPlayerTerritory_Implementation(ETileStatus InStatus, int32 InPlayerID, const FString& InPlayerNickname, EColor InPlayerColor)
{
	bIsAttacked = false;
	MaterialOwned = TileMaterials.FindRef(InPlayerColor).TileMeshMaterial;
	TileMeshMaterial = MaterialOwned;
	StaticMesh->SetMaterial(0, TileMeshMaterial);
	OwnerPlayerID = InPlayerID;
	OwnerPlayerNickname = InPlayerNickname;
	MaterialCastle = TileMaterials.FindRef(InPlayerColor).CastleMaterial;
	ChangeStatus(InStatus);
	MC_RemoveSelection();
	SC_SetAttackingColorForTileEdges(EColor::Undefined);
	OnBannerMeshSpawned.Clear();
}

void ABM_TileBase::MC_ShowEdges_Implementation(bool bVisibility, EColor InPlayerColor)
{
	if (!bVisibility) 
	{
		EdgesBox->ShapeColor = FColor::Blue;
	}
	else
	{
		EdgesBox->ShapeColor = TileMaterials.FindRef(InPlayerColor).TileEdgesColor;
	}
	EdgesBox->SetVisibility(bVisibility);
}

void ABM_TileBase::MC_RemoveSelection_Implementation()
{
	BannerMesh->SetVisibility(false);
}

void ABM_TileBase::CancelAttack_Implementation()
{
	bIsAttacked = false;
	MC_RemoveSelection();
	OnBannerMeshSpawned.Clear();
	SC_SetAttackingColorForTileEdges(EColor::Undefined);
}

void ABM_TileBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABM_TileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABM_TileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABM_TileBase, Points);
	DOREPLIFETIME(ABM_TileBase, Status);
	DOREPLIFETIME(ABM_TileBase, bIsArtillery);
	DOREPLIFETIME(ABM_TileBase, TileMeshMaterial);
	DOREPLIFETIME(ABM_TileBase, MaterialAttacked);
	DOREPLIFETIME(ABM_TileBase, DisputedTerritoryMaterial);
	DOREPLIFETIME(ABM_TileBase, MaterialOwned);
	DOREPLIFETIME(ABM_TileBase, MaterialCastle);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerNickname);
	DOREPLIFETIME(ABM_TileBase, StaticMesh);
	DOREPLIFETIME(ABM_TileBase, bIsFortified);
	DOREPLIFETIME(ABM_TileBase, bIsAttacked);
	DOREPLIFETIME(ABM_TileBase, TileHP);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerID);
	DOREPLIFETIME(ABM_TileBase, AttackingColor);
}
