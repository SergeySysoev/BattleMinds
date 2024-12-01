// Battle Minds, 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerStateBase.generated.h"

UCLASS()
class BATTLEMINDS_API ABM_PlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayerColor(EColor NewColor);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE EColor GetPlayerColor() const { return PlayerColor;}
protected:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	EColor PlayerColor = EColor::Undefined;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
