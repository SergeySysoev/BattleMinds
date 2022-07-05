// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BM_CharacterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_Character, Display, All);

UCLASS()
class BATTLEMINDS_API ABM_CharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ABM_CharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
