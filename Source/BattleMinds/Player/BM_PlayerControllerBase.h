// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerControllerBase.generated.h"

class ABM_TileBase;
class ABM_PlayerState;

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerController, Display, All);

UCLASS()
class BATTLEMINDS_API ABM_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABM_PlayerControllerBase();
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestion();
	/*UFUNCTION(Server, Reliable, BlueprintCallable, BlueprintImplementableEvent)
	void SC_RequestToOpenQuestion();
	UFUNCTION(Client, Reliable, BlueprintCallable,BlueprintImplementableEvent)
	void CC_OpenQuestionWidget();*/
protected:
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile);

};
