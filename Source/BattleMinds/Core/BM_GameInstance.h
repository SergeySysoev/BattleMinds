// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Engine/GameInstance.h"
#include "BM_GameInstance.generated.h"

UCLASS()
class BATTLEMINDS_API UBM_GameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	int32 NumberOfPlayers = 2;
};
