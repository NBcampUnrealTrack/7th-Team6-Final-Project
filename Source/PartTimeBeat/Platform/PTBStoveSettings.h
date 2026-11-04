#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PTBStoveSettings.generated.h"

UENUM()
enum class EPTBStoveEnvironment : uint8
{
	Sandbox UMETA(DisplayName = "SANDBOX (개발·QA)"),
	Live    UMETA(DisplayName = "LIVE (실서비스)")
};

/**
 * STOVE PC SDK 연동 설정.
 * 에디터: Edit → Project Settings → Game → "PTB STOVE 연동"
 * 저장 위치: Config/DefaultGame.ini
 *
 * SDK 파일(Source/ThirdParty/StovePCSDK)이 없으면 이 설정과 무관하게 STOVE 기능은 꺼진 채 빌드됨
 *
 * Game ID / App Key는 공개 저장소에 올라가지 않도록 별도 설정(PTB STOVE 키, Config/DefaultStoveKeys.ini)에 저장
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB STOVE 연동"))
class PARTTIMEBEAT_API UPTBStoveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** STOVE 연동 사용 (STOVE 런처로 배포하는 빌드에서 켬. Steam 빌드 등에서는 끔) */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE")
	bool bEnableStove = false;

	/**
	 * 에디터(PIE)에서도 연동 시도.
	 * 기본 꺼짐: 런처 없이 실행되면 SDK가 런처로 재실행을 요구하므로 에디터 테스트용으로는 부적합
	 */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE", meta = (EditCondition = "bEnableStove"))
	bool bEnableInEditor = false;

	/** 개발 빌드(Development 등) 환경 */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE", meta = (EditCondition = "bEnableStove"))
	EPTBStoveEnvironment DevelopmentEnvironment = EPTBStoveEnvironment::Sandbox;

	/** Shipping 빌드 환경 (Live 전환 시 여기만 LIVE로) */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE", meta = (EditCondition = "bEnableStove"))
	EPTBStoveEnvironment ShippingEnvironment = EPTBStoveEnvironment::Sandbox;

	/** 런처 실행 여부 확인 대기 시간 (ms) */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE|Advanced", meta = (EditCondition = "bEnableStove", ClampMin = "1000"))
	int32 RestartCheckWaitMs = 60000;

	/** 런처로 실행되지 않았을 때 STOVE 런처를 자동으로 띄움 */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE|Advanced", meta = (EditCondition = "bEnableStove"))
	bool bLaunchStoveLauncher = true;

	/** 초기화 실패 시 게임 종료 (Shipping 빌드에서만 적용. 개발 빌드는 로그만 남기고 계속) */
	UPROPERTY(Config, EditAnywhere, Category = "STOVE|Advanced", meta = (EditCondition = "bEnableStove"))
	bool bQuitOnInitFailureInShipping = true;

	// ── 현재 빌드 기준 값 ──
	EPTBStoveEnvironment GetActiveEnvironment() const
	{
#if UE_BUILD_SHIPPING
		return ShippingEnvironment;
#else
		return DevelopmentEnvironment;
#endif
	}
	FString GetActiveEnvironmentString() const { return GetActiveEnvironment() == EPTBStoveEnvironment::Live ? TEXT("LIVE") : TEXT("SANDBOX"); }
	FString GetActiveGameId() const;
	FString GetActiveAppKey() const;
};

/**
 * STOVE Game ID / App Key (저장소에 올리지 않는 값).
 * 에디터: Edit → Project Settings → Game → "PTB STOVE 키 (커밋 금지)"
 * 저장 위치: Config/DefaultStoveKeys.ini → .gitignore로 제외. 패키징하는 PC에만 입력
 * (패키징 시 Config 폴더의 ini는 빌드에 포함되므로 게임에서는 정상적으로 읽힘)
 */
UCLASS(Config = StoveKeys, DefaultConfig, meta = (DisplayName = "PTB STOVE 키 (커밋 금지)"))
class PARTTIMEBEAT_API UPTBStoveKeySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** STOVE 스튜디오의 Game ID (Sandbox) */
	UPROPERTY(Config, EditAnywhere, Category = "Sandbox")
	FString SandboxGameId;

	/** STOVE 스튜디오의 Application Key (Sandbox) */
	UPROPERTY(Config, EditAnywhere, Category = "Sandbox")
	FString SandboxAppKey;

	/** STOVE 스튜디오의 Game ID (Live) */
	UPROPERTY(Config, EditAnywhere, Category = "Live")
	FString LiveGameId;

	/** STOVE 스튜디오의 Application Key (Live) */
	UPROPERTY(Config, EditAnywhere, Category = "Live")
	FString LiveAppKey;
};
