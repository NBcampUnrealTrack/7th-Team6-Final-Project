#include "MiniGames/BB/PTBBBHUDWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MiniGames/BB/PTBBBMiniGame.h"
#include "MiniGames/BB/PTBBBCueWidgetBase.h"
#include "Debug/PTBTeamLog.h"
#include "TimerManager.h"

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
		BBMiniGame->OnMiniGameStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBIntroStarted);
		BBMiniGame->OnMiniGameGameplayStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBGameplayStarted);
		BBMiniGame->OnMiniGameOutroStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBOutroStarted);
		BBMiniGame->OnMiniGameOutroFinished.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBOutroFinished);
	}

	BBMiniGame = InMiniGame;

	InMiniGame->OnBBNoteCue.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBNoteCue);
	InMiniGame->OnBBParrySuccess.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBParrySuccess);
	InMiniGame->OnBBParryFail.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBParryFail);
	InMiniGame->OnBBBossHPChanged.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBBossHPChanged);
	InMiniGame->OnBBPlayerHPChanged.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBPlayerHPChanged);
	InMiniGame->OnBBBossDefeated.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBBossDefeated);
	InMiniGame->OnMiniGameStarted.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBIntroStarted);
	InMiniGame->OnMiniGameGameplayStarted.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBGameplayStarted);
	InMiniGame->OnMiniGameOutroStarted.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBOutroStarted);
	InMiniGame->OnMiniGameOutroFinished.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBOutroFinished);

	// 바인딩 시점의 HP로 메인 바 및 고스트 바 초기 동기화
	SyncBarsToMiniGame();
}

void UPTBBBHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Blueprint에 Event Tick 노드가 없어도 NativeTick이 호출되도록 강제 활성화
	bHasScriptImplementedTick = true;

	// BindToMiniGame이 AddToViewport 이전에 호출된 경우를 대비한 초기 동기화
	if (IsValid(BBMiniGame))
	{
		SyncBarsToMiniGame();
	}
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
		BBMiniGame->OnMiniGameStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBIntroStarted);
		BBMiniGame->OnMiniGameGameplayStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBGameplayStarted);
		BBMiniGame->OnMiniGameOutroStarted.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBOutroStarted);
		BBMiniGame->OnMiniGameOutroFinished.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBOutroFinished);
	}

	ClearCenterMessageTimers();
}

void UPTBBBHUDWidget::ShowCenterMessage(const FText& Message)
{
	OnCenterMessageShown(Message);
	ApplyCenterMessageText(Message);
}

void UPTBBBHUDWidget::ApplyCenterMessageText(const FText& Message)
{
	if (CenterMessageText)
	{
		if (CenterMessageText->TextDelegate.IsBound())
		{
			CenterMessageText->TextDelegate.Unbind();
		}

		CenterMessageText->SetText(Message);
		CenterMessageText->SetRenderOpacity(1.0f);
		CenterMessageText->SetRenderScale(FVector2D(1.0f, 1.0f));
		CenterMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UPTBBBHUDWidget::HideCenterMessage()
{
	bCenterMessageHoldActive = false;
	CenterMessageHoldRemainingSeconds = 0.0f;
	CenterMessageHoldText = FText::GetEmpty();

	if (CenterMessageText)
	{
		CenterMessageText->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnCenterMessageHidden();
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
		Cue->OnJudgementResult(Result);
	}
	OnParrySuccessEffect(Result, BossHPPercent);
}

void UPTBBBHUDWidget::HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent)
{
	if (UPTBBBCueWidgetBase* Cue = FindAndRemoveCue(Result.NoteId))
	{
		Cue->OnJudgement(EPTBJudgementType::Miss);
		Cue->OnJudgementResult(Result);
	}
	OnParryFailEffect(Result, PlayerHPPercent);
}

void UPTBBBHUDWidget::HandleBBBossHPChanged(float NewHP, float MaxHP)
{
	const float NewPercent = (MaxHP > 0.f) ? FMath::Clamp(NewHP / MaxHP, 0.f, 1.f) : 0.f;

	if (BossHPBar)
	{
		BossHPBar->SetPercent(NewPercent);
	}

	if (NewPercent < BossTargetPercent)
	{
		// HP 감소: 고스트 바를 현 위치에 유지하고 딜레이 타이머 리셋
		BossGhostDecayTimer = GhostBarDecayDelay;
	}
	else if (NewPercent > BossTargetPercent)
	{
		// HP 증가: 고스트 바를 새 값으로 즉시 동기화 (메인 바보다 낮아지면 안됨)
		BossGhostPercent = NewPercent;
		if (BossHPGhostBar)
		{
			BossHPGhostBar->SetPercent(BossGhostPercent);
		}
	}
	// HP 동일(재방송): 고스트 바 상태 유지 (감소 중이면 계속 감소)
	BossTargetPercent = NewPercent;

	OnBossHPUpdated(NewHP, MaxHP);
}

void UPTBBBHUDWidget::HandleBBPlayerHPChanged(float NewHP, float MaxHP)
{
	const float NewPercent = (MaxHP > 0.f) ? FMath::Clamp(NewHP / MaxHP, 0.f, 1.f) : 0.f;

	if (PlayerHPBar)
	{
		PlayerHPBar->SetPercent(NewPercent);
	}

	if (NewPercent < PlayerTargetPercent)
	{
		// HP 감소: 고스트 바를 현 위치에 유지하고 딜레이 타이머 리셋
		PlayerGhostDecayTimer = GhostBarDecayDelay;
	}
	else if (NewPercent > PlayerTargetPercent)
	{
		// HP 증가: 고스트 바를 새 값으로 즉시 동기화 (메인 바보다 낮아지면 안됨)
		PlayerGhostPercent = NewPercent;
		if (PlayerHPGhostBar)
		{
			PlayerHPGhostBar->SetPercent(PlayerGhostPercent);
		}
	}
	// HP 동일(재방송): 고스트 바 상태 유지 (감소 중이면 계속 감소)
	PlayerTargetPercent = NewPercent;

	OnPlayerHPUpdated(NewHP, MaxHP);
}

void UPTBBBHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 보스 고스트 바 감소
	if (bCenterMessageHoldActive)
	{
		CenterMessageHoldRemainingSeconds = FMath::Max(0.0f, CenterMessageHoldRemainingSeconds - InDeltaTime);
		if (CenterMessageText)
		{
			ApplyCenterMessageText(CenterMessageHoldText);
		}

		if (CenterMessageHoldRemainingSeconds <= 0.0f)
		{
			bCenterMessageHoldActive = false;
		}
	}

	if (BossHPGhostBar && BossGhostPercent > BossTargetPercent)
	{
		if (BossGhostDecayTimer > 0.f)
		{
			BossGhostDecayTimer = FMath::Max(0.f, BossGhostDecayTimer - InDeltaTime);
		}
		else
		{
			BossGhostPercent = FMath::Max(BossTargetPercent, BossGhostPercent - GhostBarDecaySpeed * InDeltaTime);
			BossHPGhostBar->SetPercent(BossGhostPercent);
		}
	}

	// 플레이어 고스트 바 감소
	if (PlayerHPGhostBar && PlayerGhostPercent > PlayerTargetPercent)
	{
		if (PlayerGhostDecayTimer > 0.f)
		{
			PlayerGhostDecayTimer = FMath::Max(0.f, PlayerGhostDecayTimer - InDeltaTime);
		}
		else
		{
			PlayerGhostPercent = FMath::Max(PlayerTargetPercent, PlayerGhostPercent - GhostBarDecaySpeed * InDeltaTime);
			PlayerHPGhostBar->SetPercent(PlayerGhostPercent);
		}
	}
}

void UPTBBBHUDWidget::HandleBBBossDefeated()
{
	OnBossDefeatedEffect();
}

void UPTBBBHUDWidget::HandleBBIntroStarted()
{
	ClearCenterMessageTimers();
	ShowCenterMessageForDuration(FText::FromString(TEXT("3")), 0.95f);
	QueueCenterMessage(1.0f, FText::FromString(TEXT("2")));
	QueueCenterMessage(2.0f, FText::FromString(TEXT("1")));
	QueueCenterMessage(2.75f, FText::FromString(TEXT("Start!")));
}

void UPTBBBHUDWidget::HandleBBGameplayStarted()
{
	ClearCenterMessageTimers();
	HideCenterMessage();
}

void UPTBBBHUDWidget::HandleBBOutroStarted(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
	ClearCenterMessageTimers();
	ShowCenterMessage(FText::FromString(TEXT("Finish!")));
}

void UPTBBBHUDWidget::HandleBBOutroFinished(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
	ClearCenterMessageTimers();
}

// ── 내부 헬퍼 ────────────────────────────────────────────────────

void UPTBBBHUDWidget::SyncBarsToMiniGame()
{
	const float BossPercent = BBMiniGame->GetBossHPPercent();
	const float PlayerPercent = BBMiniGame->GetPlayerHPPercent();

	if (BossHPBar) { BossHPBar->SetPercent(BossPercent); }
	if (BossHPGhostBar) { BossHPGhostBar->SetPercent(BossPercent); }
	BossGhostPercent = BossPercent;
	BossTargetPercent = BossPercent;
	BossGhostDecayTimer = 0.f;

	if (PlayerHPBar) { PlayerHPBar->SetPercent(PlayerPercent); }
	if (PlayerHPGhostBar) { PlayerHPGhostBar->SetPercent(PlayerPercent); }
	PlayerGhostPercent = PlayerPercent;
	PlayerTargetPercent = PlayerPercent;
	PlayerGhostDecayTimer = 0.f;
}

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

void UPTBBBHUDWidget::ClearCenterMessageTimers()
{
	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& TimerHandle : CenterMessageTimerHandles)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}

	CenterMessageTimerHandles.Reset();
}

void UPTBBBHUDWidget::ShowCenterMessageForDuration(const FText& Message, float DurationSeconds)
{
	ShowCenterMessage(Message);
	bCenterMessageHoldActive = DurationSeconds > 0.0f;
	CenterMessageHoldRemainingSeconds = FMath::Max(0.0f, DurationSeconds);
	CenterMessageHoldText = Message;
}

void UPTBBBHUDWidget::QueueCenterMessage(float DelaySeconds, const FText& Message)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindWeakLambda(this, [this, Message]()
	{
		ShowCenterMessageForDuration(Message, 0.95f);
	});
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, FMath::Max(0.0f, DelaySeconds), false);
	CenterMessageTimerHandles.Add(TimerHandle);
}
