// Battle Minds, 2021. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BM_PlayerControllerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerController, Display, All);

UCLASS()
class BATTLEMINDS_API ABM_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABM_PlayerControllerBase();

protected:
	UPROPERTY(BLueprintReadOnly, Category = "Player Info")
	FString Nickname;

};
