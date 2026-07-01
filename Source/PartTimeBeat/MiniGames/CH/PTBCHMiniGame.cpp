#include "MiniGames/CH/PTBCHMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "MiniGames/CH/PTBCHMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Blueprint/UserWidget.h"
#include "Components/PrimitiveComponent.h"

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
    // CH 전용 초기화
    CurrentCursorIndex = 0;
    CurrentCustomerIndex = 0;
    PlacedIngredients.Reset();
    CustomerOrders.Reset();

    // 손님 주문 고정 (채보에 맞춤)
// 1: 빵→패티→빵
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }

    NoteIdToIngredientAction.Reset();
    SlideGroups.Reset();
    DroppingIngredients.Reset();

    // ChartAsset NoteEvents 순서 기반으로 NoteId → IngredientAction 사전 매핑
    // HandleNoteCue 시점에 맵에 없어서 bCorrectKey가 false되는 레이스 컨디션 방지
    if (ChartAsset.Get() && ChartAsset->NoteEvents.Num() > 0)
    {
        TArray<EPTBCHIngredientType> AllIngredients;
        for (const FPTBCHCustomerOrder& Order : CustomerOrders)
        {
            for (EPTBCHIngredientType Ing : Order.Ingredients)
            {
                AllIngredients.Add(Ing);
            }
        }

        for (int32 i = 0; i < ChartAsset->NoteEvents.Num() && i < AllIngredients.Num(); ++i)
        {
            EPTBActionType ActionType = EPTBActionType::None;
            switch (AllIngredients[i])
            {
            case EPTBCHIngredientType::BreadBottom: ActionType = EPTBActionType::ActionA; break;
            case EPTBCHIngredientType::BreadTop:    ActionType = EPTBActionType::ActionA; break;
            case EPTBCHIngredientType::Lettuce:     ActionType = EPTBActionType::ActionB; break;
            case EPTBCHIngredientType::Patty:       ActionType = EPTBActionType::ActionC; break;
            case EPTBCHIngredientType::Cheese:      ActionType = EPTBActionType::ActionD; break;
            case EPTBCHIngredientType::Tomato:      ActionType = EPTBActionType::ActionE; break;
            default: break;
            }
            if (ActionType != EPTBActionType::None)
            {
                NoteIdToIngredientAction.Add(ChartAsset->NoteEvents[i].NoteId, ActionType);
            }
        }

        UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH NoteIdToIngredientAction pre-mapped: %d entries"),
            *GetNameSafe(this), NoteIdToIngredientAction.Num());
    }

    // 첫 번째 접시 스폰
    if (PlateActorClass && GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* FirstPlate = GetWorld()->SpawnActor<AActor>(PlateActorClass, FVector(1240.0f, 0.0f, 5.0f), FRotator::ZeroRotator, SpawnParams);
        if (FirstPlate)
        {
            FCHSlideGroup Group;
            Group.Plate = FirstPlate;
            Group.TargetX = 840.0f;
            SlideGroups.Add(Group);
            CompletedPlates.Add(FirstPlate);

            // StackHeight: 접시 윗면 기준
            FVector PlateOrigin, PlateExtent;
            FirstPlate->GetActorBounds(true, PlateOrigin, PlateExtent);
            StackHeight = PlateOrigin.Z + PlateExtent.Z;
        }
    }

    CreateAndAddHUD();

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

    if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId))
    {
        FPTBNoteEvent RemappedNote = Note;
        RemappedNote.ActionType = *Mapped;
        OnCHNoteReached.Broadcast(RemappedNote);
    }
    else
    {
        OnCHNoteReached.Broadcast(Note);
    }

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

    if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId))
    {
        FPTBNoteEvent RemappedNote = Note;
        RemappedNote.ActionType = *Mapped;
        OnCHNoteArm.Broadcast(RemappedNote);
    }
    else
    {
        OnCHNoteArm.Broadcast(Note);
    }

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

    // 사전 매핑 완료 상태에서 NoteId가 맵에 없으면 이미 판정된 것(HandleJudgementResult에서 제거)
    // → Cue가 늦게 발동돼도 슬롯을 다시 켜지 않음 (노란불 잔류 버그 방지)
    const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId);
    if (!Mapped && NoteIdToIngredientAction.Num() > 0)
    {
        return;
    }

    if (Mapped)
    {
        FPTBNoteEvent ModifiedNote = Note;
        ModifiedNote.ActionType = *Mapped;
        OnCHNoteCue.Broadcast(ModifiedNote);
    }
    else
    {
        OnCHNoteCue.Broadcast(Note);
    }

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
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] HandleJudgementResult: NoteId=%d bHasJudgedNote=%d Reason=%d"),
        Result.NoteId, bHasJudgedNote ? 1 : 0, static_cast<int32>(Result.Reason));

    // 키 정확도 판정 (Super 호출 전에 계산해서 점수에도 반영)
    EPTBActionType ExpectedIngredientAction = EPTBActionType::None;
    if (Result.NoteId != 0)
    {
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            ExpectedIngredientAction = *Mapped;
        }
    }
    const bool bCorrectKey = (Result.Reason != EPTBJudgementReason::EmptyInput
        && LastPressedAction != EPTBActionType::None
        && ExpectedIngredientAction != EPTBActionType::None
        && ExpectedIngredientAction == LastPressedAction);

    // 타이밍이 맞아도 틀린 키면 Miss 처리
    if (Result.NoteId != 0 && Result.Reason == EPTBJudgementReason::Note && !bCorrectKey)
    {
        Result.JudgementType = EPTBJudgementType::Miss;
        Result.ScoreDelta = 0;
        Result.bBreaksCombo = true;
    }

    Super::HandleJudgementResult(Result);
    CallHUDShowJudgement(Result.JudgementType);
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] Result.ActionType=%d, NoteId=%d"),
        static_cast<int32>(Result.ActionType), Result.NoteId);

    //햄버거 스폰
    if (Result.NoteId != 0)
    {
        UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] SpawnCheck: bHasJudged=%d NoteId=%d Reason=%d LastPressedAction=%d bCorrectKey=%d"),
            bHasJudgedNote ? 1 : 0,
            Result.NoteId,
            static_cast<int32>(Result.Reason),
            static_cast<int32>(LastPressedAction),
            bCorrectKey ? 1 : 0);

        if (bCorrectKey)
        {
            EPTBCHIngredientType IngredientType = EPTBCHIngredientType::None;
            if (CustomerOrders.IsValidIndex(CurrentCustomerIndex) &&
                CustomerOrders[CurrentCustomerIndex].Ingredients.IsValidIndex(CurrentIngredientIndex))
            {
                IngredientType = CustomerOrders[CurrentCustomerIndex].Ingredients[CurrentIngredientIndex];
            }
            if (IngredientType != EPTBCHIngredientType::None)
            {
                bool bIsLastBread = (CurrentIngredientIndex >= CustomerOrders[CurrentCustomerIndex].Ingredients.Num() - 1);
                SpawnIngredient(IngredientType, bIsLastBread);
            }
        }

        // 맞든 틀리든 노트가 지나가면 다음 재료로 진행
        if (Result.Reason != EPTBJudgementReason::EmptyInput)
        {
            CurrentIngredientIndex++;
            if (CustomerOrders.IsValidIndex(CurrentCustomerIndex) &&
                CurrentIngredientIndex >= CustomerOrders[CurrentCustomerIndex].Ingredients.Num())
            {
                // 모든 그룹을 왼쪽으로 400 슬라이드
                // 방금 완성된 접시(윗빵이 막 올라간 것)는 0.5초 딜레이 후 출발
                AActor* JustCompletedPlate = CompletedPlates.Num() > 0 ? CompletedPlates.Last().Get() : nullptr;
                for (FCHSlideGroup& Group : SlideGroups)
                {
                    if (!Group.Plate) continue;
                    Group.TargetX -= 400.0f;
                    Group.Delay = (Group.Plate == JustCompletedPlate) ? 0.5f : 0.0f;
                }

                // 새 접시 스폰
                if (PlateActorClass && GetWorld())
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    AActor* NewPlate = GetWorld()->SpawnActor<AActor>(PlateActorClass, FVector(1240.0f, 0.0f, 5.0f), FRotator::ZeroRotator, SpawnParams);
                    if (NewPlate)
                    {
                        FCHSlideGroup NewGroup;
                        NewGroup.Plate = NewPlate;
                        NewGroup.TargetX = 840.0f;
                        NewGroup.Delay = 0.5f;
                        SlideGroups.Add(NewGroup);
                        CompletedPlates.Add(NewPlate);
                    }
                }

                CompletedHamburgers.Add(SpawnedIngredients);
                SpawnedIngredients.Reset();
                CurrentIngredientIndex = 0;
                CurrentCustomerIndex++;
                OnCHCustomerChanged.Broadcast(CurrentCustomerIndex + 1);
                CallHUDUpdateCustomerNumber(CurrentCustomerIndex + 1);

                // StackHeight: 새 접시 윗면 기준으로 리셋
                if (CompletedPlates.Num() > 0 && CompletedPlates.Last())
                {
                    FVector PlateOrigin, PlateExtent;
                    CompletedPlates.Last()->GetActorBounds(true, PlateOrigin, PlateExtent);
                    StackHeight = PlateOrigin.Z + PlateExtent.Z;
                }
            }
        }

        LastPressedAction = EPTBActionType::None;
    }

    ++JudgementCount;
    if (bHasJudgedNote)
    {
        RemoveTrackedNote(CuedNotes, Result.NoteId);
        RemoveTrackedNote(ArmedNotes, Result.NoteId);
        RemoveTrackedNote(ReachedNotes, Result.NoteId);

        // 리매핑된 ActionType으로 브로드캐스트해야 HUD가 올바른 재료 슬롯을 클리어함
        // (차트 원본 ActionA로 브로드캐스트하면 HandleCHJudgement가 항상 SlotBread만 클리어)
        EPTBActionType RemappedActionType = Result.ActionType;
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            RemappedActionType = *Mapped;
        }
        NoteIdToIngredientAction.Remove(Result.NoteId);

        FPTBJudgementResult BroadcastResult = Result;
        BroadcastResult.ActionType = RemappedActionType;
        OnCHJudgement.Broadcast(BroadcastResult, JudgedNote);
        OnCHNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);

        // 같은 ActionType에 이미 Cue된 다음 노트가 있으면 다시 Cue 브로드캐스트
        // (BreadTop 판정이 ActionA 슬롯을 끄면, 이미 Cue된 BreadBottom 슬롯이 같이 꺼지는 버그 방지)
        for (const FPTBNoteEvent& CuedNote : CuedNotes)
        {
            if (const EPTBActionType* NextMapped = NoteIdToIngredientAction.Find(CuedNote.NoteId))
            {
                if (*NextMapped == RemappedActionType)
                {
                    FPTBNoteEvent ReCueNote = CuedNote;
                    ReCueNote.ActionType = *NextMapped;
                    OnCHNoteCue.Broadcast(ReCueNote);
                    break;
                }
            }
        }
    }
    else if (Result.Reason == EPTBJudgementReason::EmptyInput)
    {
        FPTBNoteEvent EmptyInputNote;
        OnCHJudgement.Broadcast(Result, EmptyInputNote);
    }
    else if (Result.NoteId != 0)
    {
        // 판정이 Arm/Cue 틱보다 먼저 실행된 경우 — 맵 제거 + HUD 클리어
        EPTBActionType RemappedActionType = Result.ActionType;
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            RemappedActionType = *Mapped;
        }
        NoteIdToIngredientAction.Remove(Result.NoteId);

        FPTBJudgementResult BroadcastResult = Result;
        BroadcastResult.ActionType = RemappedActionType;
        FPTBNoteEvent EmptyNote;
        OnCHJudgement.Broadcast(BroadcastResult, EmptyNote);
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

void APTBCHMiniGame::HandleReadyToStart()
{
    UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH HandleReadyToStart called. HUDWidget=%s"), *GetNameSafe(this), *GetNameSafe(HUDWidget.Get()));

    Super::HandleReadyToStart();

    if (HUDWidget)
    {
        if (UFunction* InitFunc = HUDWidget->FindFunction(TEXT("Init")))
        {
            struct { APTBCHMiniGame* InMiniGame; } Params{ this };
            HUDWidget->ProcessEvent(InitFunc, &Params);
        }
        else
        {
            UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] CH Init function NOT found!"), *GetNameSafe(this));
        }
    }

    OnCHCustomerChanged.Broadcast(1);
    CallHUDUpdateCustomerNumber(1);
}

void APTBCHMiniGame::CallHUDShowJudgement(EPTBJudgementType JudgementType)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("ShowJudgement")))
    {
        struct { EPTBJudgementType InJudgementType; } Params{ JudgementType };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CallHUDUpdateCustomerNumber(int32 CustomerNumber)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("UpdateCustomerNumber")))
    {
        struct { int32 InCustomerNumber; } Params{ CustomerNumber };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CreateAndAddHUD()
{
    if (!HUDWidgetClass)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] CH HUDWidgetClass is not set!"), *GetNameSafe(this));
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
    if (HUDWidget)
    {
        HUDWidget->AddToViewport();
        UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH HUD created and added to viewport."), *GetNameSafe(this));
    }
}

void APTBCHMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] HandleRhythmInput called. Action=%d"), static_cast<int32>(Action));
    if (!CanAcceptInput()) return;

    const float ResolvedTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();

    FPTBNoteEvent BestNote;
    bool bFound = false;
    float BestDelta = TNumericLimits<float>::Max();

    for (const EPTBActionType ActionType : {
        EPTBActionType::ActionA, EPTBActionType::ActionB,
            EPTBActionType::ActionC, EPTBActionType::ActionD,
            EPTBActionType::ActionE })
    {
        FPTBNoteEvent Note;
        if (JudgementSystem->FindBestPendingNote(ActionType, ResolvedTimeMs, Note))
        {
            const float Delta = FMath::Abs(ResolvedTimeMs - Note.TimeMs);
            if (Delta < BestDelta)
            {
                BestDelta = Delta;
                BestNote = Note;
                bFound = true;
            }
        }
    }

    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] Input: Pressed=%d bFound=%d BestNoteId=%d BestNoteAction=%d BestDelta=%.1f"),
        static_cast<int32>(Action), bFound ? 1 : 0, BestNote.NoteId, static_cast<int32>(BestNote.ActionType), bFound ? BestDelta : -1.f);

    if (bFound)
    {
        LastPressedAction = Action;
        Super::HandleRhythmInput(BestNote.ActionType, TimeMs);
    }
    else
    {
        Super::HandleRhythmInput(Action, TimeMs);
    }
}

void APTBCHMiniGame::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 드롭 애니메이션 (스프링 물리: 가속 → 착지 → 미세 바운스 → 안정화)
    for (FCHDropAnim& Anim : DroppingIngredients)
    {
        if (Anim.bDone || !Anim.Ingredient) continue;

        TArray<USceneComponent*> DropSC;
        Anim.Ingredient->GetComponents<USceneComponent>(DropSC);
        if (DropSC.Num() == 0) continue;

        const float CurrentZ = DropSC[0]->GetComponentLocation().Z;

        // 중력으로 가속
        Anim.Velocity -= DropGravity * DeltaTime;
        float NewZ = CurrentZ + Anim.Velocity * DeltaTime;

        // 착지면(TargetZ) 도달 시 아래로 통과하지 않고 위로 튕김
        if (NewZ <= Anim.TargetZ)
        {
            NewZ = Anim.TargetZ;
            if (Anim.Velocity < 0.0f)
            {
                Anim.Velocity = -Anim.Velocity * DropRestitution;
            }
        }

        const float DeltaZ = NewZ - CurrentZ;
        for (USceneComponent* SC : DropSC)
        {
            if (SC)
            {
                FVector Loc = SC->GetComponentLocation();
                SC->SetWorldLocation(FVector(Loc.X, Loc.Y, Loc.Z + DeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
            }
        }

        // 착지 후 속도가 충분히 작으면 완료
        if (NewZ <= Anim.TargetZ + 0.5f && FMath::Abs(Anim.Velocity) < 8.0f)
        {
            const float SnapDeltaZ = Anim.TargetZ - NewZ;
            for (USceneComponent* SC : DropSC)
            {
                if (SC)
                {
                    FVector Loc = SC->GetComponentLocation();
                    SC->SetWorldLocation(FVector(Loc.X, Loc.Y, Loc.Z + SnapDeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
                }
            }
            Anim.bDone = true;
        }
    }
    DroppingIngredients.RemoveAll([](const FCHDropAnim& A) { return A.bDone; });

    for (FCHSlideGroup& Group : SlideGroups)
    {
        if (!Group.Plate) continue;

        if (Group.Delay > 0.0f)
        {
            Group.Delay -= DeltaTime;
            continue;
        }

        const FVector PlateLoc = Group.Plate->GetActorLocation();
        if (FMath::IsNearlyEqual(PlateLoc.X, Group.TargetX, 1.0f)) continue;

        const float NewX = FMath::FInterpConstantTo(PlateLoc.X, Group.TargetX, DeltaTime, SlideSpeed);
        const float DeltaX = NewX - PlateLoc.X;

        Group.Plate->SetActorLocation(FVector(NewX, PlateLoc.Y, PlateLoc.Z), false, nullptr, ETeleportType::TeleportPhysics);
        for (AActor* Ingredient : Group.Ingredients)
        {
            if (!Ingredient) continue;
            TArray<USceneComponent*> IngSC;
            Ingredient->GetComponents<USceneComponent>(IngSC);
            for (USceneComponent* SC : IngSC)
            {
                if (SC)
                {
                    FVector Loc = SC->GetComponentLocation();
                    SC->SetWorldLocation(FVector(Loc.X + DeltaX, Loc.Y, Loc.Z), false, nullptr, ETeleportType::TeleportPhysics);
                }
            }
        }

        if (FMath::IsNearlyEqual(NewX, Group.TargetX, 1.0f))
        {
            Group.Plate->SetActorLocation(FVector(Group.TargetX, PlateLoc.Y, PlateLoc.Z), false, nullptr, ETeleportType::TeleportPhysics);
            for (AActor* Ingredient : Group.Ingredients)
            {
                if (!Ingredient) continue;
                TArray<USceneComponent*> IngSC;
                Ingredient->GetComponents<USceneComponent>(IngSC);
                for (USceneComponent* SC : IngSC)
                {
                    if (SC)
                    {
                        FVector Loc = SC->GetComponentLocation();
                        SC->SetWorldLocation(FVector(Group.TargetX, Loc.Y, Loc.Z), false, nullptr, ETeleportType::TeleportPhysics);
                    }
                }
            }
        }
    }
}

void APTBCHMiniGame::SpawnIngredient(EPTBCHIngredientType IngredientType, bool bIsLastBread)
{
    TSubclassOf<AActor> ClassToSpawn = nullptr;

    switch (IngredientType)
    {
    case EPTBCHIngredientType::BreadBottom:
        ClassToSpawn = IngredientClass_BreadBottom;
        break;
    case EPTBCHIngredientType::BreadTop:
        ClassToSpawn = IngredientClass_BreadTop;
        break;
    case EPTBCHIngredientType::Lettuce:
        ClassToSpawn = IngredientClass_Lettuce;
        break;
    case EPTBCHIngredientType::Patty:
        ClassToSpawn = IngredientClass_Patty;
        break;
    case EPTBCHIngredientType::Cheese:
        ClassToSpawn = IngredientClass_Cheese;
        break;
    case EPTBCHIngredientType::Tomato:
        ClassToSpawn = IngredientClass_Tomato;
        break;
    default:
        return;
    }

    if (!ClassToSpawn || !GetWorld())
    {
        return;
    }

    // SM_Plate X, Y 기준으로 스폰
    FVector SpawnLocation(840.0f, 0.0f, StackHeight);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
    if (SpawnedActor)
    {
        // 스폰 즉시 모든 컴포넌트 절대 위치 모드 + 물리 비활성화
        // → Tick에서 SetWorldLocation으로 직접 이동 제어 (BP 설정 무관)
        TArray<USceneComponent*> SpawnSC;
        SpawnedActor->GetComponents<USceneComponent>(SpawnSC);
        for (USceneComponent* SC : SpawnSC)
        {
            if (!SC) continue;
            SC->SetAbsolute(true, false, false);
            if (UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(SC))
            {
                PC->SetSimulatePhysics(false);
            }
        }

        SpawnedIngredients.Add(SpawnedActor);
        if (SlideGroups.Num() > 0)
        {
            SlideGroups.Last().Ingredients.Add(SpawnedActor);
        }

        // TargetZ = 접시 윗면(StackHeight) + 재료 반높이 → 재료 바닥이 접시 위에 딱 닿도록
        FVector Origin, BoxExtent;
        SpawnedActor->GetActorBounds(true, Origin, BoxExtent);
        const float TargetZ = StackHeight + BoxExtent.Z;
        StackHeight += BoxExtent.Z * 2.0f;

        // 드롭 시작 위치(TargetZ + 오프셋)로 배치 후 애니메이션 등록
        TArray<USceneComponent*> DropSC;
        SpawnedActor->GetComponents<USceneComponent>(DropSC);
        for (USceneComponent* SC : DropSC)
        {
            if (SC)
            {
                FVector Loc = SC->GetComponentLocation();
                SC->SetWorldLocation(FVector(Loc.X, Loc.Y, TargetZ + DropStartOffset), false, nullptr, ETeleportType::TeleportPhysics);
            }
        }
        FCHDropAnim Anim;
        Anim.Ingredient = SpawnedActor;
        Anim.TargetZ = TargetZ;
        Anim.Velocity = 0.0f;
        DroppingIngredients.Add(Anim);
    }
}