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
#include "Math/RandomStream.h"

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
	ScheduledJumpNoteIds.Reset();
	NextSameActionNote.Reset();

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
	ScheduledJumpNoteIds.Reset();

	if (JumpSpeeds.Num() < 3)
	{
		JumpSpeeds.Init(1.0f, 3);
	}

	// 점프 캐릭터 스폰 + 등록
	SpawnJumpActors();

	// 채보 스캔 → 노트별 체공시간 산출
	PrecomputeAirtimes();

	UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] JJ BuildRuntimeState RuleSet=%s Chart=%s Insane=%d Chain=%d Ratio=%.2f Min=%.0f Max=%.0f Jitter=%.2f"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()),
		(bIsInsaneDifficulty || bForceInsaneAirtime) ? 1 : 0,
		IsChainJumpEnabled() ? 1 : 0,
		ActiveAirtimeProfile.AirtimeRatio,
		ActiveAirtimeProfile.MinAirtimeMs,
		ActiveAirtimeProfile.MaxAirtimeMs,
		ActiveAirtimeProfile.RatioJitter);
}

void APTBJJMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	++CueCount;
	TrackNote(CuedNotes, Note);

	const int32 CharacterIndex = ResolveCharacterIndex(Note.ActionType);
	OnJJJumpCue.Broadcast(CharacterIndex, Note);

	// 체인 모드에서는 이전 착지 시점에 이미 예약됐을 수 있음 → 내부에서 중복 무시
	ScheduleJumpForNote(Note, GetCurrentChartTimeMs());

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

	// Insane: 착지하자마자 같은 캐릭터의 다음 점프를 바로 예약
	// (Cue는 정시점 직전에만 오므로, Cue에만 의존하면 긴 간격에서 체공이 Cue 선행시간으로 잘린다)
	if (IsChainJumpEnabled())
	{
		if (const FPTBNoteEvent* NextNote = NextSameActionNote.Find(Note.NoteId))
		{
			ScheduleJumpForNote(*NextNote, GetCurrentChartTimeMs());
		}
	}

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
	// BuildRuntimeState(→ PrecomputeAirtimes)보다 먼저 난이도/시드를 세팅해야 한다
	// TODO: 프로젝트의 실제 난이도 enum 이름/값으로 맞출 것
	bIsInsaneDifficulty = (Context.SessionRequest.Difficulty == EPTBDifficulty::Insane);
	AirtimeSeed = Context.SessionRequest.RandomSeed;

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

FPTBJJAirtimeProfile APTBJJMiniGame::ResolveAirtimeProfile() const
{
	if (bIsInsaneDifficulty || bForceInsaneAirtime)
	{
		return InsaneAirtimeProfile;
	}

	// Easy / Normal: 기존 값 그대로 (채보로 난이도 조절)
	FPTBJJAirtimeProfile Profile;
	Profile.AirtimeRatio = AirtimeRatio;
	Profile.MinAirtimeMs = MinAirtimeMs;
	Profile.MaxAirtimeMs = MaxAirtimeMs;
	Profile.RatioJitter = 0.0f;
	return Profile;
}

bool APTBJJMiniGame::IsChainJumpEnabled() const
{
	// Easy / Normal은 기존 Cue 기반 동작 유지
	return bIsInsaneDifficulty || bForceInsaneAirtime;
}

void APTBJJMiniGame::ScheduleJumpForNote(const FPTBNoteEvent& Note, float NowMs)
{
	if (ScheduledJumpNoteIds.Contains(Note.NoteId))
	{
		return;
	}
	ScheduledJumpNoteIds.Add(Note.NoteId);

	const int32 CharacterIndex = ResolveCharacterIndex(Note.ActionType);
	const float SpeedScale = JumpSpeeds.IsValidIndex(CharacterIndex) ? JumpSpeeds[CharacterIndex] : 1.0f;
	const float TimeToReach = Note.TimeMs - NowMs;   // 지금부터 착지까지 실제 남은 시간

	// 1) 채보 간격 기반 희망 체공시간
	const float DesiredAirtime = GetAirtimeMsForNote(Note.NoteId);

	// 2) 실제 남은 시간으로 상한 (예약이 늦으면 그만큼만 떠야 늦지 않음)
	float AirtimeMs = FMath::Min(DesiredAirtime, TimeToReach);

	// 3) 하한 적용
	AirtimeMs = FMath::Max(AirtimeMs, ActiveAirtimeProfile.MinAirtimeMs);

	const float DelayMs = TimeToReach - AirtimeMs;

	const bool bHasActor = JumpActors.IsValidIndex(CharacterIndex) && JumpActors[CharacterIndex] != nullptr;

	UE_LOG(LogPTBMiniGames, Warning,
		TEXT("SCHEDULE NoteId=%d CharIdx=%d Desired=%.0f Airtime=%.0f Now=%.0f Target=%.0f TimeToReach=%.0f Delay=%.0f HasActor=%d"),
		Note.NoteId, CharacterIndex, DesiredAirtime, AirtimeMs, NowMs, Note.TimeMs, TimeToReach, DelayMs,
		bHasActor ? 1 : 0);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

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

void APTBJJMiniGame::PrecomputeAirtimes()
{
	NoteAirtimeMs.Reset();
	NextSameActionNote.Reset();
	ActiveAirtimeProfile = ResolveAirtimeProfile();

	if (!ChartAsset)
	{
		return;
	}

	const FPTBJJAirtimeProfile& Profile = ActiveAirtimeProfile;
	const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;

	// 시드 고정 랜덤: 같은 세션(Retry 포함)이면 같은 체공 패턴
	FRandomStream Stream(AirtimeSeed);

	// 노트 N의 점프는 "직전 같은 액션 노트 착지(N-1) ~ N 정시점" 사이에서 일어나야 하므로
	// 이전 같은 액션 노트와의 간격을 기준으로 체공시간을 계산한다.
	// (다음 노트 기준으로 계산하면 이전 점프가 착지하기 전에 새 점프가 덮어쓸 수 있음)
	TMap<EPTBActionType, float> PrevSameActionTimeMs;
	TMap<EPTBActionType, int32> PrevSameActionNoteId;

	for (const FPTBNoteEvent& Note : Notes)
	{
		// 이전 같은 액션 노트 → 이 노트 연결 (착지 시 다음 점프 예약용)
		if (const int32* PrevId = PrevSameActionNoteId.Find(Note.ActionType))
		{
			NextSameActionNote.Add(*PrevId, Note);
		}
		PrevSameActionNoteId.Add(Note.ActionType, Note.NoteId);

		float Airtime = Profile.MaxAirtimeMs;

		if (const float* PrevTime = PrevSameActionTimeMs.Find(Note.ActionType))
		{
			const float Gap = Note.TimeMs - *PrevTime;
			if (Gap > 0.0f)
			{
				float Ratio = Profile.AirtimeRatio;
				if (Profile.RatioJitter > 0.0f)
				{
					Ratio = FMath::Clamp(Ratio + Stream.FRandRange(-Profile.RatioJitter, Profile.RatioJitter), 0.1f, 1.0f);
				}

				Airtime = FMath::Min(Profile.MaxAirtimeMs, Gap * Ratio);
			}
		}

		// 하한 클램프(너무 짧은 점프 방지)
		Airtime = FMath::Max(Airtime, Profile.MinAirtimeMs);

		NoteAirtimeMs.Add(Note.NoteId, Airtime);
		PrevSameActionTimeMs.Add(Note.ActionType, Note.TimeMs);
	}
}

float APTBJJMiniGame::GetAirtimeMsForNote(int32 NoteId) const
{
	if (const float* Found = NoteAirtimeMs.Find(NoteId))
	{
		return *Found;
	}
	return ActiveAirtimeProfile.MaxAirtimeMs;
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