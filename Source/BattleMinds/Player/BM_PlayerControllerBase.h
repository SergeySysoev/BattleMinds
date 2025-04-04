// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/BM_UWPlayerHUD.h"
#include "UI/BM_UWResults.h"
#include "BM_PlayerControllerBase.generated.h"

class UBM_GameInstance;
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
	UBM_UWQuestion* QuestionWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ChooseQuestionWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWQuestion> ShotQuestionWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWResults* ResultsWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UUserWidget> ResultsWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "User Widgets")
	UBM_UWPlayerHUD* PlayerHUD = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "User Widgets")
	TSubclassOf<UBM_UWPlayerHUD> PlayerHUDClass;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category= "Questions")
	void SC_AddAnsweredQuestionChoice(FInstancedStruct InPlayerChoice);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_OpenQuestionWidget(FInstancedStruct LastQuestion, const TArray<int32>& AnsweringPlayers, AActor* NewViewTarget, float QuestionTimerLength);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_RemoveQuestionWidget(bool bSwitchViewTargetBackToTiles = true);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_ShowResultsWidget(const TArray<APlayerState*>& PlayerArray);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_ShowCorrectAnswers(const TArray<FInstancedStruct>& PlayersChoices);
	
	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void SetPlayerBorderBlinking(bool IsEnabled);
	
	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void UpdateCurrentPlayerNickname(const int32 CurrentPlayerID);

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void UpdatePlayerTurnsAmount(const TArray<FPlayersCycleUI> & NewPlayerTurnsCycles);

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void UpdatePlayerTurnWidget(const int32 CurrentCycle, const int32 CurrentPlayerCount);

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void StopHUDAnimations();

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void UpdateRoundWidget(EGameRound NewRound);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_MarkAnsweredPlayers(int32 LastSentPlayer);

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void ResetTurnTimer(EGameRound GameRound);

	UFUNCTION(BlueprintImplementableEvent, Category= "HUD")
	void StartCountdownTimer(float TimerValue);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "HUD")
	void CC_InitPlayerHUD(const TArray<FPlayerInfo>& PlayersHUDInfo);

	UFUNCTION(Client, Unreliable, BlueprintCallable, Category= "HUD")
	void CC_SetGameLength(const EGameLength GameLength);

	UFUNCTION(Client, Unreliable, BlueprintCallable, Category= "HUD")
	void CC_ShowWarningPopup(const FText& InText);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Controls")
	void SC_TryClickTheTile(FIntPoint TargetTile);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player Info")
	void SC_SetPlayerInfo(const FString& InPlayerNickname);

	UFUNCTION(BlueprintCallable, Category = "Player Info")
	void SetPlayerInfoFromGI();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	FLinearColor GetPlayerColorByID(int32 PlayerID) const;

	UFUNCTION(BlueprintPure)
	FUniqueNetIdRepl GetPlayerUniqueNetIdByPlayerId(int32 PlayerID) const;
	
};
