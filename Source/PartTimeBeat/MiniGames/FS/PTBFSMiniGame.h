// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBFSMiniGame.generated.h"


class AFishActor;
class UPTBFSMiniGameRuleSet;
class APTBFSCharacter;
class UFSWidget;

UENUM(BlueprintType)
enum class EFishingLineState : uint8
{
	None,
	Loose,
	Taut,
	Maximum,
};

/** 물고기 거리 변화 시 발행 (0.0 = 수면, 1.0 = 최대 수심) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishDistanceChanged, float, NewDistance);
 
/** 줄 긴장 상태 변화 시 발행 — BP에서 줄 연출, 사운드 강도 연동 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingLineStateChanged, EFishingLineState, NewState);
 
/** Cue 시점 발행 — BP에서 어떤 키를 눌러야 하는지 HUD 표시 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishingPromptShown, EPTBActionType, RequiredAction, bool, bIsLongNote);
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingJudgement, EPTBJudgementType, JudgementType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingComboChanged, int32, NewCombo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFishingPromptCue, EPTBActionType, RequiredAction, bool, bIsLongNote, float, TargetTimeMs,float, CueLeadTimeMs);

/*
 * 성공 입력 시 발행
 * PullStrength로 물튀김 이펙트 강도, 컨트롤러 진동 강도 조절
 * HighPerfect=1.0 / Perfect=1.0 / Good=1.0
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishPulled, float, PullStrength);
 
/** Miss 시 발행 — SlipStrength로 줄 흔들림 연출 강도 조절 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishSlipped, float, SlipStrength);
 
/** AllNotesPassed 시 발행 — 물고기 점프 연출, 종류 공개 UI 연동 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishRevealed, AFishActor*, CaughtFish);
 
/** 롱노드판정을위한 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingPromptReleased, EPTBActionType, ReleasedAction);

UCLASS()
class PARTTIMEBEAT_API APTBFSMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
 
public:
	APTBFSMiniGame();
 
	virtual void BeginPlay() override;
	
	/**롱노드 판정을위한 델리게이트*/
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingPromptReleased OnFishingPromptReleased;
	
	/** 물고기 거리 변화 시 발행 — 물고기 수심, 낚싯줄 길이 연출 연동 */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishDistanceChanged OnFishDistanceChanged;
 
	/** 줄 긴장 상태 변화 시 발행 — 줄 색깔, 팽팽함 애니메이션, 사운드 연동 */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingLineStateChanged OnFishingLineStateChanged;
 
	/** Cue 시점 발행 — 물고기 발버둥 모션 + 요구 키 HUD */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingPromptShown OnFishingPromptShown;
 
	/** 성공 입력 시 발행 — 물튀김 이펙트, 컨트롤러 진동 강도 연동 */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishPulled OnFishPulled;
 
	/** Miss 시 발행 — 줄 흔들림 연출, 실패 사운드 연동 */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishSlipped OnFishSlipped;
 
	/** AllNotesPassed 시 발행 — 물고기 점프 연출, 종류 공개 UI 연동 */
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishRevealed OnFishRevealed;
	
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingJudgement OnFishingJudgement;
	
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingComboChanged OnFishingComboChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingPromptCue OnFishingPromptCue;
	
	UFUNCTION(BlueprintPure, Category = "Fishing")
	float GetFishDistance() const { return FishDistance; }

	UFUNCTION(BlueprintPure, Category = "Fishing")
	EFishingLineState GetLineState() const { return CurrentLineState; }
	
	UFUNCTION(BlueprintPure, Category = "Fishing")
	int32 GetCorrectCount() const { return CorrectCount; }
	
	UFUNCTION(BlueprintPure, Category = "Fishing")
	int32 GetMovingCount() const { return MovingCount; }
 
	
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	virtual void HandleActionAInput(); 
 
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionBInput(); 
 
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionCInput(); 
	
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionAInputReleased();

	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionBInputReleased();

	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionCInputReleased();
	
	/**
	 * 키 안내 UI용: KeyIndex(0=A,1=B,2=C) 키가 VisualTimeMs 시점에 어느 칸(0=왼,1=가운데,2=오른쪽)에 있는지.
	 * 교체 연출 중이면 FromLane→ToLane, Alpha 0~1. 연출 중이 아니면 From=To, Alpha=1
	 */
	UFUNCTION(BlueprintPure, Category = "Fishing|KeyShuffle")
	void GetKeyGuideMotion(int32 KeyIndex, float VisualTimeMs, int32& OutFromLane, int32& OutToLane, float& OutAlpha) const;

	/** 키 셔플 기믹이 이번 판에 켜져 있는지 */
	UFUNCTION(BlueprintPure, Category = "Fishing|KeyShuffle")
	bool IsKeyShuffleActive() const { return bKeyShuffleActive; }

	/** 노트를 화면 어느 칸에 보여줄지 (ActionA=왼쪽, B=가운데, C=오른쪽). 판정 키는 노트의 ActionType 그대로 */
	EPTBActionType GetVisualAction(const FPTBNoteEvent& Note) const;

	/** NoteId로 화면 칸 찾기 (판정 결과용) */
	EPTBActionType GetVisualActionForNoteId(int32 NoteId, EPTBActionType Fallback) const;

	/** 오입력(다른 키) 즉시 흔들림 감지 후 공통 입력 처리 */
	virtual void HandleRhythmInput(EPTBActionType Action, float TimeMs = -1.0f) override;

	TObjectPtr<UPTBWwiseRhythmSyncComponent> GetUPTBWwiseRhythmSyncComponent()
	{
		return RhythmSyncComponent;
	};
protected:
	virtual void Tick(float DeltaTime) override;
	
	virtual void BuildRuntimeState()override;
	
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;
	
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;
	
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	
	virtual FPTBMiniGameResultPayload BuildResultPayload() const override;
	
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void InitializeMiniGame(const FPTBMiniGameContext& Context)override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 튜토리얼 연습 ↔ 본게임 채보 전환 시 키 셔플 일정 재계산 */
	virtual void HandleRoundChartChanged() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFSWidget> WidgetClass;
	
	UPROPERTY()
	UFSWidget* ActiveFSWidget;
	
private:
	UFUNCTION()
	void OnAllNotesPassedFishing();
	
	float FishDistance =1.0f;
	
	EFishingLineState CurrentLineState = EFishingLineState::None;
	
	float StepDistance =0;
	
	int32 CorrectCount=0;
	
	int32 MovingCount = 0;
	
	FVector FishStartLocation = FVector::ZeroVector;
	
	FVector CharacterLocation = FVector::ZeroVector;
	
	UPROPERTY()
	TObjectPtr<AFishActor> FishActor=nullptr;
	
	UPROPERTY()
	TObjectPtr<UPTBFSMiniGameRuleSet> FishRuleSet =nullptr;
	
	UPROPERTY()
	TObjectPtr<APTBFSCharacter> Character = nullptr;
	
	void ApplyDistanceDelta(float Delta);
	
	EFishingLineState CalculateLineState(float Distance)const;
	
	FVector FishLateralOffset = FVector::ZeroVector;
	
	// ── 키 셔플 ──
	struct FFSKeySegment
	{
		/** KeyForLane[칸] = 그 칸을 맞추는 키 (0=A,1=B,2=C) */
		int32 KeyForLane[3] = { 0, 1, 2 };
		/** 이전 구간 배치 → 이 구간 배치로 바뀌는 연출 구간 (차트 시간 ms) */
		float SwapStartMs = TNumericLimits<float>::Lowest();
		float SwapEndMs = TNumericLimits<float>::Lowest();
		bool bHasSwap = false;
	};

	/** 이번 채보 기준으로 구간별 키 배치/교체 연출 시간 계산 (항상 같은 결과) */
	void RebuildKeyShuffleSchedule();

	/** 노트 박자 → 구간 인덱스 */
	int32 GetKeySegmentIndex(float BeatTime) const;

	/** 해당 구간에서 키가 있는 칸 */
	int32 GetLaneOfKey(int32 SegmentIndex, int32 KeyIndex) const;

	TArray<FFSKeySegment> KeySegments;
	float KeySegmentBeats = 4.0f;
	bool bKeyShuffleActive = false;
	
	int32 CurrentComboCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> MissCameraShakeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SuccessAnim; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> FailAnim;; 
	
	UPROPERTY(EditAnywhere, Category = "Camera",meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULevelSequence> SuccessCameraSequence;
	
	UPROPERTY(EditAnywhere, Category = "Camera",meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULevelSequence> FailCameraSequence;

	// ── 미스 화면 흔들림 ──
	// 기존: 노트가 판정 범위(Miss 500ms)를 완전히 지나 만료될 때만 흔들려서 체감상 늦음.
	// 변경: ① 다른 키를 누르면 즉시 ② 안 누른 노트는 Good 범위가 끝나는 순간(더 이상 성공 불가) 흔들림.
	//       같은 노트로는 한 번만 흔들림 (이후 만료 Miss에서 중복 안 함).

	/** 노트 근처에서 다른 키를 누르면 즉시 흔들림 (노트 자체는 그대로 남아 맞는 키로 다시 칠 수 있음) */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	bool bShakeOnWrongKey = true;

	/** 안 누른 노트를 만료(Miss 범위 끝)까지 기다리지 않고 Good 범위가 끝나는 즉시 흔들림 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	bool bEarlyMissShake = true;

	/** 흔들림 최소 간격(초). 연타 오입력 시 과도하게 흔들리지 않도록 */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MissShakeCooldownSec = 0.2f;

	/** NoteId 기준 한 번만 흔들기. NoteId=0이면 쿨다운만 적용 */
	void TriggerMissShake(int32 NoteId);

	/** 판정 대기 중인 노트 시간 (조기 흔들림 검사용). 판정되면 제거 */
	TMap<int32, float> ArmedNoteTimesMs;
	TSet<int32> ShakenNoteIds;
	float LastMissShakeWorldTime = -1000.0f;
};
