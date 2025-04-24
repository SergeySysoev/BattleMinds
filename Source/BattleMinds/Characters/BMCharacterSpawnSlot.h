// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/Actor.h"
#include "BMCharacterSpawnSlot.generated.h"

class UArrowComponent;
class ABM_CharacterBase;

UCLASS()
class BATTLEMINDS_API ABMCharacterSpawnSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	ABMCharacterSpawnSlot();

	UFUNCTION()
	ABM_CharacterBase* SpawnCharacter();

	UFUNCTION()
	FORCEINLINE EQuestionType GetQuestionType() const {return QuestionType;}
	
	UFUNCTION()
	FORCEINLINE EGameRound GetGameRound() const {return GameRound;}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Slot")
	EQuestionType QuestionType = EQuestionType::Undefined;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Slot")
	TSubclassOf<ABM_CharacterBase> SpawnCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Slot")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Slot")
	EGameRound GameRound = EGameRound::SetTerritory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Slot")
	USkeletalMeshComponent* PreviewSkeletalMesh = nullptr;
};
