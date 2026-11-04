// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// 외부 평가용(현업 피드백·설문) 빌드 타깃: STOVE SDK 미포함, 치트 제외.
// STOVE 클라이언트가 없는 PC에서도 실행됨. 판매용(STOVE) 빌드는 PartTimeBeat 타깃을 사용.
public class PartTimeBeatEvalTarget : TargetRules
{
	public PartTimeBeatEvalTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("PartTimeBeat");
	}
}
