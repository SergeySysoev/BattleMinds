// Battle Minds, 2021. All rights reserved.


#include "BM_TileBase.h"
#include "BattleMinds/Player/BM_PlayerState.h"
#include "GameFramework/Actor.h"

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
	Status = NewStatus;
	UE_LOG(LogBM_Tile, Warning, TEXT("New owner: %s"), *OwnerPlayerNick);
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
			StaticMesh->SetMaterial(0, PlayerState->Material);
		}
		
	}
}

void ABM_TileBase::Unhighlight(AActor* TouchedActor)
{
	StaticMesh->SetMaterial(0, OriginalMaterial);
}

void ABM_TileBase::TileWasChosen_Implementation(const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	switch (Status)
	{
	case ETileStatus::NotOwned:
		if (!PlayerNick.IsEmpty()) // && Player.HasCastle())
		{
			OwnerPlayerNick = PlayerNick;
			ChangeStatus_Implementation(ETileStatus::Controlled);
			Material = PlayerMaterial;
			StaticMesh->SetMaterial(0, Material);
			UE_LOG(LogBM_Tile, Display, TEXT("Current Material %s"), *Material->GetName());
			this->OnBeginCursorOver.RemoveDynamic(this, &ABM_TileBase::Highlight);
			this->OnEndCursorOver.RemoveDynamic(this, &ABM_TileBase::Unhighlight);
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

bool ABM_TileBase::TileWasChosen_Validate(const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	return true;
}

void ABM_TileBase::TileWasClicked_Implementation(FKey ButtonPressed, const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	TileWasChosen_Implementation(PlayerNick, PlayerMaterial);
}

bool ABM_TileBase::TileWasClicked_Validate(FKey ButtonPressed, const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	if (ButtonPressed == EKeys::LeftMouseButton)
	{
		return true;
	}
	return false;
}

void ABM_TileBase::BeginPlay()
{
	Super::BeginPlay();
	bReplicates = true;
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
	DOREPLIFETIME(ABM_TileBase, OriginalMaterial);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerNick);
	DOREPLIFETIME(ABM_TileBase, StaticMesh);
}
