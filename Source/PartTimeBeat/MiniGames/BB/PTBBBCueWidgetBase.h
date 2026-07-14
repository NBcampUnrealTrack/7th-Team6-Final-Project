#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBCueWidgetBase.generated.h"

class UTexture2D;

/**
 * BB 미니게임 노트 큐 위젯 베이스.
 *
 * PTBActionCueWidgetBase(키 라벨·색상 제공)를 상속하고
 * BB에서 공통으로 필요한 NoteId 추적과 큐 시작·제거 인터페이스를 추가한다.
 *
 * WBP_BB_AttackCue 등 BB 전용 큐 위젯이 이 클래스를 부모로 사용한다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBBBCueWidgetBase : public UPTBActionCueWidgetBase
{
	GENERATED_BODY()

public:
	UPTBBBCueWidgetBase();

	/** 이 큐가 나타내는 노트 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|BB|Cue")
	int32 GetNoteId() const { return NoteId; }

	/** NoteId·ActionType 저장 후 OnCueStarted 발행. ApproachStartTranslationX가 설정되면 Translation 이동도 시작. */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	virtual void InitCue(int32 InNoteId, EPTBActionType InActionType, float InApproachDurationSec = 0.f);

	/** 즉시 뷰포트에서 제거. 판정 완료·놓침 후 HUD에서 호출. */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	virtual void RemoveCue();

	/** 판정 결과 전달. BP에서 피드백 애니메이션을 재생하고 완료 시 RemoveCue()를 호출. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnJudgementResult(const FPTBJudgementResult& Result);

	/** Translation 이동을 멈추고 히트존(X=0)에 스냅. OnJudgementResult 시작 시 호출. */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	void StopApproachTranslation();

	/** 스폰 시 HUD가 설정하는 초기 X 오프셋. 양수면 보스가 오른쪽. */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|Cue")
	float ApproachStartTranslationX = 0.f;

	// ── 판정 문구 이펙트 ─────────────────────────────────────────

	/** JudgementType에 대응하는 판정 문구 텍스처 반환. OnJudgementResult(BP)에서 판정 이미지 표시에 사용. */
	UFUNCTION(BlueprintPure, Category = "PTB|BB|Cue|Judgement")
	UTexture2D* GetJudgementTexture(EPTBJudgementType JudgementType) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Cue|Judgement")
	TObjectPtr<UTexture2D> HighPerfectTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Cue|Judgement")
	TObjectPtr<UTexture2D> PerfectTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Cue|Judgement")
	TObjectPtr<UTexture2D> GoodTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Cue|Judgement")
	TObjectPtr<UTexture2D> MissTexture;

protected:
	/** InitCue() 호출 후 BP에서 접근 애니메이션 시작 등 처리. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueStarted(int32 InNoteId, EPTBActionType InActionType, float ApproachDurationSec);

	/** RemoveCue() 호출 후 BP에서 페이드아웃 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueRemoved();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "PTB|BB|Cue")
	int32 NoteId = 0;

private:
	bool bApproaching = false;
	float ApproachElapsedSec = 0.f;
	float ApproachTotalDurationSec = 0.f;
};
