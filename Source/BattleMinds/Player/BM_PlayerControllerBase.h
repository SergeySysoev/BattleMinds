// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/BM_UWPlayerHUD.h"
#include "UI/BM_UWResults.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ChooseQuestionWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ShotQuestionWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWResults* ResultsWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UUserWidget> ResultsWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWPlayerHUD* PlayerHUD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWPlayerHUD> PlayerHUDClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Tiles")
	ABM_TileBase* CurrentClickedTile = nullptr;
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestion();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RequestToOpenQuestion();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_OpenQuestionWidget(FQuestion LastQuestion, const TArray<int32>& AnsweringPlayers);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentPlayerNickname(const FString& CurrentPlayerNickname);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_MarkAnsweredPlayers(int32 LastSentPlayer);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_RemoveQuestionWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowResultsWidget(const TArray<APlayerState*>& PlayerArray);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowCorrectAnswers(const TArray<FPlayerChoice>& PlayersChoices);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTurnTimer(EGameRound GameRound);
	UFUNCTION(BlueprintImplementableEvent)
	void ResetTurnTimer(EGameRound GameRound);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_InitPlayerHUD(const TArray<APlayerState*>& PlayerArray);
	
};
