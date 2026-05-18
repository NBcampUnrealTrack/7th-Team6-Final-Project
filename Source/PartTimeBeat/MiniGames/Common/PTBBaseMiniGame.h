#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBBaseMiniGame.generated.h"

class UPTBMiniGameRuleSet;
class UPTBWwiseEventMapAsset;
class UPTBRhythmConductorComponent;
class UPTBJudgementSystem;
class UPTBScoreCalculator;

UCLASS()
class PARTTIMEBEAT_API APTBBaseMiniGame : public AActor
{
	GENERATED_BODY()
	
public:	
	APTBBaseMiniGame();

protected:
	virtual void BeginPlay() override;

public:	
	/**고유 ID */
	FName MiniGameId;
	/** 2글자 코드(JJ 등) */
	FName MiniGameCode;
	/** UI 표시 이름 */
	FText DisplayName;
	/** 난이도별 규칙 DataAsset */
	UPTBMiniGameRuleSet* RuleSet;
	/** Wwise 이벤트 매핑 */
	UPTBWwiseEventMapAsset* AudioEventSet;
	/** Beat 발행 컴포넌트 */
	UPTBRhythmConductorComponent* RhythmConductor;
	/** 판정 계산 */
	UPTBJudgementSystem* JudgementSystem;
	/** 점수 계산 */
	UPTBScoreCalculator* ScoreCalculator;
	/** 라운드 컨텍스트 */
	FPTBMiniGameContext GameContext;
	/** 처리 중인 노트 */
	TArray<FPTBNoteEvent> ActiveNoteQueue;
	/** 종료 결과 */
	FPTBRoundResult RoundResult;
	/** 초기화 완료 */
	bool bIsInitialized;
	/** 라운드 진행 중 */
	bool bIsRoundActive;
	/**입력 잠금*/
	bool bInputLocked;

	virtual void Tick(float DeltaTime) override;
	/** 컨텍스트 주입 */
	void InitializeMiniGame(const FPTBMiniGameContext& Context);
	/** 라운드 시작 전 에셋 준비(하위 override 권장) */
	void PreloadAssets();
	/** 오브젝트 / 상태 구성(하위 override) */
	void BuildRuntimeState();
	/** BGM + Conductor 시작, 입력 허용 */
	void StartMiniGame();
	/** 종료, 결과 생성 */
	FPTBRoundResult FinishMiniGame(EPTBRoundEndReason Reason);
	/** 일시정지  */
	void PauseMiniGame();
	/** 재개 */
	void ResumeMiniGame();

	/** 채보 이벤트 처리(하위 override) */
	void HandleChartEvent(const FPTBNoteEvent& Note);
	/** 선행 비주얼 큐(하위 override) */
	void HandleNoteCue(const FPTBNoteEvent& Note);
	/** 입력 → 판정 */
	void HandleRhythmInput(EPTBActionType Action, float TimeMs);
	/** 단일 판정 */
	FPTBJudgementResult EvaluateInput(EPTBActionType Action, float TimeMs);
	/** 점수 / HUD / SFX 반영 */
	void HandleJudgementResult(const FPTBJudgementResult& Result);
	/** 미니게임 전용 피드백(하위 override) */
	void PlayJudgementFeedback(const FPTBJudgementResult& Result);

	/** 전용 결과(하위 override) */
	FPTBMiniGameResultPayload BuildResultPayload() const;
	/** AudioManager에 이벤트 요청 */
	void RequestWwiseEvent(FName EventKey, AActor* Target);
	/** 입력 가능 여부 */
	bool CanAcceptInput() const;
	/** 키 - 액션 매핑(하위 override) */
	TMap<FKey, EPTBActionType> GetActionMapping() const;

};
