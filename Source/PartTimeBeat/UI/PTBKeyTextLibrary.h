#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/PTBStructEnums.h"
#include "PTBKeyTextLibrary.generated.h"

/**
 * 키 바인딩 기반 텍스트 유틸.
 *
 * 스토리/튜토리얼/HUD 문구에 {ActionA}~{ActionE} 토큰을 쓰면
 * 표시 시점의 실제 키 이름(CachedSettings.RhythmKeys)으로 치환된다.
 * 예) "{ActionA} 키를 눌러 점프!" → "Z 키를 눌러 점프!"
 *
 * 표시할 때마다 치환하므로 키 설정을 바꾸면 다음 표시부터 바로 반영된다.
 * FText::Format 기반이라 번역 텍스트에서도 토큰이 그대로 동작한다.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBKeyTextLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Action에 바인딩된 키 이름. 바인딩을 못 찾으면 기본값(Z/X/C/V/B) */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static FText GetActionKeyLabel(const UObject* WorldContextObject, EPTBActionType Action);

	/** {ActionA}~{ActionE} 토큰을 현재 키 이름으로 치환 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static FText FormatKeyTokens(const UObject* WorldContextObject, const FText& SourceText);
};
