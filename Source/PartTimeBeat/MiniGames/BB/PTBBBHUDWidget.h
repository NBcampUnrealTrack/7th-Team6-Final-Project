#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBHUDWidget.generated.h"

class APTBBBMiniGame;
class UPTBBBCueWidgetBase;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
struct FPTBNoteEvent;
struct FPTBJudgementResult;

/**
 * BB 미니게임 HUD 위젯 베이스.
 *
 *  - 노트 예고(OnBBNoteCue) → 큐 위젯 스폰 → CanvasPanel에 배치
 *  - 판정(OnBBParrySuccess/Fail) → 큐에 판정 피드백 전달 → TMap에서 제거
 *  - HP 변경(OnBBBossHPChanged/PlayerHPChanged) → ProgressBar 자동 업데이트
 *  - 고스트 바(BossHPGhostBar/PlayerHPGhostBar): 피해 직후 잔상이 남고 GhostBarDecayDelay 후 선형 감소
 *
 * 에디터에서 설정해야 하는 항목:
 *  - TapCueClass / HoldCueClass    : 스폰할 큐 위젯 클래스
 *  - AnchorPositions               : ActionType별 캔버스 좌표 (Z→ActionA … B→ActionE)
 *  - BossSpawnCanvasPosition       : 노트 스폰 시작점 (보스 이미지 중심 좌표)
 *  - GhostBarDecayDelay            : 피해 후 고스트 바 감소 시작까지의 지연(초)
 *  - GhostBarDecaySpeed            : 고스트 바 감소 속도(초당 비율)
 *
 * Designer 탭에서 반드시 만들어야 하는 위젯:
 *  - CueLayer (CanvasPanel, BindWidget)
 *
 * Designer 탭에서 선택적으로 만들 수 있는 위젯:
 *  - BossHPBar, PlayerHPBar (ProgressBar, BindWidgetOptional) — 메인 HP 바
 *  - BossHPGhostBar, PlayerHPGhostBar (ProgressBar, BindWidgetOptional) — 메인 바 뒤에 배치하는 잔상 바
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

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|HUD|Flow")
	void ShowCenterMessage(const FText& Message);

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|HUD|Flow")
	void HideCenterMessage();

	/** ActionType에 대응하는 큐 배치 좌표 반환. 기본 구현은 AnchorPositions 맵을 읽으며 BP에서 재정의 가능. */
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

	/** ActionType별 캔버스 배치 좌표. 에디터 Details 패널에서 각 키(ActionA~E)마다 입력한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	TMap<EPTBActionType, FVector2D> AnchorPositions;

	/** 노트 스폰 시작 X 좌표 (보스 이미지 중심). 큐는 이 X에서 AnchorPositions.X 방향으로 이동한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	FVector2D BossSpawnCanvasPosition = FVector2D(1200.f, 360.f);

	// ── BindWidget ───────────────────────────────────────────────

	/** 큐 위젯들이 배치되는 캔버스 패널. Designer 탭에 반드시 존재해야 한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CueLayer;

	/** 보스 HP 바. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHPBar;

	/** 보스 HP 고스트 바 (피해 잔상). BossHPBar 뒤(ZOrder 낮게)에 배치. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHPGhostBar;

	/** 플레이어 HP 바. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PlayerHPBar;

	/** 플레이어 HP 고스트 바 (피해 잔상). PlayerHPBar 뒤(ZOrder 낮게)에 배치. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PlayerHPGhostBar;

	/** 3, 2, 1, Start!, Finish! 표시용 중앙 텍스트. Designer 탭에 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CenterMessageText;

	/** 피해 발생 후 고스트 바 감소 시작까지의 지연 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|GhostBar", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GhostBarDecayDelay = 0.5f;
	/** 고스트 바 감소 속도 (초당 비율, 1.0 = 1초에 전체 감소) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|GhostBar", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GhostBarDecaySpeed = 1.5f;
	/** 현재 바인딩된 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|HUD")
	TObjectPtr<APTBBBMiniGame> BBMiniGame;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD|Flow")
	void OnCenterMessageShown(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD|Flow")
	void OnCenterMessageHidden();

private:
	/** 현재 미니게임 HP 기준으로 메인 바·고스트 바 상태를 동기화. NativeConstruct/BindToMiniGame 양쪽에서 호출 */
	void SyncBarsToMiniGame();

	// ── 고스트 바 런타임 상태 ────────────────────────────────────

	float BossGhostPercent = 1.0f;
	float BossTargetPercent = 1.0f;
	float BossGhostDecayTimer = 0.0f;

	float PlayerGhostPercent = 1.0f;
	float PlayerTargetPercent = 1.0f;
	float PlayerGhostDecayTimer = 0.0f;

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

	UFUNCTION()
	void HandleBBIntroStarted();

	UFUNCTION()
	void HandleBBGameplayStarted();

	UFUNCTION()
	void HandleBBOutroStarted(FPTBRoundResult Result, EPTBRoundEndReason EndReason);

	UFUNCTION()
	void HandleBBOutroFinished(FPTBRoundResult Result, EPTBRoundEndReason EndReason);

	// ── 내부 헬퍼 ────────────────────────────────────────────────

	/** 큐 위젯을 스폰·배치하고 InitCue 호출 후 반환. 실패 시 nullptr. */
	UPTBBBCueWidgetBase* SpawnAndPlaceCue(
		TSubclassOf<UPTBBBCueWidgetBase> CueClass,
		const FPTBNoteEvent& Note);

	/** NoteId로 두 맵 중 해당 큐를 찾아 맵에서 제거하고 반환. 없으면 nullptr. */
	UPTBBBCueWidgetBase* FindAndRemoveCue(int32 NoteId);

	void ClearCenterMessageTimers();
	void QueueCenterMessage(float DelaySeconds, const FText& Message);
	void ShowCenterMessageForDuration(const FText& Message, float DurationSeconds);

	TArray<FTimerHandle> CenterMessageTimerHandles;
	FTimerHandle CenterMessageHoldTimerHandle;

	// NativeDestruct 이후 GC 전 구간에서 타이머가 발화되지 않도록 막는 플래그
	bool bIsDestructed = false;
};
