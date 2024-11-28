// Battle Minds, 2022. All rights reserved.


#include "BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "Core/BM_GameModeBase.h"
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
	CurrentMaterial = MaterialOwned;

}

void ABM_TileBase::ChangeStatus_Implementation(ETileStatus NewStatus)
{
	Status = NewStatus;
	switch (Status)
	{
		case ETileStatus::Castle:
			CastleMesh->SetMaterial(0, MaterialCastle);
			CastleMesh->SetVisibility(true);
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
	StaticMesh->SetMaterial(0, CurrentMaterial);
}

void ABM_TileBase::OnRep_FlagMeshChanged()
{
	BannerMesh->SetMaterial(0, MaterialAttacked);
}
void ABM_TileBase::OnRep_CastleMeshChanged()
{
	CastleMesh->SetMaterial(0, MaterialCastle);
}

void ABM_TileBase::SC_SiegeTile_Implementation(UMaterialInterface* InMaterialTile)
{
	bIsAttacked = true;
	MaterialAttacked = InMaterialTile;
	StaticMesh->SetMaterial(0 , MaterialOwned);
	MC_RemoveHighlighting();
	MC_ShowEdges(false, FColor::Black);
	BannerMesh->SetVisibility(true);
	BannerMesh->SetMaterial(0, MaterialAttacked);
}

void ABM_TileBase::SC_SetDisputedAppearance_Implementation()
{
	BannerMesh->SetVisibility(true);
	MaterialAttacked = DisputedTerritoryMaterial;
	BannerMesh->SetMaterial(0, DisputedTerritoryMaterial);
}

void ABM_TileBase::DecreaseCastleHP_Implementation()
{
	TileHP--;
}

void ABM_TileBase::RemoveTileFromPlayerTerritory_Implementation()
{
	bIsAttacked = false;
	// TODO? reset Material/OwnerID/OwnerNickname
}

void ABM_TileBase::AddTileToPlayerTerritory_Implementation(ETileStatus InStatus, int32 InPlayerID, const FString& InPlayerNickname, UMaterialInterface* InMaterialTile)
{
	bIsAttacked = false;
	MaterialOwned = InMaterialTile;
	CurrentMaterial = MaterialOwned;
	StaticMesh->SetMaterial(0, CurrentMaterial);
	OwnerPlayerID = InPlayerID;
	OwnerPlayerNickname = InPlayerNickname;
	ChangeStatus(InStatus);
	MC_RemoveSelection();
	MC_ShowEdges(false, FColor::Black);
}

void ABM_TileBase::MC_ShowEdges_Implementation(bool bVisibility, FColor PlayerColor)
{
	EdgesBox->ShapeColor = PlayerColor;
	EdgesBox->SetVisibility(bVisibility);
}

void ABM_TileBase::TurnOffHighlight_Implementation()
{
	CurrentMaterial = MaterialOwned;
	OnRep_TileMeshChanged();
}

void ABM_TileBase::TurnOnHighlight_Implementation(UMaterialInterface* NeighborMaterial)
{
	CurrentMaterial = NeighborMaterial;
	OnRep_TileMeshChanged();
}

void ABM_TileBase::MC_RemoveHighlighting_Implementation()
{
	StaticMesh->SetMaterial(0, MaterialOwned);
}

void ABM_TileBase::MC_RemoveSelection_Implementation()
{
	BannerMesh->SetVisibility(false);
}

void ABM_TileBase::CancelAttack_Implementation()
{
	bIsAttacked = false;
	MC_RemoveSelection();
	MC_ShowEdges(false, FColor::Black);
}

void ABM_TileBase::TileWasChosen_Implementation(ABM_PlayerState* PlayerState, EGameRound GameRound)
{
}

void ABM_TileBase::TileWasClicked_Implementation(FKey ButtonPressed, EGameRound GameRound, ABM_PlayerState* PlayerState)
{
	/* add Tooltip widget opening when pressing with RMB or holding on the Tile*/
	switch (GameRound)
	{
		case EGameRound::ChooseCastle:
			TileWasChosen(PlayerState, GameRound);
			break;
		case EGameRound::SetTerritory:
			TileWasChosen(PlayerState, GameRound);
			break;
		case EGameRound::FightForTerritory:
			TileWasChosen(PlayerState, GameRound);
			break;
	}
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
	DOREPLIFETIME(ABM_TileBase, CurrentMaterial);
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
}
