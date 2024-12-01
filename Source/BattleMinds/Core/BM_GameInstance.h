// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Engine/GameInstance.h"
#include "Core/BM_Types.h"
#include "BM_GameInstance.generated.h"

UCLASS()
class BATTLEMINDS_API UBM_GameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	int32 NumberOfPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> CastleMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> BannerMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> TileMeshMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, FColor> TileEdgesColors;
};
