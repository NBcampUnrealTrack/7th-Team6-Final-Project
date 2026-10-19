#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Core/PTBStructEnums.h"
#include "PTBMiniGameIntroData.generated.h"

class UTexture2D;

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
