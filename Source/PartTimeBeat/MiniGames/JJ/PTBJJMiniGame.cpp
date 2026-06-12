#include "MiniGames/JJ/PTBJJMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "MiniGames/JJ/PTBJJMiniGameRuleSet.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Debug/PTBTeamLog.h"
#include "MiniGames/JJ/PTBJJJumpActor.h"
#include "TimerManager.h"
#include "Engine/World.h"

void APTBJJMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	JumpCount = 0;
	LandingCount = 0;
	CueCount = 0;
	CuedNotes.Reset();
	ArmedNotes.Reset();
	ReachedNotes.Reset();

	if (JumpSpeeds.Num() < 3)
	{
		JumpSpeeds.Init(1.0f, 3);
	}

	// 채보 스캔 → 노트별 체공시간 산출
	PrecomputeAirtimes();   

	UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] JJ BuildRuntimeState RuleSet=%s Chart=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()));
}

void APTBJJMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	++CueCount;
	TrackNote(CuedNotes, Note);

	const int32 CharacterIndex = ResolveCharacterIndex(Note.ActionType);
	OnJJJumpCue.Broadcast(CharacterIndex, Note);

	const float AirtimeMs = GetAirtimeMsForNote(Note.NoteId);
	const float SpeedScale = JumpSpeeds.IsValidIndex(CharacterIndex) ? JumpSpeeds[CharacterIndex] : 1.0f;

	// 지금 시각과 착지 목표(정시점) 사이 남은 시간
	const float NowMs = GetCurrentChartTimeMs();
	const float TargetMs = Note.TimeMs;
	const float TimeToReach = TargetMs - NowMs;   // Cue 시점이므로 양수여야 정상

	// 점프를 발동해야 하는 시점 = 착지목표 - 체공시간
	// → 지금부터 (TimeToReach - AirtimeMs) 후에 StartJump
	const float DelayMs = TimeToReach - AirtimeMs;

	if (UWorld* World = GetWorld())
	{
		if (DelayMs <= 0.0f)
		{
			// 이미 점프해야 할 시각을 지났거나 딱 맞음 → 즉시 발동
			// (Cue 선행시간 < 체공시간인 설정에서 발생. 가능하면 CueLeadTimeMs를 늘릴 것)
			StartScheduledJump(CharacterIndex, AirtimeMs, SpeedScale);
		}
		else
		{
			FTimerHandle Handle;
			FTimerDelegate Del = FTimerDelegate::CreateUObject(
				this, &APTBJJMiniGame::StartScheduledJump, CharacterIndex, AirtimeMs, SpeedScale);
			World->GetTimerManager().SetTimer(Handle, Del, DelayMs / 1000.0f, false);
			PendingJumpTimers.Add(Handle);
		}
	}

	LogNoteDebug(TEXT("JumpCue"), Note);
}

void APTBJJMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);

	TrackNote(ArmedNotes, Note);

	LogNoteDebug(TEXT("NoteArm"), Note);
}

void APTBJJMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);

	// 정시점 도달 = 이 시점에 캐릭터가 바닥에 착지(점프는 Cue 때 이미 시작됨)
	++JumpCount;
	TrackNote(ReachedNotes, Note);

	const int32 CharacterIndex = ResolveCharacterIndex(Note.ActionType);
	OnJJJumpTriggered.Broadcast(CharacterIndex, Note);

	LogNoteDebug(TEXT("JumpTriggered"), Note);
}

void APTBJJMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	FPTBNoteEvent JudgedNote;
	const bool bHasJudgedNote = FindTrackedNote(Result.NoteId, JudgedNote);

	// 주의: Super가 내부에서 PlayJudgementFeedback() 호출 → 착지 연출은 거기서 처리
	Super::HandleJudgementResult(Result);

	++LandingCount;

	if (bHasJudgedNote)
	{
		const int32 CharacterIndex = ResolveCharacterIndex(JudgedNote.ActionType);

		RemoveTrackedNote(CuedNotes, Result.NoteId);
		RemoveTrackedNote(ArmedNotes, Result.NoteId);
		RemoveTrackedNote(ReachedNotes, Result.NoteId);

		OnJJLanding.Broadcast(CharacterIndex, Result, JudgedNote);
		OnJJNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);
	}
	else if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		const int32 CharacterIndex = ResolveCharacterIndex(Result.ActionType);
		FPTBNoteEvent EmptyInputNote;
		OnJJLanding.Broadcast(CharacterIndex, Result, EmptyInputNote);
	}
}

void APTBJJMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	Super::InitializeMiniGame(Context);
}

void APTBJJMiniGame::BeginPlay()
{
	Super::BeginPlay();
}

//void APTBJJMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
//{
//	Super::PlayJudgementFeedback(Result);
//
//	// 착지 연출은 여기서만 호출 (Super::HandleJudgementResult 경로로 1회 보장)
//	FPTBNoteEvent JudgedNote;
//	const int32 CharacterIndex = FindTrackedNote(Result.NoteId, JudgedNote)
//		? ResolveCharacterIndex(JudgedNote.ActionType)
//		: ResolveCharacterIndex(Result.ActionType);
//
//	PlayLandingFeedback(CharacterIndex, Result);
//}

FPTBMiniGameResultPayload APTBJJMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();
	Payload.PayloadType = TEXT("JJ");

	const UPTBJJMiniGameRuleSet* JJRuleSet = GetJJRuleSet();
	if (JJRuleSet && !JJRuleSet->bIncludeDebugPayload)
	{
		return Payload;
	}

	Payload.IntValues.Add(TEXT("JumpCount"), JumpCount);
	Payload.IntValues.Add(TEXT("LandingCount"), LandingCount);
	Payload.IntValues.Add(TEXT("CueCount"), CueCount);
	Payload.IntValues.Add(TEXT("CuedNotesRemaining"), CuedNotes.Num());
	Payload.IntValues.Add(TEXT("ArmedNotesRemaining"), ArmedNotes.Num());
	Payload.IntValues.Add(TEXT("ReachedNotesRemaining"), ReachedNotes.Num());

	if (ScoreCalculator)
	{
		Payload.IntValues.Add(TEXT("MissCount"), ScoreCalculator->MissCount);
	}

	return Payload;
}

void APTBJJMiniGame::TriggerCharacterJump(int32 CharacterIndex, float JumpPowerScale)
{
	// TODO: 좌/중/우 캐릭터 Actor 점프 연출 연결 (Play_SFX_JJ_Jump)
}

void APTBJJMiniGame::PlayLandingFeedback(int32 CharacterIndex, const FPTBJudgementResult& Result)
{
	// TODO: 판정 타입별 착지 연출 분기 (성공: Play_SFX_JJ_Land / 실패: Play_SFX_JJ_Fail)
}

void APTBJJMiniGame::ApplyVariableJumpSpeed(float SpeedScale)
{
	// TODO: 진행 중 전체 점프 속도 배율 변경 (변속 구간 연출)
}

void APTBJJMiniGame::HandleJJInput(EPTBActionType Action, float TimeMs)
{
	if (Action == EPTBActionType::None)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid JJ action input."), *GetNameSafe(this));
		return;
	}

	HandleRhythmInput(Action, TimeMs);
}

void APTBJJMiniGame::HandleJJInputReleased(EPTBActionType Action, float TimeMs)
{
	if (Action == EPTBActionType::None)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid JJ action release input."), *GetNameSafe(this));
		return;
	}

	HandleRhythmInputReleased(Action, TimeMs);
}

const UPTBJJMiniGameRuleSet* APTBJJMiniGame::GetJJRuleSet() const
{
	return Cast<UPTBJJMiniGameRuleSet>(RuleSet.Get());
}

int32 APTBJJMiniGame::ResolveCharacterIndex(EPTBActionType Action) const
{
	if (const UPTBJJMiniGameRuleSet* JJRuleSet = GetJJRuleSet())
	{
		return JJRuleSet->ResolveCharacterIndex(Action);
	}

	// RuleSet 없을 때 폴백
	switch (Action)
	{
	case EPTBActionType::ActionA: return 0; // 좌
	case EPTBActionType::ActionB: return 1; // 중
	case EPTBActionType::ActionC: return 2; // 우
	default:                      return 0;
	}
}

void APTBJJMiniGame::TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note)
{
	for (const FPTBNoteEvent& TrackedNote : Notes)
	{
		if (TrackedNote.NoteId == Note.NoteId)
		{
			return;
		}
	}

	Notes.Add(Note);
}

bool APTBJJMiniGame::RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId)
{
	for (int32 Index = 0; Index < Notes.Num(); ++Index)
	{
		if (Notes[Index].NoteId == NoteId)
		{
			Notes.RemoveAt(Index);
			return true;
		}
	}

	return false;
}

bool APTBJJMiniGame::FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const
{
	for (const FPTBNoteEvent& Note : ArmedNotes)
	{
		if (Note.NoteId == NoteId) { OutNote = Note; return true; }
	}
	for (const FPTBNoteEvent& Note : CuedNotes)
	{
		if (Note.NoteId == NoteId) { OutNote = Note; return true; }
	}
	for (const FPTBNoteEvent& Note : ReachedNotes)
	{
		if (Note.NoteId == NoteId) { OutNote = Note; return true; }
	}

	return false;
}

void APTBJJMiniGame::LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const
{
	const UPTBJJMiniGameRuleSet* JJRuleSet = GetJJRuleSet();
	if (JJRuleSet && !JJRuleSet->bLogNoteEvent)
	{
		return;
	}

	UE_LOG(LogPTBMiniGames, Log,
		TEXT("[%s] JJ %s NoteId=%d Action=%d NoteType=%d Lane=%d Beat=%.3f TimeMs=%.3f Long=%d"),
		*GetNameSafe(this),
		EventName,
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		static_cast<int32>(Note.NoteType),
		Note.Lane,
		Note.BeatTime,
		Note.TimeMs,
		Note.bIsLongNote ? 1 : 0);
}

void APTBJJMiniGame::PrecomputeAirtimes()
{
	NoteAirtimeMs.Reset();

	if (!ChartAsset)
	{
		return;
	}

	const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;

	// 액션별로 "바로 다음에 같은 액션이 등장하는 노트의 TimeMs"를 알아내기 위해
	// 뒤에서 앞으로 스캔하며 각 액션의 직후 등장 시각을 추적한다.
	TMap<EPTBActionType, float> NextSameActionTimeMs;

	for (int32 Index = Notes.Num() - 1; Index >= 0; --Index)
	{
		const FPTBNoteEvent& Note = Notes[Index];

		// 이 노트 기준, 같은 액션의 "다음" 노트 시각
		float Airtime = MaxAirtimeMs;
		if (const float* NextTime = NextSameActionTimeMs.Find(Note.ActionType))
		{
			const float Gap = *NextTime - Note.TimeMs;   // 같은 액션끼리의 간격
			if (Gap > 0.0f)
			{
				Airtime = FMath::Min(MaxAirtimeMs, Gap);
			}
		}

		// 하한 클램프(너무 짧은 점프 방지)
		Airtime = FMath::Max(Airtime, MinAirtimeMs);

		NoteAirtimeMs.Add(Note.NoteId, Airtime);

		// 다음(앞쪽) 노트 스캔을 위해 이 액션의 등장 시각 갱신
		NextSameActionTimeMs.Add(Note.ActionType, Note.TimeMs);
	}
}

float APTBJJMiniGame::GetAirtimeMsForNote(int32 NoteId) const
{
	if (const float* Found = NoteAirtimeMs.Find(NoteId))
	{
		return *Found;
	}
	return MaxAirtimeMs;
}

void APTBJJMiniGame::StartScheduledJump(int32 CharacterIndex, float AirtimeMs, float HeightScale)
{
	if (JumpActors.IsValidIndex(CharacterIndex) && JumpActors[CharacterIndex])
	{
		JumpActors[CharacterIndex]->StartJump(AirtimeMs, HeightScale);
	}

	// 기존 TriggerCharacterJump 훅도 유지하고 싶으면 함께 호출
	TriggerCharacterJump(CharacterIndex, HeightScale);
}

void APTBJJMiniGame::SetJumpActor(int32 CharacterIndex, APTBJJJumpActor* JumpActor)
{
	if (CharacterIndex < 0)
	{
		return;
	}

	if (!JumpActors.IsValidIndex(CharacterIndex))
	{
		JumpActors.SetNum(CharacterIndex + 1);
	}
	JumpActors[CharacterIndex] = JumpActor;
}