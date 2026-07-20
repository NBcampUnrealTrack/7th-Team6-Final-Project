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

APTBJJMiniGame::APTBJJMiniGame()
{
	// 좌(-) / 중(0) / 우(+) — Y축으로 벌려놓은 임의값. 화면 구도 보고 조절.
	JumpSpawnTransforms.Reset();
	JumpSpawnTransforms.Add(FTransform(FVector(0.0f, -200.0f, 0.0f))); // 0: 좌
	JumpSpawnTransforms.Add(FTransform(FVector(0.0f, 0.0f, 0.0f))); // 1: 중
	JumpSpawnTransforms.Add(FTransform(FVector(0.0f, 200.0f, 0.0f))); // 2: 우
}

void APTBJJMiniGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 스폰된 Actor 자체는 base(RegisterSpawnedRoundActor)가 정리하므로 추적용 배열과 타이머만 비운다
	JumpActors.Reset();

	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& TimerHandle : PendingJumpTimers)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	PendingJumpTimers.Reset();

	Super::EndPlay(EndPlayReason);
}

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

	// 점프 캐릭터 스폰 + 등록
	SpawnJumpActors();

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

	const float SpeedScale = JumpSpeeds.IsValidIndex(CharacterIndex) ? JumpSpeeds[CharacterIndex] : 1.0f;
	const float NowMs = GetCurrentChartTimeMs();
	const float TargetMs = Note.TimeMs;
	const float TimeToReach = TargetMs - NowMs;   // 지금부터 착지까지 실제 남은 시간

	// 1) 채보 간격 기반 희망 체공시간
	const float DesiredAirtime = GetAirtimeMsForNote(Note.NoteId);

	// 2) 실제 남은 시간으로 상한 (Cue 선행이 짧으면 그만큼만 떠야 늦지 않음)
	float AirtimeMs = FMath::Min(DesiredAirtime, TimeToReach);

	// 3) 하한 적용
	AirtimeMs = FMath::Max(AirtimeMs, MinAirtimeMs);

	const float DelayMs = TimeToReach - AirtimeMs;

	const bool bHasActor = JumpActors.IsValidIndex(CharacterIndex) && JumpActors[CharacterIndex] != nullptr;

	UE_LOG(LogPTBMiniGames, Warning,
		TEXT("CUE NoteId=%d CharIdx=%d Desired=%.0f Airtime=%.0f Now=%.0f Target=%.0f TimeToReach=%.0f Delay=%.0f HasActor=%d Actors=%d"),
		Note.NoteId, CharacterIndex, DesiredAirtime, AirtimeMs, NowMs, TargetMs, TimeToReach, DelayMs,
		bHasActor ? 1 : 0, JumpActors.Num());

	if (UWorld* World = GetWorld())
	{
		if (DelayMs <= 0.0f)
		{
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

	// 캐릭터 인덱스를 먼저 구함(모든 판정 케이스 공통)
	const int32 CharacterIndex = bHasJudgedNote
		? ResolveCharacterIndex(JudgedNote.ActionType)
		: ResolveCharacterIndex(Result.ActionType);

	// 판정 색 플래시
	if (JumpActors.IsValidIndex(CharacterIndex) && JumpActors[CharacterIndex])
	{
		JumpActors[CharacterIndex]->FlashJudgementColor(Result.JudgementType);
	}

	if (bHasJudgedNote)
	{
		RemoveTrackedNote(CuedNotes, Result.NoteId);
		RemoveTrackedNote(ArmedNotes, Result.NoteId);
		RemoveTrackedNote(ReachedNotes, Result.NoteId);

		OnJJLanding.Broadcast(CharacterIndex, Result, JudgedNote);
		OnJJNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);
	}
	else if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
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
				// 간격의 일정 비율만 체공 → 다음 노트 전에 착지 여유
				// (비율을 곱한 뒤 상한으로 자른다)
				Airtime = FMath::Min(MaxAirtimeMs, Gap * AirtimeRatio);
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
	const bool bHasActor = JumpActors.IsValidIndex(CharacterIndex) && JumpActors[CharacterIndex] != nullptr;
	UE_LOG(LogPTBMiniGames, Warning,
		TEXT(">>> StartScheduledJump CharIdx=%d Airtime=%.0f HasActor=%d Now=%.0f"),
		CharacterIndex, AirtimeMs, bHasActor ? 1 : 0, GetCurrentChartTimeMs());

	if (bHasActor)
	{
		JumpActors[CharacterIndex]->StartJump(AirtimeMs, HeightScale);
	}

	TriggerCharacterJump(CharacterIndex, HeightScale);
}

void APTBJJMiniGame::SpawnJumpActors()
{
	UE_LOG(LogPTBMiniGames, Warning, TEXT("=== SpawnJumpActors ENTER === Class=%s SpawnPoints=%d"),
		*GetNameSafe(JumpActorClass), JumpSpawnTransforms.Num());

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogPTBMiniGames, Error, TEXT("SpawnJumpActors: World is null"));
		return;
	}

	if (!JumpActorClass)
	{
		UE_LOG(LogPTBMiniGames, Error, TEXT("SpawnJumpActors: JumpActorClass NOT SET — 디테일 패널에서 BP 지정 필요"));
		return;
	}

	for (TObjectPtr<APTBJJJumpActor>& Existing : JumpActors)
	{
		if (Existing) { Existing->Destroy(); }
	}
	JumpActors.Reset();

	for (int32 Index = 0; Index < JumpSpawnTransforms.Num(); ++Index)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APTBJJJumpActor* Spawned =
			World->SpawnActor<APTBJJJumpActor>(JumpActorClass, JumpSpawnTransforms[Index], Params);

		if (Spawned)
		{
			SetJumpActor(Index, Spawned);
			RegisterSpawnedRoundActor(Spawned);
			UE_LOG(LogPTBMiniGames, Warning, TEXT("SpawnJumpActors: spawned idx=%d at %s"),
				Index, *Spawned->GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogPTBMiniGames, Error, TEXT("SpawnJumpActors: FAILED idx=%d"), Index);
		}
	}

	UE_LOG(LogPTBMiniGames, Warning, TEXT("=== SpawnJumpActors DONE === total=%d"), JumpActors.Num());
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