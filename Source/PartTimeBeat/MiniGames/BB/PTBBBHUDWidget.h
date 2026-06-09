#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBHUDWidget.generated.h"

class APTBBBMiniGame;
class UPTBBBCueWidgetBase;
class UCanvasPanel;
class UProgressBar;
struct FPTBNoteEvent;
struct FPTBJudgementResult;

/**
 * BB 미니게임 HUD 위젯 베이스.
 *
 * TG HUD(WBP_TG_HUD)의 큐 관리 로직을 C++로 구현한 버전.
 *  - 노트 예고(OnBBNoteCue) → 큐 위젯 스폰 → CanvasPanel에 배치
 *  - 판정(OnBBParrySuccess/Fail) → 큐에 판정 피드백 전달 → TMap에서 제거
 *  - HP 변경(OnBBBossHPChanged/PlayerHPChanged) → ProgressBar 자동 업데이트
 *
 * 에디터에서 설정해야 하는 항목:
 *  - TapCueClass / HoldCueClass : 스폰할 큐 위젯 클래스
 *  - AnchorPositions : ActionType별 캔버스 좌표 (Z→ActionA … B→ActionE)
 *
 * Designer 탭에서 반드시 만들어야 하는 위젯:
 *  - CueLayer (CanvasPanel, BindWidget)
 *
 * Designer 탭에서 선택적으로 만들 수 있는 위젯:
 *  - BossHPBar, PlayerHPBar (ProgressBar, BindWidgetOptional)
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBBBHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 미니게임에 HUD를 연결한다.
	 * Level BP의 BeginPlay 또는 미니게임 시작 이벤트에서 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|HUD")
	void BindToMiniGame(APTBBBMiniGame* InMiniGame);

	/**
	 * ActionType에 대응하는 큐 배치 좌표 반환.
	 * 기본 구현은 AnchorPositions 맵을 읽으며, BP에서 재정의해 커스텀 로직을 구현할 수 있다.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|HUD")
	FVector2D GetAnchorCanvasPosition(EPTBActionType ActionType) const;
	virtual FVector2D GetAnchorCanvasPosition_Implementation(EPTBActionType ActionType) const;

	// ── 에디터 설정 ──────────────────────────────────────────────

	/** 탭 노트에 사용할 큐 위젯 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	TSubclassOf<UPTBBBCueWidgetBase> TapCueClass;

	/** 홀드 노트에 사용할 큐 위젯 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	TSubclassOf<UPTBBBCueWidgetBase> HoldCueClass;

	/**
	 * ActionType별 캔버스 배치 좌표.
	 * GetAnchorCanvasPosition의 기본 구현이 이 값을 읽는다.
	 * 에디터 Details 패널에서 각 키(ActionA~E)마다 화면 좌표를 직접 입력한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	TMap<EPTBActionType, FVector2D> AnchorPositions;

	// ── BindWidget ───────────────────────────────────────────────

	/** 큐 위젯들이 배치되는 캔버스 패널. Designer 탭에 반드시 존재해야 한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CueLayer;

	/** 보스 HP 바. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHPBar;

	/** 플레이어 HP 바. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PlayerHPBar;

	/** 현재 바인딩된 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|HUD")
	TObjectPtr<APTBBBMiniGame> BBMiniGame;

protected:
	virtual void NativeDestruct() override;

	// ── BP 구현 이벤트 ───────────────────────────────────────────

	/** 큐 위젯이 스폰된 직후 호출. BP에서 추가 설정 가능. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnCueSpawned(UPTBBBCueWidgetBase* CueWidget, const FPTBNoteEvent& Note);

	/** 패링 성공 시 호출 (HP 바 자동 갱신 후). BP에서 연출 처리. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnParrySuccessEffect(const FPTBJudgementResult& Result, float BossHPPercent);

	/** 패링 실패 시 호출 (HP 바 자동 갱신 후). BP에서 연출 처리. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnParryFailEffect(const FPTBJudgementResult& Result, float PlayerHPPercent);

	/** 보스 처치 시 호출. BP에서 연출 처리. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnBossDefeatedEffect();

	/** 보스 HP 변경 시 호출 (ProgressBar 자동 갱신 후). BP에서 추가 연출 가능. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnBossHPUpdated(float NewHP, float MaxHP);

	/** 플레이어 HP 변경 시 호출 (ProgressBar 자동 갱신 후). BP에서 추가 연출 가능. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD")
	void OnPlayerHPUpdated(float NewHP, float MaxHP);

private:
	// ── 큐 추적 맵 ───────────────────────────────────────────────

	UPROPERTY()
	TMap<int32, TObjectPtr<UPTBBBCueWidgetBase>> TapCueMap;

	UPROPERTY()
	TMap<int32, TObjectPtr<UPTBBBCueWidgetBase>> HoldCueMap;

	// ── 델리게이트 핸들러 ────────────────────────────────────────

	UFUNCTION()
	void HandleBBNoteCue(FPTBNoteEvent Note);

	UFUNCTION()
	void HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent);

	UFUNCTION()
	void HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent);

	UFUNCTION()
	void HandleBBBossHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleBBPlayerHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleBBBossDefeated();

	// ── 내부 헬퍼 ────────────────────────────────────────────────

	/** 큐 위젯을 스폰·배치하고 InitCue 호출 후 반환. 실패 시 nullptr. */
	UPTBBBCueWidgetBase* SpawnAndPlaceCue(
		TSubclassOf<UPTBBBCueWidgetBase> CueClass,
		const FPTBNoteEvent& Note);

	/** NoteId로 두 맵 중 해당 큐를 찾아 맵에서 제거하고 반환. 없으면 nullptr. */
	UPTBBBCueWidgetBase* FindAndRemoveCue(int32 NoteId);
};
