// Battle Minds, 2022. All rights reserved.

#include "BM_CharacterBase.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogBM_Character);

ABM_CharacterBase::ABM_CharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
}

void ABM_CharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABM_CharacterBase, PlayerColor);
	DOREPLIFETIME(ABM_CharacterBase, bIsAttacker);
}

void ABM_CharacterBase::OnRep_PlayerColor()
{
	GetMesh()->SetMaterial(0, MeshMaterials.FindRef(PlayerColor));
}


void ABM_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABM_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABM_CharacterBase::SC_SetAttacker_Implementation(bool IsAttacker)
{
	bIsAttacker = IsAttacker;
}

void ABM_CharacterBase::SC_SetColor_Implementation(EColor InPlayerColor)
{
	PlayerColor = InPlayerColor;
	GetMesh()->SetMaterial(0, MeshMaterials.FindRef(PlayerColor));
}

void ABM_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABM_CharacterBase::MC_PlayAnimation_Implementation(EQuestionResult QuestionResult)
{
	UAnimMontage* LMontageToPlay = nullptr;
	switch (QuestionResult) {
		case EQuestionResult::ShotQuestionNeeded:
			LMontageToPlay = AttackMontage;
			break;
		case EQuestionResult::WrongAnswer:
			LMontageToPlay = bIsAttacker ? DeathMontage : DeathMontage;
			break;
		case EQuestionResult::NobodyAnswered:
			LMontageToPlay = BlockMontage;
			break;
		case EQuestionResult::TileDefended:
			LMontageToPlay = bIsAttacker ? AttackMontage : BlockMontage;
			break;
		case EQuestionResult::TileDamaged:
			LMontageToPlay = bIsAttacker ? AttackMontage : DeathMontage;
			break;
		case EQuestionResult::TileCaptured:
			LMontageToPlay = bIsAttacker ? AttackMontage : DeathMontage;
			break;
	}
	const float LMontageLength = PlayAnimMontage(LMontageToPlay, 1);
	UE_LOG(LogBM_Character, Display, TEXT("Question Montage length %f"), LMontageLength);
}

