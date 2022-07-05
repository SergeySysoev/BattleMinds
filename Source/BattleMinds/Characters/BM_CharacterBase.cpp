// Battle Minds, 2022. All rights reserved.

#include "BM_CharacterBase.h"

DEFINE_LOG_CATEGORY(LogBM_Character);

ABM_CharacterBase::ABM_CharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABM_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABM_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABM_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

