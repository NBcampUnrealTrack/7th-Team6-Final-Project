#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgeSkin.generated.h"

class UTexture2D;

/** 공통 판정 표시 위치 종류 */
UENUM(BlueprintType)
enum class EPTBJudgementAnchorType : uint8
{
	Default        UMETA(DisplayName = "기본 (RuleSet 화면 위치)"),
	WorldLocation  UMETA(DisplayName = "월드 위치 (캐릭터 머리 위 등, 따라다님)"),
	ScreenPosition UMETA(DisplayName = "화면 위치 (UI 노트 위치 등, 판정 순간 고정)")
};

/** 공통 판정 표시 위치 (미니게임의 GetJudgementDisplayAnchor가 반환) */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBJudgementAnchor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Judgement")
	EPTBJudgementAnchorType Type = EPTBJudgementAnchorType::Default;

	/** Type=WorldLocation일 때. RuleSet의 JudgementWorldOffset이 더해짐 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Judgement")
	FVector WorldLocation = FVector::ZeroVector;

	/**
	 * Type=ScreenPosition일 때. 뷰포트 위치 (Set Position in Viewport의 Remove DPI Scale=false 기준 단위).
	 * 위젯 위치를 쓰려면 Make Judgement Anchor From Widget 사용. RuleSet의 JudgementScreenAnchorOffset이 더해짐
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Judgement")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	/** 동시에 따로 보여줄 표시 번호 (예: 레인/캐릭터 인덱스). 같은 번호면 이전 표시를 덮어씀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Judgement")
	int32 SlotIndex = 0;
};

/**
 * 판정 스킨 (Perfect / Good / Miss 등을 어떤 이미지와 연출로 보여줄지).
 *
 * 스킨 하나 = 이 에셋 하나. DT_PTBItems의 JudgeSkin 행 Asset에 연결하면 상점에서 구매·장착 가능.
 * 생성: 콘텐츠 브라우저 우클릭 → Miscellaneous → Data Asset → PTBJudgeSkin
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBJudgeSkin : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 판정별 이미지. 없는 판정은 표시하지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin")
	TMap<EPTBJudgementType, TObjectPtr<UTexture2D>> Textures;

	/** 화면에 표시할 이미지 크기 (px, 1080p 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin")
	FVector2D ImageSize = FVector2D(320.f, 120.f);

	/** 이미지 색 곱하기 (기본 흰색 = 원본 그대로) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin")
	FLinearColor Tint = FLinearColor::White;

	// ── 연출 ──

	/** 전체 표시 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin|Animation", meta = (ClampMin = "0.05"))
	float DisplayDuration = 0.6f;

	/** 등장 시 시작 크기 배율 (1보다 크면 커졌다가 줄어드는 팝 연출) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin|Animation", meta = (ClampMin = "0.1"))
	float PopStartScale = 1.4f;

	/** 시작 크기 → 원래 크기로 줄어드는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin|Animation", meta = (ClampMin = "0.0"))
	float PopInTime = 0.08f;

	/** 사라질 때 페이드아웃 시간 (초, DisplayDuration 끝부분에 포함) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin|Animation", meta = (ClampMin = "0.0"))
	float FadeOutTime = 0.2f;

	/** 표시되는 동안 위로 떠오르는 거리 (px) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JudgeSkin|Animation")
	float RiseDistance = 0.f;

	UFUNCTION(BlueprintPure, Category = "PTB|JudgeSkin")
	UTexture2D* GetTexture(EPTBJudgementType JudgementType) const
	{
		const TObjectPtr<UTexture2D>* Found = Textures.Find(JudgementType);
		return Found ? Found->Get() : nullptr;
	}
};
