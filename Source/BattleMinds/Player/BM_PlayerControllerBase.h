// Battle Minds, 2022. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core/BM_Types.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/BM_UWPlayerHUD.h"
#include "UI/BM_UWResults.h"
#include "InputMappingContext.h"
#include "SharedStructs.h"
#include "Interfaces/BMPostQuestionPhase.h"
#include "Interfaces/BMPrePlayerTurnInterface.h"
#include "Interfaces/BMQuestionEffectsInterface.h"
#include "BM_PlayerControllerBase.generated.h"

class UBM_GameInstance;
class UBM_UWQuestion;
class ABM_TileBase;
class ABM_PlayerState;
class UEnhancedInputLocalPlayerSubsystem;

DECLARE_LOG_CATEGORY_EXTERN(LogBM_PlayerController, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetTimerDelegate);

UCLASS()
class BATTLEMINDS_API ABM_PlayerControllerBase
	: public APlayerController, public IBMPostQuestionPhase,
	  public IBMPrePlayerTurnInterface, public IBMQuestionEffectsInterface
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

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Camera")
	void CC_SetPlayerCameraPivotLocation(FVector InPivotLocation);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_SetViewTargetWithBlend(AActor* NewViewTarget, float BlendTime);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category= "Questions")
	void SC_AddAnsweredQuestionChoice(FInstancedStruct InPlayerChoice);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_OpenQuestionWidget(FInstancedStruct LastQuestion, const TArray<int32>& AnsweringPlayers, AActor* NewViewTarget, float QuestionTimerLength);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Questions")
	void CC_RemoveQuestionWidget(bool bSwitchViewTargetBackToTiles = true, AActor* NewViewTarget = nullptr);
	
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
	void UpdateCurrentPlayerTurnSlot(const int32 CurrentPlayerCounter, FUniqueNetIdRepl PlayerUniqueNetId, EColor PlayerColor);

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

	UFUNCTION(Client, Reliable, BlueprintCallable, Category= "Tiles")
	void CC_CheckForTileUnderCursor(const FIntPoint& FirstTileAxials);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Controls")
	void SC_TryClickTheTile(FIntPoint TargetTile);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player Info")
	void SC_SetPlayerInfo(const FString& InPlayerNickname);

	UFUNCTION(BlueprintCallable, Category = "Player Info")
	void SetPlayerInfoFromGI();

	UFUNCTION(BlueprintNativeEvent, Category = "Inputs")
	void SetInputEnabled(bool bIsEnabled);

	UFUNCTION(Client, Reliable, Category = "Inputs")
	void CC_SetInputEnabled(bool bIsEnabled);

	UFUNCTION(BlueprintPure, Category = "Tiles")
	FORCEINLINE ABM_TileBase* GetCachedTileWithPreview() {return CachedTileWithPreview;}

	// IBMPrePlayerTurn Interface
	virtual void CheckPrePlayerTurnPhaseHandled() override;
	//~IBMPrePlayerTurn Interface

	// IBMPostQuestionPhase interface
	virtual void CheckPostQuestionPhaseHandled() override;
	//~IBMPostQuestionPhase interface

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "General Camera")
	void SC_SetCameraDefaultProperties(const FUniversalCameraPositionSaveFormat& InDefaultProperties);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "General Camera")
	void SC_SetCameraPlayerTurnProperties(const FUniversalCameraPositionSaveFormat& InDefaultProperties);
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	FLinearColor GetPlayerColorByID(int32 PlayerID) const;

	UFUNCTION(BlueprintPure)
	FUniqueNetIdRepl GetPlayerUniqueNetIdByPlayerId(int32 PlayerID) const;

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TSoftObjectPtr<UInputMappingContext> EssentialIMC = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TSoftObjectPtr<UInputMappingContext> ClassicIMC = nullptr;

	// IBMPostQuestionPhase interface
	virtual void PostQuestionPhaseChooseCastle(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo) override;
	virtual void PostQuestionPhaseFightForTerritory(const FPostQuestionPhaseInfo& PostQuestionPhaseInfo) override;
	//~ IBMPostQuestionPhase interface

	// IBMPrePlayerTurn Interface
	virtual void PrePlayerTurnChooseCastle(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo) override;
	virtual void PrePlayerTurnSetTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo) override;
	virtual void PrePlayerTurnFightForTheRestTiles(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo) override;
	virtual void PrePlayerTurnFightForTerritory(FPrePlayerTurnPhaseInfo PrePlayerTurnPhaseInfo) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "PrePlayerTurnPhase")
	void SC_CheckPrePlayerTurnPhaseHandled();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "PostQuestionPhase")
	void SC_CheckPostQuestionPhaseHandled();
	//~IBMPrePlayerTurn interface

	// QuestionEffects interface
	void ReactToBombExplosion_Implementation(FVector Location, FVector WorldDirection) override;
	//~QuestionEffects interface
	
	UPROPERTY(BlueprintReadOnly, Category="Tiles")
	TObjectPtr<ABM_TileBase> CachedTileWithPreview;
};
