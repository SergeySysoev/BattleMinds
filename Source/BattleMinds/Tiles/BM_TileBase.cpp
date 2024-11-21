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
	StaticMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	StaticMesh->SetIsReplicated(true);
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	FlagMesh->AttachToComponent(StaticMesh, FAttachmentTransformRules::KeepRelativeTransform);
	FlagMesh->SetIsReplicated(true);
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Mesh"));
	CastleMesh->AttachToComponent(StaticMesh, FAttachmentTransformRules::KeepRelativeTransform);
	CastleMesh->SetIsReplicated(true);
	EdgesBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Edges box"));
	EdgesBox->SetIsReplicated(true);
	EdgesBox->AttachToComponent(StaticMesh,FAttachmentTransformRules::KeepRelativeTransform);
	EdgesBox->SetVisibility(false);
	EdgesBox->SetHiddenInGame(false);
	CurrentMaterial = MaterialOwned;

}

void ABM_TileBase::ChangeStatus_Implementation(ETileStatus NewStatus)
{
	Status = NewStatus;
	UE_LOG(LogBM_Tile, Warning, TEXT("New owner: %s"), *OwnerPlayerNickname);
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
	FlagMesh->SetMaterial(0, MaterialAttacked);
}
void ABM_TileBase::OnRep_CastleMeshChanged()
{
	CastleMesh->SetMaterial(0, MaterialCastle);
}

void ABM_TileBase::DecreaseCastleHP_Implementation()
{
	TileHP--;
}

void ABM_TileBase::RemoveTileFromPlayerTerritory_Implementation(ABM_PlayerState* PlayerState)
{
	bIsAttacked = false;
	PlayerState->AddPoints(-1 * Points);
	PlayerState->OwnedTiles.RemoveSwap(this);
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
	FlagMesh->SetVisibility(false);
}

void ABM_TileBase::AddTileToPlayerTerritory_Implementation(ABM_PlayerState* PlayerState)
{
	bIsAttacked = false;
	if(PlayerState->OwnedTiles.Num() == 0)
	{
		ChangeStatus(ETileStatus::Castle);
		PlayerState->AddPoints(1000.0f);
	}
	else
	{
		ChangeStatus(ETileStatus::Controlled);
		PlayerState->AddPoints(Points);
	}
	MaterialOwned = PlayerState->MaterialTile;
	CurrentMaterial = MaterialOwned;
	StaticMesh->SetMaterial(0, CurrentMaterial);
	PlayerState->OwnedTiles.Add(this);
	OwnerPlayerID = PlayerState->BMPlayerID;
	OwnerPlayerNickname = PlayerState->Nickname;
	//PlayerState->SetPointsInWidget();
	//MC_TryUpdatePlayersHUD();
	MC_RemoveSelection();
	MC_ShowEdges(false, FColor::Black);
}

void ABM_TileBase::CancelAttack_Implementation()
{
	bIsAttacked = false;
	MC_RemoveSelection();
	MC_ShowEdges(false, FColor::Black);
}

void ABM_TileBase::TileWasChosen_Implementation(ABM_PlayerState* PlayerState, EGameRound GameRound)
{
	switch (Status)
	{
	case ETileStatus::NotOwned:
		if (IsValid(PlayerState))
		{
			if (GameRound == EGameRound::ChooseCastle)
			{
				OwnerPlayerNickname = PlayerState->Nickname;
				ChangeStatus(ETileStatus::Castle);
				MaterialCastle = PlayerState->MaterialCastle;
				CurrentMaterial = MaterialCastle;
				if (StaticMesh->GetMaterial(0))
					UE_LOG(LogBM_Tile, Display, TEXT("Current Material %s"), *StaticMesh->GetMaterial(0)->GetName());
				StaticMesh->SetMaterial(0, CurrentMaterial);
				CastleMesh->SetVisibility(true);
				CastleMesh->SetMaterial(0, MaterialCastle);
				AddTileToPlayerTerritory(PlayerState);
				//PlayerState->OwnedTiles.Add(this);
			}
			else //Set the territory
			{
				bIsAttacked = true;
				//OriginalMaterial = Material;
				MaterialAttacked = PlayerState->MaterialAttack;
				StaticMesh->SetMaterial(0 , MaterialOwned);
				MC_RemoveHighlighting();
				MC_ShowEdges(false, FColor::Black);
				FlagMesh->SetVisibility(true);
				FlagMesh->SetMaterial(0, MaterialAttacked);
			}
		}
		break;
	case ETileStatus::Controlled:
		if(OwnerPlayerNickname == PlayerState->Nickname)
		{
			if (!bIsFortified)
			{
				bIsFortified = true;
				TileHP++;
				UE_LOG(LogBM_Tile, Display, TEXT("Tile was fortified"));
			}
		}
		else
		{
			bIsAttacked = true;
			//OriginalMaterial = Material;
			MaterialAttacked = PlayerState->MaterialAttack;
			StaticMesh->SetMaterial(0 , MaterialOwned);
			MC_RemoveHighlighting();
			MC_ShowEdges(false, FColor::Black);
			FlagMesh->SetVisibility(true);
			FlagMesh->SetMaterial(0, MaterialAttacked);
		}
	case ETileStatus::Castle:
		break;
	default:
		break;
	};
}

void ABM_TileBase::TileWasClicked_Implementation(FKey ButtonPressed, EGameRound GameRound, ABM_PlayerState* PlayerState)
{
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
	DOREPLIFETIME(ABM_TileBase, MaterialOwned);
	DOREPLIFETIME(ABM_TileBase, MaterialCastle);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerNickname);
	DOREPLIFETIME(ABM_TileBase, StaticMesh);
	DOREPLIFETIME(ABM_TileBase, bIsFortified);
	DOREPLIFETIME(ABM_TileBase, bIsAttacked);
	DOREPLIFETIME(ABM_TileBase, TileHP);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerID);
}
