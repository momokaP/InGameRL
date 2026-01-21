// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InGameRL : ModuleRules
{
	public InGameRL(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
            "LearningAgents",
            "LearningAgentsTraining",
            "Learning",
            "LearningTraining",
            "Sockets",
            "Networking",
            "Json",
			"JsonUtilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        PublicIncludePaths.AddRange(new string[] {
			"InGameRL"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
