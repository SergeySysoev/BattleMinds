// Battle Minds, 2021. All rights reserved.


#include "BM_TileBase.h"

DEFINE_LOG_CATEGORY(LogBM_Tile);

ABM_TileBase::ABM_TileBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
	StaticMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

void ABM_TileBase::ChangeStatus_Implementation(ETileStatus NewStatus)
{
	this->OnEndCursorOver.Clear();
	Status = NewStatus;
	//UE_LOG(LogBM_Tile, Warning, TEXT("New owner: %s"), *OwnerPlayer->Name.ToString());
}

bool ABM_TileBase::ChangeStatus_Validate(ETileStatus NewStatus)
{
	if (LastClickedPlayer != nullptr)
	{
		return true;
	}
	return false;
}

void ABM_TileBase::Highlight(AActor* TouchedActor)
{
	if (Status == ETileStatus::NotOwned)
	{
		StaticMesh->SetMaterial(0, Material);
	}
}

void ABM_TileBase::Unhighlight(AActor* TouchedActor)
{
	StaticMesh->SetMaterial(0, OriginalMaterial);
}

void ABM_TileBase::TileWasChosen(FKey ButtonPressed, AActor* TouchedActor, ABM_PlayerControllerBase* Player)
{
	if (ButtonPressed.IsMouseButton())
	{
		switch (Status)
		{
		case ETileStatus::NotOwned:
			if (Player != nullptr) // && Player.HasCastle())
			{
				LastClickedPlayer = Player;
				ChangeStatus_Implementation(ETileStatus::Controlled);
			}
			//if(Player != nullptr // && !Player.HasCastle())
			//{
			//	LastClickedPlayer = Player;
			//	ChangeStatus_Implementation(ETileStatus::Castle);
			//}
			break;
		case ETileStatus::Controlled:
			Status = ETileStatus::Fortified;
			this->OnBeginCursorOver.Clear();
		case ETileStatus::Castle:
			break;
		case ETileStatus::Fortified:
			break;
		default:
			break;
		};
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
	DOREPLIFETIME(ABM_TileBase, LastClickedPlayer);
	DOREPLIFETIME(ABM_TileBase, Material);
	DOREPLIFETIME(ABM_TileBase, OriginalMaterial);
}
