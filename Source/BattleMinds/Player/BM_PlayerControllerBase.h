// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BM_PlayerControllerBase.generated.h"

class UBM_UWQuestion;
class ABM_TileBase;
class ABM_PlayerState;

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerController, Display, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetTimerDelegate);

UCLASS()
class BATTLEMINDS_API ABM_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABM_PlayerControllerBase();
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestion();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RequestToOpenQuestion();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_OpenQuestionWidget(FName QuestionRowName);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_RemoveQuestionWidget();
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTurnTimer();
	UFUNCTION(BlueprintImplementableEvent)
	void ResetTurnTimer();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWQuestion* QuestionWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> QuestionWidgetClass;
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile, EGameRound GameRound);

};
