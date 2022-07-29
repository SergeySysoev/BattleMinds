// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BM_PlayerPawn.generated.h"

UCLASS()
class BATTLEMINDS_API ABM_PlayerPawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	ABM_PlayerPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
