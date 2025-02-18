// Battle Minds, 2022. All rights reserved.


#include "BMDataUtilsLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

DEFINE_LOG_CATEGORY(LogBMEditorDataUtils);

void UBMDataUtilsLibrary::CreateQuestionsDataTable(FName QuestionsStringTableID, FName AnswersStringTableID, EQuestionType QuestionType, EQuestionCategories QuestionsCategory, FString InTableName)
{
	
	TArray<FString> LQuestionsKeys;
	TArray<FString> LQuestionsTexts;
	
	FStringTableConstPtr LQuestionsStringTablePtr = FStringTableRegistry::Get().FindStringTable(QuestionsStringTableID);
	if (!LQuestionsStringTablePtr.IsValid())
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Failed to find string table with the id: %s"), *QuestionsStringTableID.ToString());
		return;
	}
	
	LQuestionsStringTablePtr->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
	{
		LQuestionsKeys.Add(InKey);
		LQuestionsTexts.Add(InSourceString);
		return true; // continue enumeration
	});
	
	if (LQuestionsKeys.Num() == 0)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Question Keys are not populated"));
		return;
	}
	if (LQuestionsTexts.Num() == 0)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Question Texts are not populated"));
		return;
	}
	
	
	TArray<FString> LAnswersKeys;
	TArray<FString> LAnswersTexts;
	TArray<FName> LMetaDataIdsFromAnswersStringTable;
	TArray<FString> LMetaDataFromAnswersStringTable;
	TMap<FString, FString> LQuestionAnswersKeysMap;
	FStringTableConstPtr LAnswersStringTablePtr = FStringTableRegistry::Get().FindStringTable(AnswersStringTableID);
	if (!LAnswersStringTablePtr.IsValid())
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Failed to find string table with the id: %s"), *AnswersStringTableID.ToString());
		return;
	}
	
	LAnswersStringTablePtr->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
	{
		LAnswersKeys.Add(InKey);
		LAnswersTexts.Add(InSourceString);
		return true; // continue enumeration
	});
	for (const FString& AnswerKey : LAnswersKeys)
	{
		LAnswersStringTablePtr->EnumerateMetaData(AnswerKey, [&](FName InMetaDataId, const FString& InMetaData) -> bool
		{
			LMetaDataIdsFromAnswersStringTable.Add(InMetaDataId);
			LMetaDataFromAnswersStringTable.Add(InMetaData);
			return true; // continue enumeration
		});
	}
	if (LAnswersKeys.Num() == 0)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Answer Keys are not populated"));
		return;
	}
	if (LAnswersTexts.Num() == 0)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Answer Texts are not populated"));
		return;
	}
	if (LMetaDataFromAnswersStringTable.Num() == 0)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Answers Metadata are not populated"));
		return;
	}
	

	//Step 3: Create DT
	UDataTable* LNewDataTable = NewObject<UDataTable>();
	switch (QuestionType)
	{
		case EQuestionType::Choose:
		{
			LNewDataTable->RowStruct = FQuestionChooseText::StaticStruct();
			for (int32 i = 0,j = 0; i < LAnswersKeys.Num(), j < LQuestionsKeys.Num(); i+=4, j++)
			{
				FQuestionChooseText LNewQuestionChooseText;
				LNewQuestionChooseText.Category = QuestionsCategory;
				LNewQuestionChooseText.Type = QuestionType;
				FString LQuestionKey = LQuestionsKeys[j].RightChop(2).LeftChop(2);
				LNewQuestionChooseText.Question = FText::FromStringTable(QuestionsStringTableID, LQuestionsKeys[j], EStringTableLoadingPolicy::FindOrFullyLoad);
				LNewQuestionChooseText.Answers = TArray<FText>{
					FText::FromStringTable(AnswersStringTableID,LAnswersKeys[i], EStringTableLoadingPolicy::FindOrFullyLoad),
					FText::FromStringTable(AnswersStringTableID,LAnswersKeys[i+1], EStringTableLoadingPolicy::FindOrFullyLoad),
					FText::FromStringTable(AnswersStringTableID,LAnswersKeys[i+2], EStringTableLoadingPolicy::FindOrFullyLoad),
					FText::FromStringTable(AnswersStringTableID,LAnswersKeys[i+3], EStringTableLoadingPolicy::FindOrFullyLoad)};
				TArray<FString> LCurrentMetaData = TArray<FString>{
					LMetaDataFromAnswersStringTable[i], LMetaDataFromAnswersStringTable[i+1],
				LMetaDataFromAnswersStringTable[i+2], LMetaDataFromAnswersStringTable[i+3]};
				if (!LCurrentMetaData.Find("1", LNewQuestionChooseText.RightAnswer))
				{
					LNewQuestionChooseText.RightAnswer = -1;
				}
				LNewDataTable->AddRow(*LQuestionsKeys[j], LNewQuestionChooseText);
			}
			break;
		}
		case EQuestionType::Shot:
		{
			LNewDataTable->RowStruct = FQuestionShot::StaticStruct();
			//TODO
			break;
		}
		default: break;
	}
	
	//Save DT as an asset
	FString PackagePath = FString("/Game/BattleMinds/Data/").Append(InTableName);
	UPackage* Package = CreatePackage(*PackagePath);

	if (!Package)
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Failed to create package: %s"), *PackagePath);
		return;
	}

	// Rename the DataTable to be inside the package
	LNewDataTable->Rename(*FPaths::GetBaseFilename(PackagePath), Package);
	LNewDataTable->SetFlags(RF_Public | RF_Standalone);
	Package->MarkPackageDirty();

	// Register the asset with Asset Registry
	FAssetRegistryModule::AssetCreated(LNewDataTable);

	// Step 4: Save the package to disk (Editor Only)
	#if WITH_EDITOR
	FString FilePath = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, LNewDataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
    
	if (bSaved)
	{
		UE_LOG(LogBMEditorDataUtils, Warning, TEXT("DataTable asset successfully saved at %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogBMEditorDataUtils, Error, TEXT("Failed to save DataTable asset."));
	}
	#endif
}