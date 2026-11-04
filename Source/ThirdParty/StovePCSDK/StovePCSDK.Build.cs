// STOVE PC SDK 3.5 (Full, C API) - 외부(바이너리) 모듈
// 폴더 구성 = STOVE에서 받은 PCSDK_Full_C zip 구조 그대로
//   Include/   : SDK 헤더 (stove_api.h 등)
//   lib/x64/   : BaseSDK.lib
//   dll/x64/   : BaseSDK.dll (빌드·패키징 시 실행 파일 옆으로 자동 복사)
// SDK 버전을 올릴 때는 새 zip의 압축을 풀어 이 폴더에 그대로 덮어쓰면 됨 (이 .cs 파일만 유지)

using System.IO;
using UnrealBuildTool;

public class StovePCSDK : ModuleRules
{
	public StovePCSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform != UnrealTargetPlatform.Win64)
		{
			return;
		}

		string IncludeDir = Path.Combine(ModuleDirectory, "Include");
		string LibPath    = Path.Combine(ModuleDirectory, "lib", "x64", "BaseSDK.lib");
		string DllPath    = Path.Combine(ModuleDirectory, "dll", "x64", "BaseSDK.dll");

		PublicSystemIncludePaths.Add(IncludeDir);
		PublicAdditionalLibraries.Add(LibPath);

		// 지연 로드: 게임 코드에서 DLL을 직접 로드한 뒤 사용 (DLL이 없어도 실행 파일은 뜸)
		PublicDelayLoadDLLs.Add("BaseSDK.dll");

		// 에디터·패키징 빌드 모두 실행 파일 폴더(Binaries/Win64)로 복사
		RuntimeDependencies.Add("$(BinaryOutputDir)/BaseSDK.dll", DllPath);
	}
}
