// Battle Minds, 2022. All rights reserved.


#include "Developer/BM_TestCreateWidget.h"
#include "WidgetBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"

UUserWidget* UBM_TestCreateWidget::CreateChosenWidget_Implementation()
{
	return nullptr;
}

void UBM_TestCreateWidget::CreateWidgetsList(FString Path)
{
	WidgetClassMap.Empty();
	// Доступ к реестру ассетов
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.PackagePaths.Add(*Path); // Указываем папку
	Filter.ClassPaths.Add(UWidgetBlueprint::StaticClass()->GetClassPathName()); // Только виджеты
	Filter.bRecursivePaths = true; // Поиск во вложенных папках

	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	for (const FAssetData& Asset : AssetData)
	{
		if (UBlueprint* WidgetBP = Cast<UBlueprint>(Asset.GetAsset()))
		{
			if (UClass* GeneratedClass = WidgetBP->GeneratedClass)
			{
				if (GeneratedClass->IsChildOf(UUserWidget::StaticClass()))
				{
					WidgetClassMap.Add(WidgetBP->GetName(), GeneratedClass);
				}
			}
		}
	}
}