// Battle Minds, 2022. All rights reserved.


#include "Player/BM_PlayerPawn.h"

ABM_PlayerPawn::ABM_PlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABM_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABM_PlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABM_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

