// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerStateBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerStateBase, Display, All);

UCLASS()
class BATTLEMINDS_API ABM_PlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayerColor(EColor NewColor);

	UFUNCTION(BlueprintPure)
	FORCEINLINE EColor GetPlayerColor() const { return PlayerColor; }

	UFUNCTION(BlueprintCallable)
	void SetPlayerAvatar(UTexture2D* NewAvatar);

	UFUNCTION(BlueprintPure)
	FORCEINLINE UTexture2D* GetPlayerAvatar() const { return PlayerAvatar; }

	UFUNCTION(BlueprintCallable)
	void SetPlayerNickname(FString NewNickname);

	UFUNCTION(BlueprintPure)
	FORCEINLINE FString GetPlayerNickname() const { return PlayerNickname; }

protected:
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_Color, Category = "Player Info")
	EColor PlayerColor = EColor::Undefined;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	TObjectPtr<UTexture2D> PlayerAvatar;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	FString PlayerNickname = "NewUser";

	virtual void CopyProperties(APlayerState* PlayerState) override;

	virtual void OverrideWith(APlayerState* PlayerState) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Color();

	virtual void PlayerColorChanged();
};
