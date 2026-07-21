#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBHUDWidget.generated.h"

class APTBBBMiniGame;
class UPTBBBCueWidgetBase;
class UPTBBBHitZoneWidgetBase;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UTexture2D;
struct FPTBNoteEvent;
struct FPTBJudgementResult;

/**
 * BB 미니게임 HUD 위젯 베이스.
 *
 *  - 노트 예고(OnBBNoteCue) → 큐 위젯 스폰 → CanvasPanel에 배치
 *  - 판정(OnBBParrySuccess/Fail) → 큐에 판정 피드백 전달 → TMap에서 제거
 *  - HP 변경(OnBBBossHPChanged/PlayerHPChanged) → ProgressBar 자동 업데이트
 *  - 고스트 바(BossHPGhostBar/PlayerHPGhostBar): 피해 직후 잔상이 남고 GhostBarDecayDelay 후 선형 감소
 *  - 히트존 마커(HitZoneClass): AnchorPositions의 각 위치에 고정 배치되어 입력 타이밍을 안내.
 *    노트가 판정선(OnBBNoteReached)에 도달하면 해당 마커의 PulseHitZone()이 호출된다.
 *
 * 에디터에서 설정해야 하는 항목:
 *  - TapCueClass                   : 스폰할 큐 위젯 클래스 (BB 채보에는 롱노트가 없어 탭 큐만 사용)
 *  - HitZoneClass                  : 각 앵커에 고정 배치할 히트존 마커 위젯 클래스 (미설정 시 스폰 안 함)
 *  - AnchorPositions               : ActionType별 캔버스 좌표 (Z→ActionA … B→ActionE)
 *  - BossSpawnCanvasPosition       : 노트 스폰 시작점 (보스 이미지 중심 좌표)
 *  - GhostBarDecayDelay            : 피해 후 고스트 바 감소 시작까지의 지연(초)
 *  - GhostBarDecaySpeed            : 고스트 바 감소 속도(초당 비율)
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

	/** 탭 노트에 사용할 큐 위젯 클래스 (BB 채보에는 롱노트가 없어 탭 큐만 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Cue")
	TSubclassOf<UPTBBBCueWidgetBase> TapCueClass;

	/**
	 * 각 액션 앵커 위치에 고정 배치할 히트존 마커 위젯 클래스.
	 * 미설정(nullptr) 시 히트존 마커를 스폰하지 않는다(기존 동작과 동일).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|HitZone")
	TSubclassOf<UPTBBBHitZoneWidgetBase> HitZoneClass;

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

	// ── 아웃트로 이미지 ──────────────────────────────────────────

	/** 게임 종료 시 가장 먼저 표시할 결과 이미지 (PTB_BB_Result) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro")
	TObjectPtr<UTexture2D> ResultTexture;

	/**
	 * 결과 등급(0~3)에 따라 이어서 표시할 이미지. 인덱스 0=Outro1 ... 3=Outro4.
	 * 등급 판정(보스 잔여 체력과 무관 — 정확도·미스 수만으로 결정):
	 *           0=Failed(플레이어 체력 0),
	 *           1=Good(채보 완주 + 노트 정확도 GreatAccuracyThreshold 미만),
	 *           2=Great(채보 완주 + 정확도 GreatAccuracyThreshold 이상, 단 미스가 PerfectClearMaxMissCount 초과),
	 *           3=Perfect Clear(채보 완주 + 미스가 PerfectClearMaxMissCount 이하)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro")
	TArray<TObjectPtr<UTexture2D>> OutroTexturesByStar;

	/** Great/Good을 가르는 노트 정확도(AccuracyRate, 0~1) 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GreatAccuracyThreshold = 0.85f;

	/** 퍼펙트 클리어(3등급)로 인정할 최대 미스 허용 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro", meta = (ClampMin = "0"))
	int32 PerfectClearMaxMissCount = 0;

	/** "Finish!" 중앙 메시지를 단독으로 보여주는 시간(초). 이후 결과 이미지로 전환 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro", meta = (ClampMin = "0.0"))
	float FinishMessageSeconds = 0.8f;

	/** 결과 이미지를 보여준 뒤 등급 이미지로 전환하기까지의 지연(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HUD|Outro", meta = (ClampMin = "0.0"))
	float ResultDisplaySeconds = 1.0f;

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

	/** 아웃트로 이미지를 화면에 표시/교체할 때 호출. WBP에서 Image 위젯 갱신 담당. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HUD|Outro")
	void OnShowOutroImage(UTexture2D* Texture);

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
	// BB 채보에는 롱노트가 없어 탭 큐만 추적한다.

	UPROPERTY()
	TMap<int32, TObjectPtr<UPTBBBCueWidgetBase>> TapCueMap;

	// ── 히트존 마커 ──────────────────────────────────────────────

	/** ActionType별 고정 히트존 마커. HitZoneClass가 설정된 경우에만 채워진다. */
	UPROPERTY()
	TMap<EPTBActionType, TObjectPtr<UPTBBBHitZoneWidgetBase>> HitZoneMarkers;

	/** AnchorPositions의 각 ActionType에 대해 히트존 마커를 스폰·배치한다. HitZoneClass 미설정 시 아무 것도 하지 않는다. */
	void SpawnHitZoneMarkers();

	/**
	 * 현재 라운드의 채보에서 실제로 쓰이는 액션만 히트존 마커를 보이게 하고, 나머지는 숨긴다.
	 * 난이도마다 채보에 등장하는 액션 종류가 다르므로(Easy=A~C, Standard=A~D, Insane=A~E 등),
	 * 채보와 무관하게 항상 전체 마커를 보여주는 걸 방지한다. ChartAsset이 아직 로드되지 않았으면
	 * (예: 바인딩 시점이 너무 이르면) 아무 것도 바꾸지 않고 다음 호출(HandleBBGameplayStarted)을 기다린다.
	 */
	void RefreshHitZoneVisibility();

	// ── 델리게이트 핸들러 ────────────────────────────────────────

	UFUNCTION()
	void HandleBBNoteCue(FPTBNoteEvent Note);

	UFUNCTION()
	void HandleBBNoteReached(FPTBNoteEvent Note);

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

	/** NoteId로 TapCueMap에서 해당 큐를 찾아 맵에서 제거하고 반환. 없으면 nullptr. */
	UPTBBBCueWidgetBase* FindAndRemoveCue(int32 NoteId);

	/**
	 * EndReason(Failed=플레이어 체력 0)과 노트 정확도·미스 수로 OutroTexturesByStar의 인덱스(0~3)를 결정.
	 * 보스 잔여 체력은 쓰지 않는다 — 보스 체력은 이제 채보 진행도(정확도 100%가 아니면 완전히
	 * 처치되지 않을 뿐인 진행 지표)라서, 등급은 정확도/미스 수만으로 판단한다.
	 */
	int32 ResolveOutroTierIndex(const FPTBRoundResult& Result, EPTBRoundEndReason EndReason) const;

	void ClearCenterMessageTimers();
	void QueueCenterMessage(float DelaySeconds, const FText& Message);
	void ShowCenterMessageForDuration(const FText& Message, float DurationSeconds);

	TArray<FTimerHandle> CenterMessageTimerHandles;
	FTimerHandle CenterMessageHoldTimerHandle;
	FTimerHandle OutroImageTimerHandle;

	// NativeDestruct 이후 GC 전 구간에서 타이머가 발화되지 않도록 막는 플래그
	bool bIsDestructed = false;
};
