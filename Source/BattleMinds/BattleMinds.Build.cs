// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BattleMinds : ModuleRules
{
	public BattleMinds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "ApplicationCore", "InputCore", "CoreOnline", "NetCore", "EnhancedInput", "UniversalCameraPlugin",
			"GameplayTags","GameSettings", "ModularGameplayActors"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"OnlineSubsystemSteam", 
			"StructUtils", "AdvancedSessions", "UMGEditor", "CommonUI", "AudioModulation",
			"RHI", "DeveloperSettings", "CommonInput", "CommonGame", "CommonUser", "Slate", "SlateCore", "AudioMixer", "EngineSettings"
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"BattleMinds/"
		});
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");
		

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
