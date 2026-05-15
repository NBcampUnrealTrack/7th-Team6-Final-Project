// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PartTimeBeat : ModuleRules
{
	public PartTimeBeat(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PartTimeBeat",
			"PartTimeBeat/Variant_Platforming",
			"PartTimeBeat/Variant_Platforming/Animation",
			"PartTimeBeat/Variant_Combat",
			"PartTimeBeat/Variant_Combat/AI",
			"PartTimeBeat/Variant_Combat/Animation",
			"PartTimeBeat/Variant_Combat/Gameplay",
			"PartTimeBeat/Variant_Combat/Interfaces",
			"PartTimeBeat/Variant_Combat/UI",
			"PartTimeBeat/Variant_SideScrolling",
			"PartTimeBeat/Variant_SideScrolling/AI",
			"PartTimeBeat/Variant_SideScrolling/Gameplay",
			"PartTimeBeat/Variant_SideScrolling/Interfaces",
			"PartTimeBeat/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
