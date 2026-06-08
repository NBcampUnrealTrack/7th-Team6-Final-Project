#include "MiniGames/CH/PTBCHMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "MiniGames/CH/PTBCHMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"

void APTBCHMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();

    CueCount = 0;
    ArmCount = 0;
    NoteEventCount = 0;
    LongNoteCueCount = 0;
    JudgementCount = 0;
    CuedNotes.Reset();
    ArmedNotes.Reset();
    ReachedNotes.Reset();

    UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH BuildRuntimeState RuleSet=%s Chart=%s"),
        *GetNameSafe(this),
        *GetNameSafe(RuleSet.Get()),
        *GetNameSafe(ChartAsset.Get()));
}

void APTBCHMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);

    ++NoteEventCount;
    TrackNote(ReachedNotes, Note);
    OnCHNoteReached.Broadcast(Note);

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteEvent)
    {
        LogNoteDebug(TEXT("NoteEvent"), Note);
    }
}

void APTBCHMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);

    ++ArmCount;
    TrackNote(ArmedNotes, Note);
    OnCHNoteArm.Broadcast(Note);

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteArm)
    {
        LogNoteDebug(TEXT("NoteArm"), Note);
    }
}

void APTBCHMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    ++CueCount;
    if (Note.bIsLongNote)
    {
        ++LongNoteCueCount;
    }

    TrackNote(CuedNotes, Note);
    OnCHNoteCue.Broadcast(Note);

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteCue)
    {
        LogNoteDebug(TEXT("NoteCue"), Note);
    }
}

void APTBCHMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    FPTBNoteEvent JudgedNote;
    const bool bHasJudgedNote = FindTrackedNote(Result.NoteId, JudgedNote);

    Super::HandleJudgementResult(Result);

    ++JudgementCount;
    if (bHasJudgedNote)
    {
        RemoveTrackedNote(CuedNotes, Result.NoteId);
        RemoveTrackedNote(ArmedNotes, Result.NoteId);
        RemoveTrackedNote(ReachedNotes, Result.NoteId);

        OnCHJudgement.Broadcast(Result, JudgedNote);
        OnCHNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);
    }
    else if (Result.Reason == EPTBJudgementReason::EmptyInput)
    {
        FPTBNoteEvent EmptyInputNote;
        OnCHJudgement.Broadcast(Result, EmptyInputNote);
    }

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogJudgement)
    {
        LogJudgementDebug(Result);
    }
}

void APTBCHMiniGame::HandleCHInput(EPTBActionType Action, float TimeMs)
{
    if (Action == EPTBActionType::None)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid CH action input."), *GetNameSafe(this));
        return;
    }

    HandleRhythmInput(Action, TimeMs);
}

void APTBCHMiniGame::HandleActionAInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionA, TimeMs);
}

void APTBCHMiniGame::HandleActionBInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionB, TimeMs);
}

void APTBCHMiniGame::HandleActionCInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionC, TimeMs);
}

void APTBCHMiniGame::HandleActionDInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionD, TimeMs);
}

void APTBCHMiniGame::HandleActionEInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionE, TimeMs);
}

void APTBCHMiniGame::HandleCHInputReleased(EPTBActionType Action, float TimeMs)
{
    if (Action == EPTBActionType::None)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid CH action release input."), *GetNameSafe(this));
        return;
    }

    HandleRhythmInputReleased(Action, TimeMs);
}

void APTBCHMiniGame::HandleActionAReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionA, TimeMs);
}

void APTBCHMiniGame::HandleActionBReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionB, TimeMs);
}

void APTBCHMiniGame::HandleActionCReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionC, TimeMs);
}

void APTBCHMiniGame::HandleActionDReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionD, TimeMs);
}

void APTBCHMiniGame::HandleActionEReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionE, TimeMs);
}

FPTBJudgementResult APTBCHMiniGame::EvaluateHoldInput(EPTBActionType Action, float TimeMs)
{
    FPTBNoteEvent HoldNote;
    const bool bHasHoldNote = JudgementSystem
        && JudgementSystem->FindBestPendingNote(Action, TimeMs, HoldNote)
        && HoldNote.NoteType == EPTBNoteType::Hold;

    const FPTBJudgementResult Result = Super::EvaluateHoldInput(Action, TimeMs);
    if (bHasHoldNote && Result.Reason == EPTBJudgementReason::Note && Result.JudgementType != EPTBJudgementType::Miss)
    {
        OnCHHoldStarted.Broadcast(Result, HoldNote);
    }

    return Result;
}

const UPTBCHMiniGameRuleSet* APTBCHMiniGame::GetCHRuleSet() const
{
    return Cast<UPTBCHMiniGameRuleSet>(RuleSet.Get());
}

void APTBCHMiniGame::TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note)
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

bool APTBCHMiniGame::RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId)
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

bool APTBCHMiniGame::FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const
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

void APTBCHMiniGame::LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const
{
    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    const bool bLogLongNoteDetails = !CHRuleSet || CHRuleSet->bLogLongNoteDetails;

    UE_LOG(LogPTBMiniGames, Log,
        TEXT("[%s] CH %s NoteId=%d Action=%d NoteType=%d Lane=%d Beat=%.3f TimeMs=%.3f Long=%d DurationBeat=%.3f ReleaseBeat=%.3f ReleaseTimeMs=%.3f"),
        *GetNameSafe(this),
        EventName,
        Note.NoteId,
        static_cast<int32>(Note.ActionType),
        static_cast<int32>(Note.NoteType),
        Note.Lane,
        Note.BeatTime,
        Note.TimeMs,
        Note.bIsLongNote ? 1 : 0,
        Note.DurationBeat,
        bLogLongNoteDetails ? Note.ReleaseBeatTime : 0.0f,
        bLogLongNoteDetails ? Note.ReleaseTimeMs : 0.0f);
}

void APTBCHMiniGame::LogJudgementDebug(const FPTBJudgementResult& Result) const
{
    UE_LOG(LogPTBMiniGames, Log,
        TEXT("[%s] CH Judgement NoteId=%d Action=%d Type=%d Reason=%d ChartMs=%.3f InputMs=%.3f DeltaMs=%.3f ScoreDelta=%d Count=%d"),
        *GetNameSafe(this),
        Result.NoteId,
        static_cast<int32>(Result.ActionType),
        static_cast<int32>(Result.JudgementType),
        static_cast<int32>(Result.Reason),
        Result.ChartTimeMs,
        Result.InputTimeMs,
        Result.DeltaMs,
        Result.ScoreDelta,
        JudgementCount);
}