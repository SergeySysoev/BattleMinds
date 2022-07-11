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
	StaticMesh->SetMaterial(0, Material);
	UnbindHighlightEvents();
}

void ABM_TileBase::TileWasChosen_Implementation(const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	switch (Status)
	{
	case ETileStatus::NotOwned:
		if (!PlayerNick.IsEmpty()) // && Player.HasCastle())
		{
			OwnerPlayerNickname = PlayerNick;
			ChangeStatus(ETileStatus::Controlled);
			Material = PlayerMaterial;
			if (StaticMesh->GetMaterial(0))
				UE_LOG(LogBM_Tile, Display, TEXT("Current Material %s"), *StaticMesh->GetMaterial(0)->GetName());
			StaticMesh->SetMaterial(0, Material);
			UnbindHighlightEvents();
			//OnRep_TileChanged();
		}
		break;
	case ETileStatus::Controlled:
		if (!bIsFortified)
		{
			bIsFortified = true;
			UE_LOG(LogBM_Tile, Display, TEXT("Tile was fortified"));
			this->OnBeginCursorOver.Clear();
		}
	case ETileStatus::Castle:
		break;
	default:
		break;
	};
}

bool ABM_TileBase::TileWasChosen_Validate(const FString& PlayerNick, UMaterialInterface* PlayerMaterial)
{
	return true;
}

void ABM_TileBase::TileWasClicked_Implementation(FKey ButtonPressed, const FString& PlayerNick, UMaterialInterface* PlayerMaterial, EGameRound GameRound)
{
	TileWasChosen(PlayerNick, PlayerMaterial);
	if(GameRound == EGameRound::SetTerritory || GameRound == EGameRound::FightForTerritory)
	{
		const auto PlayerController = Cast<ABM_PlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		PlayerController->OpenQuestion();
	}
}

bool ABM_TileBase::TileWasClicked_Validate(FKey ButtonPressed, const FString& PlayerNick, UMaterialInterface* PlayerMaterial, EGameRound)
{
	return true;
}

void ABM_TileBase::UnbindHighlightEvents_Implementation()
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
	DOREPLIFETIME(ABM_TileBase, OriginalMaterial);
	DOREPLIFETIME(ABM_TileBase, OwnerPlayerNickname);
	DOREPLIFETIME(ABM_TileBase, StaticMesh);
	DOREPLIFETIME(ABM_TileBase, bIsFortified);
}
