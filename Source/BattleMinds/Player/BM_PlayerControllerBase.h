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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWQuestion* QuestionWidget;
	UPROPERTY(EditAnywhere, /*Replicated,*/ BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ChooseQuestionWidgetClass;
	UPROPERTY(EditAnywhere, /*Replicated,*/ BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ShotQuestionWidgetClass;
	UPROPERTY(EditAnywhere, /*Replicated,*/ BlueprintReadWrite, Category= "User Widgets")
	UUserWidget* ResultsWidget;;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Tiles")
	ABM_TileBase* CurrentClickedTile = nullptr;
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestion();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RequestToOpenQuestion();
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestionWidget(EQuestionType QuestionType, FName QuestionName);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_OpenQuestionWidget(FName QuestionRowName, const TArray<int32> &AnsweringPlayers);
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveQuestionWidget();
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentPlayerNickname(const FString &CurrentPlayerNickname);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_RemoveQuestionWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowResultsWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowCorrectAnswers(const TArray<FPlayerChoice> &PlayersChoices);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTurnTimer(EGameRound GameRound);
	UFUNCTION(BlueprintImplementableEvent)
	void ResetTurnTimer(EGameRound GameRound);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile);

};
