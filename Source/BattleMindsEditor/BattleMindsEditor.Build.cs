// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BattleMindsEditor : ModuleRules
{
	public BattleMindsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore"});

		PrivateDependencyModuleNames.AddRange(new string[] { "BattleMinds", "Slate", "SlateCore", "StructUtils", "UnrealEd" });

		PublicIncludePaths.AddRange(new string[]
		{
		});
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
