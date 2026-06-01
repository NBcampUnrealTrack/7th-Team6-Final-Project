// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBFSMiniGame.generated.h"


class AFishActor;
class UPTBFSMiniGameRuleSet;
class APTBRhythmCharacterBase;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingPromptShown, EPTBActionType, RequiredAction);
 
/**
 * 성공 입력 시 발행
 * PullStrength로 물튀김 이펙트 강도, 컨트롤러 진동 강도 조절
 * HighPerfect=1.0 / Perfect=0.7 / Good=0.4
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishPulled, float, PullStrength);
 
/** Miss 시 발행 — SlipStrength로 줄 흔들림 연출 강도 조절 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishSlipped, float, SlipStrength);
 
/** AllNotesPassed 시 발행 — 물고기 점프 연출, 종류 공개 UI 연동 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishRevealed, AFishActor*, CaughtFish);
 


UCLASS()
class PARTTIMEBEAT_API APTBFSMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
 
public:
	APTBFSMiniGame();
 
	virtual void BeginPlay() override;
	
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
	void HandleActionDInput(); 
	
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionEInput();

protected:
	virtual void BuildRuntimeState()override;
	
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;
	
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;
	
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	
	virtual void PlayJudgementFeedback(const FPTBJudgementResult& Result) override;
	
	virtual FPTBMiniGameResultPayload BuildResultPayload() const override;
	
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void InitializeMiniGame(const FPTBMiniGameContext& Context)override;
private:
	UFUNCTION()
	void OnAllNotesPassedFishing();
	
	float FishDistance =0.0f;
	
	EFishingLineState CurrentLineState = EFishingLineState::None;
	
	float StepDistance =0;
	
	int32 CorrectCount=0;
	
	int32 MovingCount = 0;
	
	FVector FishStartLocation = FVector::ZeroVector;
	
	FVector CharacterLocation = FVector::ZeroVector;
	
	FTimerHandle FishTimer;
	
	UPROPERTY()
	TObjectPtr<AFishActor> FishActor=nullptr;
	
	UPROPERTY()
	TObjectPtr<UPTBFSMiniGameRuleSet> FishRuleSet =nullptr
	;
	
	UPROPERTY()
	TObjectPtr<APTBRhythmCharacterBase> Character = nullptr;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Fishing",meta = (AllowPrivateAccess = "true"))
	float AutoDriftMultiplier = 0.3f;
	
	/**물고기 거리조정함수*/
	void ApplyDistanceDelta(float Delta);
	
	EFishingLineState CalculateLineState(float Distance)const;
};
