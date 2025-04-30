// Battle Minds, 2022. All rights reserved.


#include "Utils/BMGameUtilsLibrary.h"

bool UBMGameUtilsLibrary::IsEditorWorldContext(UObject* InWorldContext)
{
	UWorld* LWorld = InWorldContext? InWorldContext->GetWorld() : nullptr;
	return IsValid(LWorld) && LWorld->WorldType == EWorldType::Editor;
}