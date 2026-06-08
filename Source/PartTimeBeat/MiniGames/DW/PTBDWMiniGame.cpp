#include "MiniGames/DW/PTBDWMiniGame.h"

#include "Debug/PTBTeamLog.h"
#include "MiniGames/DW/PTBDWMiniGameRuleSet.h"
#include "Rhythm/PTBRhythmChartAsset.h"

void APTBDWMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	// TODO(9단계): 마커/장애물/Niagara 풀 미리 채우기, NoteId 추적 Map 초기화 등
	//             (이 함수는 InitializeMiniGame 내부에서 호출 — 아직 음악 시작 전)

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW BuildRuntimeState RuleSet=%s Chart=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()));
}

void APTBDWMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	// TODO(9단계): 개 짧은 예고 연출 + 마커/장애물 풀에서 스폰 후
	//             SpawnVisualMs/TargetMs/InvDuration 기록, ActiveNoteViews[NoteId] 등록

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW NoteCue NoteId=%d Action=%d NoteType=%d Long=%d Beat=%.3f TimeMs=%.3f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		static_cast<int32>(Note.NoteType),
		Note.bIsLongNote ? 1 : 0,
		Note.BeatTime,
		Note.TimeMs);
}

void APTBDWMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	// 공통 점수/콤보/SFX/실패 처리 — 절대 빼지 말 것
	Super::HandleJudgementResult(Result);

	// 공입력은 마커(NoteId)가 없으므로 Reason을 먼저 분기한다.
	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		// TODO(9단계): 공입력 연출. 기획서상 무연출이면 이 분기는 비워둔다.
		PTB_VERBOSE(LogPTBMiniGames,
			TEXT("[%s] DW EmptyInput Action=%d"),
			*GetNameSafe(this),
			static_cast<int32>(Result.ActionType));
		return;
	}

	// TODO(9단계): Result.NoteId로 ActiveNoteViews에서 마커 회수 +
	//             성공/실패(EarlyRelease 등) 주인공 연출

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW Judgement NoteId=%d Action=%d Type=%d Reason=%d DeltaMs=%.3f ScoreDelta=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		Result.DeltaMs,
		Result.ScoreDelta);
}

const UPTBDWMiniGameRuleSet* APTBDWMiniGame::GetDWRuleSet() const
{
	return Cast<UPTBDWMiniGameRuleSet>(RuleSet.Get());
}