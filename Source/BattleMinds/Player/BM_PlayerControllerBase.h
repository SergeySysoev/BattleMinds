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

	UFUNCTION()
	FORCEINLINE ABM_TileBase* GetCurrentClickedTile() const { return CurrentClickedTile; }

	UFUNCTION()
	bool HasValidCurrentClickedTile() const;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OpenQuestion();
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RequestToOpenQuestion(EQuestionType QuestionType);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AddAnsweredQuestionChoice(FInstancedStruct InPlayerChoice);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_OpenQuestionWidget(FInstancedStruct LastQuestion, const TArray<int32>& AnsweringPlayers, AActor* NewViewTarget);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentPlayerNickname(const int32 CurrentPlayerID);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_MarkAnsweredPlayers(int32 LastSentPlayer);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_RemoveQuestionWidget();
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowResultsWidget(const TArray<APlayerState*>& PlayerArray);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_ShowCorrectAnswers(const TArray<FInstancedStruct>& PlayersChoices);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTurnTimer(EGameRound GameRound);
	
	UFUNCTION(BlueprintImplementableEvent)
	void ResetTurnTimer(EGameRound GameRound);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SC_TryClickTheTile(ABM_TileBase* TargetTile);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AssignCurrentTile(ABM_TileBase* TargetTile);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CC_InitPlayerHUD(const TArray<APlayerState*>& PlayerArray);

	UFUNCTION(Client, Unreliable, BlueprintCallable)
	void CC_ShowWarningPopup(const FText& InText);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_CancelAttackForCurrentTile() const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_AddCurrentTileToTerritory(ETileStatus InTileStatus) const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SC_RemoveCurrentTileFromTerritory() const;

	UFUNCTION(BlueprintPure)
	int32 GetPointsOfCurrentClickedTile() const ;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Tiles")
	ABM_TileBase* CurrentClickedTile = nullptr;
};
