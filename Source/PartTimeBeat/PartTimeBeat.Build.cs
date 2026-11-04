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
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
			"AkAudio",
			"WwiseSoundEngine",
			"CableComponent",
			"Niagara",
			"LevelSequence",
			"MovieScene",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// 외부 평가용 빌드 (PartTimeBeatEval 타깃): STOVE 미연동 + 치트 제외 → STOVE 클라이언트 없는 PC에서도 실행
		bool bEvalBuild = Target.Name == "PartTimeBeatEval";
		PublicDefinitions.Add("PTB_EVAL_BUILD=" + (bEvalBuild ? "1" : "0"));

		// STOVE PC SDK: Source/ThirdParty/StovePCSDK 폴더에 SDK가 있을 때만 연동 (없으면 STOVE 기능 없이 빌드)
		string StoveSDKDir = System.IO.Path.Combine(ModuleDirectory, "..", "ThirdParty", "StovePCSDK");
		bool bHasStoveSDK = !bEvalBuild
			&& Target.Platform == UnrealTargetPlatform.Win64
			&& System.IO.File.Exists(System.IO.Path.Combine(StoveSDKDir, "Include", "stove_api.h"))
			&& System.IO.File.Exists(System.IO.Path.Combine(StoveSDKDir, "lib", "x64", "BaseSDK.lib"));
		if (bHasStoveSDK)
		{
			PrivateDependencyModuleNames.Add("StovePCSDK");
		}
		PublicDefinitions.Add("WITH_STOVE_SDK=" + (bHasStoveSDK ? "1" : "0"));

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
