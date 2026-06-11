#include "MiniGames/BB/PTBBBCueWidgetBase.h"

UPTBBBCueWidgetBase::UPTBBBCueWidgetBase()
{
	// NativeTick이 호출되도록 강제 활성화
	// (Blueprint에 Event Tick 노드가 없어도 tick 보장)
	bHasScriptImplementedTick = true;
}

void UPTBBBCueWidgetBase::InitCue(int32 InNoteId, EPTBActionType InActionType, float InApproachDurationSec)
{
	NoteId = InNoteId;
	SetActionType(InActionType);   // OnActionTypeSet() 호출 → BP에서 색·텍스트 반영
	OnCueStarted(InNoteId, InActionType, InApproachDurationSec);

	// OnCueStarted(BP)가 완료된 후 Translation 이동 시작
	if (InApproachDurationSec > KINDA_SMALL_NUMBER && !FMath::IsNearlyZero(ApproachStartTranslationX))
	{
		ApproachTotalDurationSec = InApproachDurationSec;
		ApproachElapsedSec = 0.f;
		bApproaching = true;
		SetRenderTranslation(FVector2D(ApproachStartTranslationX, 0.f));
	}
}

void UPTBBBCueWidgetBase::RemoveCue()
{
	OnCueRemoved();
	RemoveFromParent();
}

void UPTBBBCueWidgetBase::StopApproachTranslation()
{
	bApproaching = false;
	SetRenderTranslation(FVector2D::ZeroVector);
}

void UPTBBBCueWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bApproaching || ApproachTotalDurationSec <= 0.f) return;

	ApproachElapsedSec = FMath::Min(ApproachElapsedSec + InDeltaTime, ApproachTotalDurationSec);
	const float Alpha = ApproachElapsedSec / ApproachTotalDurationSec;
	SetRenderTranslation(FVector2D(FMath::Lerp(ApproachStartTranslationX, 0.f, Alpha), 0.f));

	if (Alpha >= 1.f)
	{
		bApproaching = false;
	}
}
