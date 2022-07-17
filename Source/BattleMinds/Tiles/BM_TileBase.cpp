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
	StaticMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	StaticMesh->SetIsReplicated(true);
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	FlagMesh->AttachToComponent(StaticMesh, FAttachmentTransformRules::KeepRelativeTransform);
	FlagMesh->SetIsReplicated(true);
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Castle Mesh"));
	CastleMesh->AttachToComponent(StaticMesh, FAttachmentTransformRules::KeepRelativeTransform);
	CastleMesh->SetIsReplicated(true);
	Material = OriginalMaterial;

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

void ABM_TileBase::Highlight(AActor* TouchedActor)
{
	if (Status == ETileStatus::NotOwned)
	{
		ABM_PlayerState* PlayerState = Cast<ABM_PlayerState>(GetWorld()->GetFirstPlayerController()->PlayerState);
		if (PlayerState)
		{
			StaticMesh->SetMaterial(0, PlayerState->MaterialTile);
		}
		
	}
}

void ABM_TileBase::Unhighlight(AActor* TouchedActor)
{
	StaticMesh->SetMaterial(0, OriginalMaterial);
}

void ABM_TileBase::OnRep_TileChanged()
{
	StaticMesh->SetMaterial(0, MaterialOwned);
	//UnbindHighlightEvents();
}

void ABM_TileBase::OnRep_TileMeshChanged()
{
	StaticMesh->SetMaterial(0, MaterialOwned);
}
void ABM_TileBase::OnRep_FlagMeshChanged()
{
	FlagMesh->SetMaterial(0, MaterialAttacked);
}
void ABM_TileBase::OnRep_CastleMeshChanged()
{
	CastleMesh->SetMaterial(0, MaterialCastle);
}

void ABM_TileBase::MC_RemoveHighlighting_Implementation()
{
	if (MaterialOwned)
		StaticMesh->SetMaterial(0, MaterialOwned);
	else
		StaticMesh->SetMaterial(0, OriginalMaterial);
}

void ABM_TileBase::MC_RemoveSelection_Implementation()
{
	FlagMesh->SetVisibility(false);
}

void ABM_TileBase::MC_BindHighlightEvents_Implementation()
{
	this->OnBeginCursorOver.AddDynamic(this, &ABM_TileBase::Highlight);
	this->OnEndCursorOver.AddDynamic(this, &ABM_TileBase::Unhighlight);
}

void ABM_TileBase::AddTileToPlayerTerritory_Implementation(ABM_PlayerState* PlayerState)
{
	bIsAttacked = false;
	ChangeStatus(ETileStatus::Controlled);
	MaterialOwned = PlayerState->MaterialTile;
	StaticMesh->SetMaterial(0, MaterialOwned);
	MC_RemoveSelection();
	MC_UnbindHighlightEvents();
}

void ABM_TileBase::CancelAttack_Implementation()
{
	ChangeStatus(ETileStatus::NotOwned);
	bIsAttacked = false;
	MC_RemoveSelection();
	MC_BindHighlightEvents();
}

void ABM_TileBase::TileWasChosen_Implementation(const FString& PlayerNick, UMaterialInterface* PlayerMaterial, EGameRound GameRound)
{
	switch (Status)
	{
	case ETileStatus::NotOwned:
		if (!PlayerNick.IsEmpty())
		{
			if (GameRound == EGameRound::ChooseCastle)
			{
				OwnerPlayerNickname = PlayerNick;
				ChangeStatus(ETileStatus::Castle);
				MaterialCastle = PlayerMaterial;
				MaterialOwned = PlayerMaterial;
				if (StaticMesh->GetMaterial(0))
					UE_LOG(LogBM_Tile, Display, TEXT("Current Material %s"), *StaticMesh->GetMaterial(0)->GetName());
				StaticMesh->SetMaterial(0, MaterialOwned);
				CastleMesh->SetVisibility(true);
				CastleMesh->SetMaterial(0, MaterialCastle);
				MC_UnbindHighlightEvents();
			}
			else //Attacking the tile or setting the territory
			{
				bIsAttacked = true;
				//OriginalMaterial = Material;
				MaterialAttacked = PlayerMaterial;
				StaticMesh->SetMaterial(0 , OriginalMaterial);
				MC_RemoveHighlighting();
				FlagMesh->SetVisibility(true);
				FlagMesh->SetMaterial(0, MaterialAttacked);
				MC_UnbindHighlightEvents();
			}
		}
		break;
	case ETileStatus::Controlled:
		if(OwnerPlayerNickname == PlayerNick)
		{
			if (!bIsFortified)
			{
				bIsFortified = true;
				UE_LOG(LogBM_Tile, Display, TEXT("Tile was fortified"));
				this->OnBeginCursorOver.Clear();
			}
		}
		else
		{
			UE_LOG(LogBM_Tile, Display, TEXT("Tile was fortified"));
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
			TileWasChosen(PlayerState->Nickname, PlayerState->MaterialCastle, GameRound);
			break;
		case EGameRound::SetTerritory:
			TileWasChosen(PlayerState->Nickname, PlayerState->MaterialAttack, GameRound);
			break;
		case EGameRound::FightForTerritory:
			TileWasChosen(PlayerState->Nickname, PlayerState->MaterialAttack, GameRound);
			const auto PlayerController = Cast<ABM_PlayerControllerBase>(PlayerState->GetPlayerController());
			PlayerController->OpenQuestion();
			break;
	}
}

void ABM_TileBase::MC_UnbindHighlightEvents_Implementation()
{
	this->OnBeginCursorOver.RemoveDynamic(this, &ABM_TileBase::Highlight);
	this->OnEndCursorOver.RemoveDynamic(this, &ABM_TileBase::Unhighlight);
}

void ABM_TileBase::BeginPlay()
{
	Super::BeginPlay();
	this->OnBeginCursorOver.AddDynamic(this, &ABM_TileBase::Highlight);
	this->OnEndCursorOver.AddDynamic(this, &ABM_TileBase::Unhighlight);
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
	DOREPLIFETIME(ABM_TileBase, Material);
	DOREPLIFETIME(ABM_TileBase, MaterialAttacked);
	DOREPLIFETIME(ABM_TileBase, MaterialOwned);
	DOREPLIFETIME(ABM_TileBase, MaterialCastle);
	DOREPLIFETIME(ABM_TileBase, OriginalMaterial);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerNickname);
	DOREPLIFETIME(ABM_TileBase, StaticMesh);
	DOREPLIFETIME(ABM_TileBase, bIsFortified);
	DOREPLIFETIME(ABM_TileBase, bIsAttacked);
}
