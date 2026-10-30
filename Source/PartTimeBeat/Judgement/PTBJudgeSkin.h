#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgeSkin.generated.h"

class UTexture2D;

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
