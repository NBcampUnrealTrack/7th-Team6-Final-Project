#include "MiniGames/BB/PTBBBHUDWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "MiniGames/BB/PTBBBMiniGame.h"
#include "MiniGames/BB/PTBBBCueWidgetBase.h"
#include "Debug/PTBTeamLog.h"

// ── 연결 / 해제 ──────────────────────────────────────────────────

void UPTBBBHUDWidget::BindToMiniGame(APTBBBMiniGame* InMiniGame)
{
	if (!InMiniGame)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBHUDWidget] BindToMiniGame: null MiniGame"));
		return;
	}

	// 이미 다른 미니게임에 바인딩되어 있다면 먼저 해제
	if (IsValid(BBMiniGame) && BBMiniGame != InMiniGame)
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBNoteCue);
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBParrySuccess);
		BBMiniGame->OnBBParryFail.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBParryFail);
		BBMiniGame->OnBBBossHPChanged.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBBossHPChanged);
		BBMiniGame->OnBBPlayerHPChanged.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBPlayerHPChanged);
		BBMiniGame->OnBBBossDefeated.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBBossDefeated);
	}

	BBMiniGame = InMiniGame;

	InMiniGame->OnBBNoteCue.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBNoteCue);
	InMiniGame->OnBBParrySuccess.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBParrySuccess);
	InMiniGame->OnBBParryFail.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBParryFail);
	InMiniGame->OnBBBossHPChanged.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBBossHPChanged);
	InMiniGame->OnBBPlayerHPChanged.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBPlayerHPChanged);
	InMiniGame->OnBBBossDefeated.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBBossDefeated);
}

void UPTBBBHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBNoteCue);
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBParrySuccess);
		BBMiniGame->OnBBParryFail.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBParryFail);
		BBMiniGame->OnBBBossHPChanged.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBBossHPChanged);
		BBMiniGame->OnBBPlayerHPChanged.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBPlayerHPChanged);
		BBMiniGame->OnBBBossDefeated.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBBossDefeated);
	}
}

// ── 앵커 좌표 ────────────────────────────────────────────────────

FVector2D UPTBBBHUDWidget::GetAnchorCanvasPosition_Implementation(EPTBActionType ActionType) const
{
	if (const FVector2D* Pos = AnchorPositions.Find(ActionType))
	{
		return *Pos;
	}
	// 설정되지 않은 경우 화면 중앙 근처의 기본값 반환
	return FVector2D(640.f, 360.f);
}

// ── 델리게이트 핸들러 ────────────────────────────────────────────

void UPTBBBHUDWidget::HandleBBNoteCue(FPTBNoteEvent Note)
{
	if (!CueLayer) return;

	const TSubclassOf<UPTBBBCueWidgetBase> CueClass =
		Note.bIsLongNote ? HoldCueClass : TapCueClass;

	UPTBBBCueWidgetBase* Cue = SpawnAndPlaceCue(CueClass, Note);
	if (!Cue) return;

	// 해당 맵에 등록
	TMap<int32, TObjectPtr<UPTBBBCueWidgetBase>>& TargetMap =
		Note.bIsLongNote ? HoldCueMap : TapCueMap;
	TargetMap.Add(Note.NoteId, Cue);

	OnCueSpawned(Cue, Note);
}

void UPTBBBHUDWidget::HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent)
{
	if (UPTBBBCueWidgetBase* Cue = FindAndRemoveCue(Result.NoteId))
	{
		Cue->OnJudgement(Result.JudgementType);
	}
	OnParrySuccessEffect(Result, BossHPPercent);
}

void UPTBBBHUDWidget::HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent)
{
	if (UPTBBBCueWidgetBase* Cue = FindAndRemoveCue(Result.NoteId))
	{
		Cue->OnJudgement(EPTBJudgementType::Miss);
	}
	OnParryFailEffect(Result, PlayerHPPercent);
}

void UPTBBBHUDWidget::HandleBBBossHPChanged(float NewHP, float MaxHP)
{
	if (BossHPBar && MaxHP > 0.f)
	{
		BossHPBar->SetPercent(NewHP / MaxHP);
	}
	OnBossHPUpdated(NewHP, MaxHP);
}

void UPTBBBHUDWidget::HandleBBPlayerHPChanged(float NewHP, float MaxHP)
{
	if (PlayerHPBar && MaxHP > 0.f)
	{
		PlayerHPBar->SetPercent(NewHP / MaxHP);
	}
	OnPlayerHPUpdated(NewHP, MaxHP);
}

void UPTBBBHUDWidget::HandleBBBossDefeated()
{
	OnBossDefeatedEffect();
}

// ── 내부 헬퍼 ────────────────────────────────────────────────────

UPTBBBCueWidgetBase* UPTBBBHUDWidget::SpawnAndPlaceCue(
	TSubclassOf<UPTBBBCueWidgetBase> CueClass,
	const FPTBNoteEvent& Note)
{
	if (!CueClass || !CueLayer) return nullptr;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBHUDWidget] SpawnAndPlaceCue: OwningPlayer is null"));
		return nullptr;
	}

	UPTBBBCueWidgetBase* Cue = CreateWidget<UPTBBBCueWidgetBase>(PC, CueClass);
	if (!Cue) return nullptr;

	// CanvasPanel에 자식으로 추가
	const FVector2D AnchorPos = GetAnchorCanvasPosition(Note.ActionType);
	UCanvasPanelSlot* CueSlot = CueLayer->AddChildToCanvas(Cue);
	if (CueSlot)
	{
		CueSlot->SetPosition(AnchorPos);
	}

	// 보스 X → 앵커 X 방향으로 이동할 초기 Translation 오프셋 전달
	// Blueprint OnCueStarted에서 SetRenderTranslation + Timeline으로 보간한다
	Cue->ApproachStartTranslationX = BossSpawnCanvasPosition.X - AnchorPos.X;

	// 큐 초기화 (NoteId, ActionType 설정 + OnCueStarted 발행)
	// ApproachDurationSec 계산: TG HUD와 동일한 방식
	float ApproachDurationSec = 0.f;
	if (IsValid(BBMiniGame))
	{
		const float RemainingMs = Note.TimeMs - BBMiniGame->GetCurrentChartTimeMs();
		ApproachDurationSec = FMath::Max(0.f, RemainingMs / 1000.f);
	}

	Cue->InitCue(Note.NoteId, Note.ActionType, ApproachDurationSec);

	return Cue;
}

UPTBBBCueWidgetBase* UPTBBBHUDWidget::FindAndRemoveCue(int32 NoteId)
{
	if (TObjectPtr<UPTBBBCueWidgetBase>* Found = TapCueMap.Find(NoteId))
	{
		UPTBBBCueWidgetBase* Cue = Found->Get();
		TapCueMap.Remove(NoteId);
		return Cue;
	}
	if (TObjectPtr<UPTBBBCueWidgetBase>* Found = HoldCueMap.Find(NoteId))
	{
		UPTBBBCueWidgetBase* Cue = Found->Get();
		HoldCueMap.Remove(NoteId);
		return Cue;
	}
	return nullptr;
}
