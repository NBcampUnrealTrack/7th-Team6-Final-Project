#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBProgressSettings.generated.h"

class UDataTable;
class UUserWidget;
class UPTBAchievementBackend;

/**
 * 진행·해금·업적·QA 치트 설정.
 * 에디터: Edit → Project Settings → Game → "PTB 진행·업적 설정"
 * 저장 위치: Config/DefaultGame.ini
 *
 * 패키징 주의: AchievementTable이 있는 폴더를 Additional Asset Directories to Cook에 추가할 것
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB 진행·업적 설정"))
class PARTTIMEBEAT_API UPTBProgressSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	// ── Insane 해금 (미니게임별) ──
	// 아래 조건 중 하나라도 만족하면 그 미니게임의 Insane 해금

	/** Insane을 처음에 잠금 상태로 시작 */
	UPROPERTY(Config, EditAnywhere, Category = "Insane Unlock")
	bool bLockInsaneByDefault = true;

	/** Easy 이상 플레이 횟수 (결과 화면까지 간 판, 성공·실패 모두) */
	UPROPERTY(Config, EditAnywhere, Category = "Insane Unlock", meta = (EditCondition = "bLockInsaneByDefault", ClampMin = "0"))
	int32 InsaneUnlockPlayCount = 30;

	/** Easy 이상 클리어 횟수 */
	UPROPERTY(Config, EditAnywhere, Category = "Insane Unlock", meta = (EditCondition = "bLockInsaneByDefault", ClampMin = "0"))
	int32 InsaneUnlockClearCount = 15;

	/** Standard에서 이 등급 이상 (PerfectFullCombo > FullCombo > S > A ...) */
	UPROPERTY(Config, EditAnywhere, Category = "Insane Unlock", meta = (EditCondition = "bLockInsaneByDefault"))
	EPTBGradeType InsaneUnlockStandardGrade = EPTBGradeType::S;

	/** 진행 관련 대상 미니게임 (치트의 "전체 Insane 해금" 등에서 사용) */
	UPROPERTY(Config, EditAnywhere, Category = "Insane Unlock")
	TArray<FName> ProgressMiniGameIds = { TEXT("BB"), TEXT("CH"), TEXT("DW"), TEXT("FS"), TEXT("JJ"), TEXT("LC"), TEXT("PC") };

	// ── 업적 ──

	/** 업적 정의 DataTable (Row Structure: PTBAchievementRow). Row Name = 업적 ID (세이브·플랫폼 연동 키, 출시 후 변경 금지) */
	UPROPERTY(Config, EditAnywhere, Category = "Achievement", meta = (RequiredAssetDataTags = "RowStructure=/Script/PartTimeBeat.PTBAchievementRow"))
	TSoftObjectPtr<UDataTable> AchievementTable;

	/**
	 * 플랫폼 업적 연동 클래스 (STOVE / Steam).
	 * 비우면 로그만 남기는 기본 클래스 사용. 플랫폼 SDK를 붙일 때 PTBAchievementBackend를 상속해 구현
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Achievement")
	TSoftClassPtr<UPTBAchievementBackend> AchievementBackendClass;

	// ── QA 치트 (Development 빌드 전용, Shipping에서는 코드가 빠짐) ──

	/** 치트 패널을 여닫는 키 */
	UPROPERTY(Config, EditAnywhere, Category = "QA Cheat")
	FKey CheatPanelKey = EKeys::F10;

	/** 치트 패널 위젯 (버튼에서 PTB Cheat 함수들을 호출하도록 꾸민 WBP) */
	UPROPERTY(Config, EditAnywhere, Category = "QA Cheat")
	TSoftClassPtr<UUserWidget> CheatPanelWidgetClass;
};
