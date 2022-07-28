// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BM_UWResults.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMINDS_API UBM_UWResults : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Players results")
	TArray<APlayerState*> PlayerArray;
};
