// Battle Minds, 2021. All rights reserved.

#include "BM_CharacterBase.h"

// Sets default values
ABM_CharacterBase::ABM_CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABM_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABM_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABM_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

