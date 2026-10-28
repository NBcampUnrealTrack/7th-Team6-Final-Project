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
 * 허브 키 토큰
 *  {Interact} → 상호작용 키 (상점 등, 기본 Enter)
 *  {Locker}   → 사물함 키 (기본 I)
 * 예) "상호작용: {Interact} 키" → "상호작용: Enter 키"
 *
 * {Player} 토큰은 현재 프로필 닉네임으로 치환된다.
 * 예) "{Player}, 오늘도 잘 부탁해!" → "람다, 오늘도 잘 부탁해!"
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

	/** 허브 상호작용 키 이름 (상점 등). 여러 개 지정돼 있으면 첫 번째 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static FText GetInteractKeyLabel(const UObject* WorldContextObject);

	/** 허브 사물함 키 이름 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static FText GetLockerKeyLabel(const UObject* WorldContextObject);

	/**
	 * 이 키가 허브 상호작용 키인지.
	 * BP에서 Enter를 직접 비교하던 곳(미니게임 입구 등)은 이걸로 바꿔두면 키 변경 시 같이 따라감
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static bool IsInteractKey(const UObject* WorldContextObject, const FKey& Key);

	/** 현재 프로필 닉네임. 프로필이 없거나 닉네임이 비면 기본 호칭 */
	UFUNCTION(BlueprintPure, Category = "PTB|Text", meta = (WorldContext = "WorldContextObject"))
	static FText GetPlayerName(const UObject* WorldContextObject);

	/** {ActionA}~{ActionE}, {Interact}, {Locker} → 현재 키 이름, {Player} → 프로필 닉네임으로 치환 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input", meta = (WorldContext = "WorldContextObject"))
	static FText FormatKeyTokens(const UObject* WorldContextObject, const FText& SourceText);
};
