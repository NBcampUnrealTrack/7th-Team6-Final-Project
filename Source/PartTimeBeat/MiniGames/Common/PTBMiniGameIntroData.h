#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Core/PTBStructEnums.h"
#include "PTBMiniGameIntroData.generated.h"

class UTexture2D;
class UPTBRhythmChartAsset;

/**
 * 미니게임 본게임 전후에 재생되는 씬 단계.
 * 흐름: 로딩 → IntroStory → Tutorial → Ready(Press Any Key) → 본게임 → OutroStory → 결과
 */
UENUM(BlueprintType)
enum class EPTBMiniGameScenePhase : uint8
{
	None        UMETA(DisplayName = "없음"),
	IntroStory  UMETA(DisplayName = "시작 스토리"),
	Tutorial    UMETA(DisplayName = "튜토리얼"),
	Ready       UMETA(DisplayName = "시작 대기"),
	OutroStory  UMETA(DisplayName = "결과 직전 스토리")
};

/** 스탠딩 이미지 위치 */
UENUM(BlueprintType)
enum class EPTBPortraitSlot : uint8
{
	Center  UMETA(DisplayName = "가운데"),
	Left    UMETA(DisplayName = "왼쪽"),
	Right   UMETA(DisplayName = "오른쪽")
};

/** 스토리 대사 한 줄 (스탠딩 이미지 + 다이얼로그) */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBDialogueLine : public FTableRowBase
{
	GENERATED_BODY()

	/** 화자 이름. 비우면 이름표 숨김(나레이션) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	FText Speaker;

	/** 스탠딩 이미지. 비우면 해당 위치의 이전 이미지 유지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** 스탠딩 이미지 위치 = 이 대사의 화자 위치. 이 위치는 밝게, 나머지는 어둡게 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	EPTBPortraitSlot PortraitSlot = EPTBPortraitSlot::Center;

	/** 체크하면 이 대사에서 화자/청자 위치를 뺀 나머지 스탠딩 이미지를 지움 (퇴장/장면 전환/혼잣말) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	bool bClearOtherPortraits = false;

	/** 듣는 사람 스탠딩 이미지 (선택). 화자가 아니므로 어둡게 표시됨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	TSoftObjectPtr<UTexture2D> ListenerPortrait;

	/** 듣는 사람 위치. ListenerPortrait가 있을 때만 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	EPTBPortraitSlot ListenerSlot = EPTBPortraitSlot::Right;

	/** 배경 그림. 비우면 이전 배경 유지 (장면이 바뀌는 줄에만 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue")
	TSoftObjectPtr<UTexture2D> Background;

	/** 대사. {ActionA}~{ActionE} 토큰은 현재 키 바인딩으로 치환됨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dialogue", meta = (MultiLine = true))
	FText Text;
};

/** 튜토리얼 한 스텝 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBTutorialStep : public FTableRowBase
{
	GENERATED_BODY()

	/** 안내 문구. {ActionA}~{ActionE} 토큰은 현재 키 바인딩으로 치환됨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial", meta = (MultiLine = true))
	FText Instruction;

	/** 다음 스텝 진행에 필요한 입력. None이면 다음 버튼/아무 키로 진행 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial")
	EPTBActionType RequiredAction = EPTBActionType::None;

	/** RequiredAction 입력 필요 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	/** 시범 이미지 (선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial")
	TSoftObjectPtr<UTexture2D> DemoImage;

	// ── 연습 (설명 → 실제 플레이) ────────────────────────────────
	// PracticePattern 또는 PracticeChartOverride가 있으면 "연습 스텝":
	// 설명 표시(채보 정지) → 시작 → 실제 미니게임으로 짧은 채보 플레이 → 통과 시 다음, 실패 시 재도전

	/**
	 * 연습 패턴. 공백으로 구분, A~E = ActionA~E 노트, - = 쉼표(빈 박)
	 * 예) "A A A A" / "A B C B A" / "A - B - C"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice")
	FString PracticePattern;

	/** 연습 템포 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (ClampMin = "30.0"))
	float PracticeBPM = 100.0f;

	/** 패턴 한 칸 사이 간격(박). 1 = 4분음표, 0.5 = 8분음표 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (ClampMin = "0.25"))
	float PracticeBeatInterval = 1.0f;

	/** 첫 노트 전 준비 박 수 (메트로놈으로 박자 감 잡는 구간) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (ClampMin = "0"))
	int32 PracticeLeadInBeats = 4;

	/** 통과에 필요한 Good 이상 판정 수. 0이면 전부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (ClampMin = "0"))
	int32 PracticeRequiredHits = 0;

	/** 통과 시 문구 (비우면 기본 문구) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (MultiLine = true))
	FText PracticeSuccessText;

	/** 실패 시 문구 (비우면 기본 문구). 다시 시작하면 같은 연습을 반복 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice", meta = (MultiLine = true))
	FText PracticeRetryText;

	/** 패턴으로 표현하기 어려운 경우(롱노트 등) 기존 채보 에셋을 대신 사용. 지정 시 패턴 무시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Tutorial|Practice")
	TSoftObjectPtr<UPTBRhythmChartAsset> PracticeChartOverride;

	/** 연습 스텝인지 */
	bool IsPracticeStep() const
	{
		return !PracticePattern.TrimStartAndEnd().IsEmpty() || !PracticeChartOverride.IsNull();
	}
};

/**
 * 미니게임별 스토리/튜토리얼 데이터.
 *
 * - 테이블이 비어 있으면 GameInstance의 DefaultIntroData(공용 템플릿)로 폴백 (OutroStory 제외)
 * - Version을 올리면 이미 본 유저에게도 한 번 더 자동재생됨 (플레이스홀더 → 실제 내용 교체 시)
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBMiniGameIntroData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 시작 스토리 (Row: FPTBDialogueLine) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|IntroStory",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/PartTimeBeat.PTBDialogueLine"))
	TObjectPtr<UDataTable> IntroStory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|IntroStory", meta = (ClampMin = "0"))
	int32 IntroStoryVersion = 0;

	/** 튜토리얼 연습 중 매 박 재생할 메트로놈 SFX 키 (RuleSet AudioEventSet의 키). 비우면 틱 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|Tutorial")
	FName TutorialMetronomeSFXKey = NAME_None;

	/** 튜토리얼 (Row: FPTBTutorialStep) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|Tutorial",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/PartTimeBeat.PTBTutorialStep"))
	TObjectPtr<UDataTable> Tutorial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|Tutorial", meta = (ClampMin = "0"))
	int32 TutorialVersion = 0;

	/** 결과 직전 스토리 (Row: FPTBDialogueLine). 없으면 바로 결과로 (기본 템플릿 폴백 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|OutroStory",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/PartTimeBeat.PTBDialogueLine"))
	TObjectPtr<UDataTable> OutroStory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|OutroStory", meta = (ClampMin = "0"))
	int32 OutroStoryVersion = 0;

	/** true면 클리어(실패 아님)했을 때만 결과 직전 스토리 재생 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Scene|OutroStory")
	bool bOutroRequiresClear = true;

	/** 단계별 테이블 */
	UDataTable* GetTable(EPTBMiniGameScenePhase Phase) const;

	/** 단계별 버전 */
	int32 GetVersion(EPTBMiniGameScenePhase Phase) const;

	/** 테이블에 행이 1개 이상 있는지 */
	bool HasContent(EPTBMiniGameScenePhase Phase) const;

	/** 프로필 저장 키. 예: "JJ.Tutorial" */
	static FName MakeSceneKey(FName MiniGameId, EPTBMiniGameScenePhase Phase);
};
