#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "Styling/SlateBrush.h"
#include "PTBLatencyCalibrationWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UButton;
class UTextBlock;
class USoundBase;

/**
 * 적용 버튼을 눌렀을 때만 호출된다(결과가 나왔다고 바로 저장하지 않음).
 * AverageOffsetMs = 실제로 저장된 판정 오프셋 값(설정창 슬라이더에 넣을 값).
 * (이름은 기존 BP 바인딩 호환을 위해 그대로 둠)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalibrationFinished, float, AverageOffsetMs);

UENUM(BlueprintType)
enum class EPTBCalibrationState : uint8
{
	Idle,     // 시작 전 / 닫힘
	Running,  // 측정 중
	Result    // 결과 확인 중 (적용 / 다시하기 / 취소 선택)
};

USTRUCT()
struct FPTBCalibrationNote
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UBorder> Widget = nullptr;

	/** 이 노트가 판정선에 닿는 시각(FPlatformTime 기준 초) */
	double HitTime = 0.0;
};

/**
 * 판정 싱크 조절 미니게임.
 *
 * - 시간은 실제 시간(FPlatformTime) 기준이라 일시정지 중에도 동작한다.
 * - 측정 키 = 설정의 ActionA 키 (bUseActionAKey). 버튼에 포커스가 있어도 키를 먼저 가로채므로
 *   스페이스/엔터가 "다시하기" 버튼을 눌러버리는 문제가 없다.
 * - 결과가 나오면 바로 저장하지 않고 Result 상태로 대기 → 적용(Apply) 시에만 저장.
 * - 노트 이동 거리/크기는 UI 단위(DPI 스케일 적용)라 해상도가 달라도 화면 대비 같은 속도·크기로 보인다.
 *
 * Designer 위젯 (이름·타입 일치 시 자동 연결, 모두 선택 사항 except NoteTrackCanvas):
 *  NoteTrackCanvas (Canvas Panel, 필수), JudgmentLineWidget (아무 위젯),
 *  ButtonRestart, ButtonApply, ButtonCancel (Button),
 *  TXT_TargetKey (Text: 측정 키 이름 표시), KeyPulseTarget (아무 위젯: 박자마다 커졌다 작아짐. 없으면 TXT_TargetKey)
 */
UCLASS()
class PARTTIMEBEAT_API UPTBLatencyCalibrationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ── 리듬 ─────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float BPM = 100.f;

	/** true면 설정의 ActionA 키로 측정 (기본). false면 TargetKey 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	bool bUseActionAKey = true;

	/** bUseActionAKey가 false이거나 ActionA 키를 못 찾았을 때 쓰는 키 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	FKey TargetKey = EKeys::SpaceBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	int32 RequiredPressCount = 8;

	/** 노트가 생성돼서 판정선까지 오는 데 걸리는 박자 수 (클수록 천천히 옴) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (ClampMin = "1"))
	int32 NoteLeadBeats = 3;

	/** 결과 오프셋 허용 범위 (설정창 슬라이더 범위와 같게) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float MaxOffsetMs = 200.f;

	/** 적용 버튼을 누르면 창도 같이 닫기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	bool bCloseOnApply = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	TObjectPtr<USoundBase> MetronomeSound;

	// ── 노트 모양 ────────────────────────────────────────────────────

	/** 판정선 X 위치 (0=왼쪽 끝, 1=오른쪽 끝). bAutoPlaceJudgmentLine이 켜져 있으면 JudgmentLineWidget을 여기로 옮김 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	float JudgmentLineAnchorX = 0.25f;

	/**
	 * true: JudgmentLineWidget을 JudgmentLineAnchorX 위치로 자동 배치.
	 * false: Designer에 놓은 위치 그대로 사용.
	 * 어느 쪽이든 노트는 JudgmentLineWidget의 "실제 화면 중앙"을 기준으로 박자에 맞춰 도착함.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	bool bAutoPlaceJudgmentLine = true;

	/**
	 * true: 판정선을 노트와 같은 크기의 네모(NoteSizePx)로 만들어 트랙 세로 중앙에 둠 → 노트가 그대로 겹치면 정박.
	 * false: 세로로 긴 선 (Designer의 가로 두께 유지).
	 * bAutoPlaceJudgmentLine이 켜져 있을 때만 적용. 판정선은 NoteTrackCanvas 안에 있어야 세로 위치가 맞음.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	bool bJudgmentLineMatchesNoteSize = true;

	/**
	 * 노트의 어느 부분이 판정선에 닿는 순간을 박자로 볼지 (0 = 노트 왼쪽 끝, 0.5 = 노트 중앙).
	 * 노트가 크면 "닿는 순간"과 "중앙이 겹치는 순간"의 차이가 커서 박자가 어긋나 보일 수 있음.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note", meta = (ClampMin = "0", ClampMax = "1"))
	float NoteHitPointX = 0.5f;

	/** 노트가 생성되는 지점 ~ 판정선 거리 (UI 단위). 이동 속도 = 거리 / (NoteLeadBeats 박자 시간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note", meta = (ClampMin = "100"))
	float NoteTravelDistance = 700.f;

	/** 노트 크기 (UI 단위. 해상도에 따라 화면 비율대로 같이 커짐) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	float NoteSizePx = 72.f;

	/** 노트 그림. 비워두면 NoteColor 단색 사각형 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	FSlateBrush NoteBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note")
	FLinearColor NoteColor = FLinearColor(0.1f, 0.8f, 1.0f, 1.0f);

	/** 생성 직후 이 비율 구간 동안 서서히 나타남 (0이면 바로 보임) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Note", meta = (ClampMin = "0", ClampMax = "0.5"))
	float NoteFadeInRatio = 0.15f;

	// ── 키 표시 ──────────────────────────────────────────────────────

	/** 박자마다 키 표시가 커지는 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key")
	float KeyPulseScale = 1.25f;

	/** 커졌다가 원래 크기로 돌아오는 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key")
	float KeyPulseDuration = 0.18f;

	/** 박자마다 움직이는 노트들도 같이 커졌다 작아짐 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key")
	bool bPulseNotes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key", meta = (EditCondition = "bPulseNotes"))
	float NotePulseScale = 1.15f;

	/** 박자마다 판정선도 같이 커졌다 작아짐 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key")
	bool bPulseJudgmentLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration|Key", meta = (EditCondition = "bPulseJudgmentLine"))
	float JudgmentLinePulseScale = 1.15f;

	// ── 이벤트 / 함수 ────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Calibration")
	FOnCalibrationFinished OnCalibrationFinished;

	/** 측정 시작 (처음부터 다시) */
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void StartCalibration();

	/** 결과를 저장하고 OnCalibrationFinished 호출. Result 상태에서만 동작 */
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void ApplyCalibrationResult();

	/** 저장하지 않고 닫기 (취소/닫기 공용) */
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void CloseCalibration();

	UFUNCTION(BlueprintPure, Category = "Calibration")
	float GetSavedJudgementOffsetMs() const;

	/** 마지막 측정으로 계산된 보정값 (적용하면 저장될 값) */
	UFUNCTION(BlueprintPure, Category = "Calibration")
	float GetResultOffsetMs() const { return ResultOffsetMs; }

	UFUNCTION(BlueprintPure, Category = "Calibration")
	EPTBCalibrationState GetCalibrationState() const { return State; }

	/** 측정에 쓰는 키의 표시 이름 (예: Z) */
	UFUNCTION(BlueprintPure, Category = "Calibration")
	FText GetTargetKeyDisplayName() const;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteTrackCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> JudgmentLineWidget;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UButton> ButtonRestart;

	/** 결과가 나왔을 때만 보임 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UButton> ButtonApply;

	/** 저장하지 않고 닫기 (취소 = 닫기) */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UButton> ButtonCancel;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UTextBlock> TXT_TargetKey;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UWidget> KeyPulseTarget;

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnMetronomeTick();

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnJudgementFlash(bool bGoodHit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnOffsetMsUpdated(const FText& FormattedOffset);

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnPressCountUpdated(const FText& FormattedCount);

	/** 측정 완료 시 (아직 저장 안 됨). FormattedAverage = 적용 시 저장될 보정값 (예: "-32 ms") */
	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnCalibrationFinishedDisplay(const FText& FormattedAverage);

	/** 상태가 바뀔 때마다 (Idle/Running/Result). 버튼 표시는 C++이 처리하므로 추가 연출용 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnCalibrationStateChanged(EPTBCalibrationState NewState);

private:
	void SetState(EPTBCalibrationState NewState);
	void SpawnNote(double HitTime);
	void ClearNotes();
	void FinishCalibration();
	void RegisterPress(double PressTime);
	void RefreshKeyText();
	/** 판정선 중앙의 X (NoteTrackCanvas 기준 로컬 좌표) */
	float ResolveJudgmentLineX() const;
	/** 0~1 (1 = 박자 순간) 값으로 키 표시·판정선·노트 크기 갱신 */
	void ApplyBeatPulse(float Pulse);
	FKey ResolveTargetKey() const;
	double ConsumePressTime() const;

	UFUNCTION() void HandleRestartClicked();
	UFUNCTION() void HandleApplyClicked();
	UFUNCTION() void HandleCancelClicked();

	EPTBCalibrationState State = EPTBCalibrationState::Idle;

	double BeatIntervalSec = 0.0;
	double NoteTravelTimeSec = 0.0;
	/** 0번 박자(첫 소리) 시각 */
	double FirstBeatTime = 0.0;
	/** 다음에 소리 낼 박자 번호 / 다음에 생성할 노트의 박자 번호 */
	int32 NextSoundBeat = 0;
	int32 NextNoteBeat = 0;

	double LastPulseTime = -1.0;

	int32 CurrentPressCount = 0;
	TArray<float> BeatOffsetsMs;
	TArray<FPTBCalibrationNote> ActiveNotes;

	float ResultOffsetMs = 0.f;
	FKey ActiveKey;
};
