// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API ABM_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	FString Nickname;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	float Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Player Info")
	UMaterialInterface* Material;
};
