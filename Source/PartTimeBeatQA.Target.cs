// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// QA용 빌드 타깃: STOVE SDK 미포함, 치트(F10 패널·PTB.Cheat.*) 사용 가능.
// Development로 패키징해서 STOVE 클라이언트 없는 PC에서 테스트. 판매용은 PartTimeBeat, 외부 평가용은 PartTimeBeatEval.
public class PartTimeBeatQATarget : TargetRules
{
	public PartTimeBeatQATarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("PartTimeBeat");
	}
}
