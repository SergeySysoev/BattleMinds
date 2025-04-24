// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> BlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category="Animation")
	bool bIsAttacker = false;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor, VisibleAnywhere, BlueprintReadWrite, Category="Visuals")
	EColor PlayerColor = EColor::Undefined;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Visuals")
	TMap<EColor, UMaterialInterface*> MeshMaterials;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PlayerColor();
	
public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(NetMulticast, Unreliable, Category="Animation")
	void MC_PlayAnimation(EQuestionResult QuestionResult);

	UFUNCTION(Server, Reliable)
	void SC_SetAttacker(bool IsAttacker);

	UFUNCTION(Server, Unreliable)
	void SC_SetColor(EColor InPlayerColor);
};