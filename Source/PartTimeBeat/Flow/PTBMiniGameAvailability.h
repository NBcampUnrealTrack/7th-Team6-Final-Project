// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PTBMiniGameAvailability.generated.h"

/** 미니게임 공개 상태 */
UENUM(BlueprintType)
enum class EPTBMiniGameAvailability : uint8
{
	Available   UMETA(DisplayName = "출시 (플레이 가능)"),
	ComingSoon  UMETA(DisplayName = "개발중 (Coming Soon 표시, 플레이 불가)"),
	Hidden      UMETA(DisplayName = "숨김 (아예 안 보이게)")
};

/** 미니게임 하나의 공개 설정 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBMiniGameAvailabilityEntry
{
	GENERATED_BODY()

	/** 미니게임 ID (예: FOD, SR) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Availability")
	FName MiniGameId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Availability")
	EPTBMiniGameAvailability Availability = EPTBMiniGameAvailability::ComingSoon;

	/** 플레이어에게 보여줄 문구 (비우면 기본 문구) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Availability", meta = (MultiLine = true))
	FText Message;

	/** 내부 메모 (비활성화 사유, 남은 작업 등). 게임에는 안 나옴 */
	UPROPERTY(EditAnywhere, Category = "PTB|Availability", meta = (MultiLine = true))
	FString InternalNote;
};

/**
 * 미니게임 공개/비활성화 관리.
 * 에디터: Edit → Project Settings → Game → "PTB 미니게임 공개 설정"
 * 저장 위치: Config/DefaultGame.ini (git으로 변경 이력 관리)
 * 목록에 없는 미니게임은 전부 "출시" 취급
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB 미니게임 공개 설정"))
class PARTTIMEBEAT_API UPTBMiniGameAvailabilitySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPTBMiniGameAvailabilitySettings();

	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** 출시가 아닌 미니게임 목록 */
	UPROPERTY(Config, EditAnywhere, Category = "Availability", meta = (TitleProperty = "MiniGameId"))
	TArray<FPTBMiniGameAvailabilityEntry> MiniGames;

	/** Message를 비워둔 개발중 미니게임에 보여줄 기본 문구 */
	UPROPERTY(Config, EditAnywhere, Category = "Availability", meta = (MultiLine = true))
	FText DefaultComingSoonMessage;

	/** 에디터(PIE)에서는 개발중 미니게임도 플레이 허용 (개발/테스트용, 패키징 빌드에는 영향 없음) */
	UPROPERTY(Config, EditAnywhere, Category = "Availability")
	bool bAllowLockedMiniGamesInEditor = false;

	const FPTBMiniGameAvailabilityEntry* FindEntry(FName MiniGameId) const;
};

/** BP용: 미니게임 공개 상태 조회 */
UCLASS()
class PARTTIMEBEAT_API UPTBMiniGameAvailabilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 공개 상태 (목록에 없으면 Available) */
	UFUNCTION(BlueprintPure, Category = "PTB|Availability")
	static EPTBMiniGameAvailability GetMiniGameAvailability(FName MiniGameId);

	/** 지금 플레이할 수 있는지 (출시 상태, 또는 에디터 테스트 허용 시) */
	UFUNCTION(BlueprintPure, Category = "PTB|Availability")
	static bool IsMiniGamePlayable(FName MiniGameId);

	/** 개발중(Coming Soon) 상태인지. UI에서 안내 표시용 (에디터 테스트 허용과 무관하게 설정값 그대로) */
	UFUNCTION(BlueprintPure, Category = "PTB|Availability")
	static bool IsMiniGameComingSoon(FName MiniGameId);

	/** 숨김 상태인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|Availability")
	static bool IsMiniGameHidden(FName MiniGameId);

	/** 개발중 안내 문구 (항목 문구 > 기본 문구) */
	UFUNCTION(BlueprintPure, Category = "PTB|Availability")
	static FText GetMiniGameLockMessage(FName MiniGameId);
};
