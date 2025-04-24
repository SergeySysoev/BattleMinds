// Battle Minds, 2022. All rights reserved.

#include "Characters/BMCharacterSpawnSlot.h"
#include "BM_CharacterBase.h"

ABMCharacterSpawnSlot::ABMCharacterSpawnSlot()
{
	PrimaryActorTick.bCanEverTick = false;
	PreviewSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewSkeletalMesh"));
	PreviewSkeletalMesh->SetupAttachment(GetRootComponent());
	PreviewSkeletalMesh->SetHiddenInGame(true);
}

ABM_CharacterBase* ABMCharacterSpawnSlot::SpawnCharacter()
{
	FActorSpawnParameters SpawnInfo;
	const FVector LSpawnLocation = GetActorLocation();
	const FRotator LSpawnRotation = PreviewSkeletalMesh->GetComponentRotation();
	FTransform LSpawnTransform;
	LSpawnTransform.SetLocation(LSpawnLocation);
	LSpawnTransform.SetRotation(LSpawnRotation.Quaternion());
	ABM_CharacterBase* LSpawnedCharacter = GetWorld()->SpawnActorDeferred<ABM_CharacterBase>(SpawnCharacterClass, LSpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (IsValid(LSpawnedCharacter))
	{
		LSpawnedCharacter->FinishSpawning(LSpawnTransform);
	}
	return LSpawnedCharacter;
}

void ABMCharacterSpawnSlot::BeginPlay()
{
	Super::BeginPlay();
}
