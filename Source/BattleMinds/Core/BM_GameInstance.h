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
	/*
	 * Max amount of Players during the game
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	int32 NumberOfPlayers = 2;

	/*
	 * Map of Materials to be applied to the Players Castle meshes
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> CastleMaterials;

	/*
	 * Map of Materials to be applied to the Players Banner meshes that are placed
	 * when a Player has chosen the tile
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> BannerMaterials;

	/*
	 * Map of Materials to be applied to the Tile meshes
	 * when the Tile is added to the Player's territory
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, UMaterialInterface*> TileMeshMaterials;

	/*
	 * Map of Colors to be applied to the Tile meshes edges
	 * when the Player is deciding on which tile to attack
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TMap<EColor, FColor> TileEdgesColors;

	/*
	 * Question categories that were chosen by the host in Lobby Game Settings UI
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Settings")
	TArray<EQuestionCategories> ChosenCategories;

	/*
	 * Amount of seconds for Player to choose the tile
	 * that was set by the host in Lobby Game Settings UI
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 PlayerTurnTimer = 10;

	/*
	 * Amount of seconds for Player to give answer to the question
	 * that was set by the host in Lobby Game Settings UI
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 QuestionTimer = 10;
	
};
