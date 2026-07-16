#include "MiniGames/BB/PTBBBHUDWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MiniGames/BB/PTBBBMiniGame.h"
#include "MiniGames/BB/PTBBBCueWidgetBase.h"
#include "MiniGames/BB/PTBBBHitZoneWidgetBase.h"
#include "Debug/PTBTeamLog.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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
		BBMiniGame->OnBBNoteReached.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBNoteReached);
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
	InMiniGame->OnBBNoteReached.AddUniqueDynamic(this, &UPTBBBHUDWidget::HandleBBNoteReached);
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
	RefreshHitZoneVisibility();

	if (InMiniGame->IsBBIntroSequenceActive())
	{
		HandleBBIntroStarted();
	}
	else if (InMiniGame->IsBBGameplayActive())
	{
		HideCenterMessage();
	}
	else if (InMiniGame->IsBBOutroSequenceActive())
	{
		ShowCenterMessage(FText::FromString(TEXT("Finish!")));
	}
}

void UPTBBBHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Blueprint에 Event Tick 노드가 없어도 NativeTick이 호출되도록 강제 활성화
	bHasScriptImplementedTick = true;

	SpawnHitZoneMarkers();
	RefreshHitZoneVisibility();

	// BindToMiniGame이 AddToViewport 이전에 호출된 경우를 대비한 초기 동기화
	if (!IsValid(BBMiniGame))
	{
		TArray<AActor*> MiniGameActors;
		UGameplayStatics::GetAllActorsOfClass(this, APTBBBMiniGame::StaticClass(), MiniGameActors);
		for (AActor* Actor : MiniGameActors)
		{
			if (APTBBBMiniGame* FoundMiniGame = Cast<APTBBBMiniGame>(Actor))
			{
				BindToMiniGame(FoundMiniGame);
				break;
			}
		}
	}
	else
	{
		SyncBarsToMiniGame();
	}
}

void UPTBBBHUDWidget::NativeDestruct()
{
	// GC 전 타이머 발화 방어. GetWorld()가 null이어도 람다·멤버 타이머 콜백이 실행되지 않도록 먼저 세팅.
	bIsDestructed = true;

	Super::NativeDestruct();

	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBNoteCue);
		BBMiniGame->OnBBNoteReached.RemoveDynamic(this, &UPTBBBHUDWidget::HandleBBNoteReached);
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OutroImageTimerHandle);
	}
}

void UPTBBBHUDWidget::ShowCenterMessage(const FText& Message)
{
	OnCenterMessageShown(Message);
	if (CenterMessageText)
	{
		CenterMessageText->SetText(Message);
		CenterMessageText->SetRenderOpacity(1.0f);
		CenterMessageText->SetRenderScale(FVector2D::UnitVector);
		CenterMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UPTBBBHUDWidget::HideCenterMessage()
{
	if (bIsDestructed) return;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CenterMessageHoldTimerHandle);
	}
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

	// BB 채보에는 롱노트가 없어 항상 TapCueClass를 사용한다.
	UPTBBBCueWidgetBase* Cue = SpawnAndPlaceCue(TapCueClass, Note);
	if (!Cue) return;

	TapCueMap.Add(Note.NoteId, Cue);

	OnCueSpawned(Cue, Note);
}

void UPTBBBHUDWidget::HandleBBNoteReached(FPTBNoteEvent Note)
{
	if (TObjectPtr<UPTBBBHitZoneWidgetBase>* Found = HitZoneMarkers.Find(Note.ActionType))
	{
		if (UPTBBBHitZoneWidgetBase* Marker = Found->Get())
		{
			Marker->PulseHitZone();
		}
	}
}

void UPTBBBHUDWidget::HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent)
{
	if (UPTBBBCueWidgetBase* Cue = FindAndRemoveCue(Result.NoteId))
	{
		Cue->OnJudgementResult(Result);
	}
	OnParrySuccessEffect(Result, BossHPPercent);
}

void UPTBBBHUDWidget::HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent)
{
	if (UPTBBBCueWidgetBase* Cue = FindAndRemoveCue(Result.NoteId))
	{
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
	RefreshHitZoneVisibility();
}

void UPTBBBHUDWidget::HandleBBOutroStarted(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
	ClearCenterMessageTimers();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OutroImageTimerHandle);
	}

	// 1단계: "Finish!" 메시지를 단독으로 표시
	ShowCenterMessage(FText::FromString(TEXT("Finish!")));

	const int32 TierIndex = ResolveOutroTierIndex(Result, EndReason);
	UTexture2D* OutroTexture = OutroTexturesByStar.IsValidIndex(TierIndex) ? OutroTexturesByStar[TierIndex] : nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 2단계: FinishMessageSeconds 후 "Finish!"를 지우고 결과 이미지 표시
	FTimerDelegate ShowResultDelegate;
	ShowResultDelegate.BindWeakLambda(this, [this, OutroTexture]()
	{
		if (bIsDestructed) return;

		HideCenterMessage();
		if (ResultTexture)
		{
			OnShowOutroImage(ResultTexture);
		}

		// 3단계: ResultDisplaySeconds 후 별점에 맞는 아웃트로 이미지로 전환
		if (!OutroTexture)
		{
			return;
		}
		if (UWorld* InnerWorld = GetWorld())
		{
			FTimerDelegate ShowOutroDelegate;
			ShowOutroDelegate.BindWeakLambda(this, [this, OutroTexture]()
			{
				if (bIsDestructed) return;
				OnShowOutroImage(OutroTexture);
			});
			InnerWorld->GetTimerManager().SetTimer(OutroImageTimerHandle, ShowOutroDelegate, ResultDisplaySeconds, false);
		}
	});
	World->GetTimerManager().SetTimer(OutroImageTimerHandle, ShowResultDelegate, FinishMessageSeconds, false);
}

void UPTBBBHUDWidget::HandleBBOutroFinished(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
	ClearCenterMessageTimers();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OutroImageTimerHandle);
	}
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

void UPTBBBHUDWidget::SpawnHitZoneMarkers()
{
	if (!HitZoneClass || !CueLayer || !HitZoneMarkers.IsEmpty())
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	for (const TPair<EPTBActionType, FVector2D>& AnchorPair : AnchorPositions)
	{
		UPTBBBHitZoneWidgetBase* Marker = CreateWidget<UPTBBBHitZoneWidgetBase>(PC, HitZoneClass);
		if (!Marker)
		{
			continue;
		}

		// 노트 큐보다 먼저 CueLayer에 추가되어 항상 뒤쪽(아래)에 그려진다.
		if (UCanvasPanelSlot* MarkerSlot = CueLayer->AddChildToCanvas(Marker))
		{
			MarkerSlot->SetPosition(AnchorPair.Value);
		}

		Marker->InitHitZone(AnchorPair.Key);
		HitZoneMarkers.Add(AnchorPair.Key, Marker);
	}
}

void UPTBBBHUDWidget::RefreshHitZoneVisibility()
{
	// ChartAsset이 아직 준비되지 않았으면(바인딩이 너무 이른 경우) 아무 것도 바꾸지 않는다.
	// 잘못 판단해서 전부 숨겨버리면 다음 갱신 전까지 마커가 깜빡였다 다시 나타나는 것처럼 보인다.
	if (!IsValid(BBMiniGame) || !BBMiniGame->ChartAsset)
	{
		return;
	}

	for (const TPair<EPTBActionType, TObjectPtr<UPTBBBHitZoneWidgetBase>>& MarkerPair : HitZoneMarkers)
	{
		UPTBBBHitZoneWidgetBase* Marker = MarkerPair.Value.Get();
		if (!Marker)
		{
			continue;
		}

		const bool bUsedInChart = BBMiniGame->IsActionUsedInChart(MarkerPair.Key);
		Marker->SetVisibility(bUsedInChart ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
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

	// 보스 X → 앵커 X 이동 오프셋. NativeTick에서 선형 보간.
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
	return nullptr;
}

int32 UPTBBBHUDWidget::ResolveOutroTierIndex(const FPTBRoundResult& Result, EPTBRoundEndReason EndReason) const
{
	// Failed(플레이어 체력 0)는 항상 최하위 등급
	if (EndReason == EPTBRoundEndReason::Failed)
	{
		return 0;
	}

	// 채보를 끝까지 마쳤다면 정확도·미스 수만으로 등급을 나눈다.
	// 보스 체력은 이제 채보 진행도 그 자체(모든 노트를 성공해야만 완전히 0이 됨)라서
	// 등급 판정에는 쓰지 않는다.
	if (Result.MissCount <= PerfectClearMaxMissCount)
	{
		return 3; // Perfect Clear
	}
	return (Result.AccuracyRate >= GreatAccuracyThreshold) ? 2 : 1; // Great : Good
}

void UPTBBBHUDWidget::ClearCenterMessageTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CenterMessageHoldTimerHandle);
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
	if (UWorld* World = GetWorld(); DurationSeconds > 0.0f && World)
	{
		World->GetTimerManager().SetTimer(
			CenterMessageHoldTimerHandle,
			this, &UPTBBBHUDWidget::HideCenterMessage,
			DurationSeconds, false);
	}
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
		if (bIsDestructed) return;
		ShowCenterMessageForDuration(Message, 0.95f);
	});
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, FMath::Max(0.0f, DelaySeconds), false);
	CenterMessageTimerHandles.Add(TimerHandle);
}
