// Fill out your copyright notice in the Description page of Project Settings.
#include "PTBFSMiniGame.h"
#include "Camera/CameraShakeBase.h"
#include "FishActor.h"
#include "FSWidget.h"
#include "PTBFSCharacter.h"
#include "PTBFSMiniGameRuleSet.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "LevelSequencePlayer.h"
#include "MiniGames/Common/UI/PTBMiniGameLoadingWidget.h"

APTBFSMiniGame::APTBFSMiniGame()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APTBFSMiniGame::BeginPlay()
{
	Super::BeginPlay();
	if (RhythmConductor)
	{
		RhythmConductor->OnAllNotesPassed.AddDynamic(
			this, &APTBFSMiniGame::OnAllNotesPassedFishing
		);
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] RhythmConductor 바인딩 완료"));
	}
	if (!WidgetClass)
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("FS 위젯클래스 비엇따"));
	}
	if (WidgetClass)
	{
		// Retry로 이 미니게임 Actor가 교체될 때, 이전 라운드의 HUD가 뷰포트에 남지 않도록 정리.
		RemoveExistingWidgetsOfClass(WidgetClass);

		ActiveFSWidget = CreateWidget<UFSWidget>(GetWorld(), WidgetClass);

		if (ActiveFSWidget)
		{
			ActiveFSWidget->AddToViewport();

			OnFishingPromptCue.AddDynamic(ActiveFSWidget, &UFSWidget::OnNoteEvent);
			ActiveFSWidget->InitializeWidget(this);

			UE_LOG(LogTemp, Warning, TEXT("[FS] 멤버 변수에 저장 및 바인딩 완료!"));
		}
	}
}

void APTBFSMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	TArray<AActor*> FishActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFishActor::StaticClass(), FishActors);
	if (FishActors.Num() > 0)
		FishActor = Cast<AFishActor>(FishActors[0]);

	FishRuleSet = Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	RebuildKeyShuffleSchedule();
	Character = Cast<APTBFSCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (!Character)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("캐릭터 유효하지 않음"));
		return;
	}
	if (!FishActor)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("물고기 유효하지 않음"));
		return;
	}
	if (!FishRuleSet)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("룰셋데이터에셋이 유효하지 않음"));
		return;
	}
	if (!ChartAsset)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("차트에셋 유효하지 않음"));
		return;
	}

	if (!GI)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("게임인스턴스 유효하지 않음"));
		return;
	}
	FishStartLocation = FishActor->GetActorLocation();
	CharacterLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 75.f;
	int32 TotalNoteCount = ChartAsset->NoteEvents.Num();
	if (TotalNoteCount <= 0)return;

	StepDistance = 1.0f / TotalNoteCount;

	if (JudgementSystem)
	{
		JudgementSystem->HitWindowMissMs = 500.0f;
		JudgementSystem->HitWindowGoodMs = 350.0f;
		JudgementSystem->HitWindowPerfectMs = 200.0f;
		JudgementSystem->HitWindowHighPerfectMs = 80.0f;
	}

	Character->SetFishLineTarget(FishActor);
	Character->OnPlayCastAnimMontage();
}

void APTBFSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);
	if (!FishRuleSet)return;
	float ActualLeadTime = 0.0f;
	if (RhythmConductor)
	{
		ActualLeadTime = RhythmConductor->CueLeadTimeMs;
	}
	OnFishingPromptCue.Broadcast(Note.ActionType, Note.bIsLongNote, Note.TimeMs, ActualLeadTime);
	MovingCount++;
}


void APTBFSMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
	if (!FishActor) return;

	OnFishingPromptReleased.Broadcast(Note.ActionType);
	OnFishingPromptShown.Broadcast(Note.ActionType, Note.bIsLongNote);
	FishLateralOffset = FVector::ZeroVector;
	FVector BaseLoc = FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance);
	FishActor->SetTargetLocation(BaseLoc);


	FVector Offset = FVector::ZeroVector;
	switch (Note.ActionType)
	{
	case EPTBActionType::ActionA:
		Offset = FVector(0, -200, 0);
		break;
	// 키 배치(A 왼쪽 / B 가운데 / C 오른쪽)와 화면 위치를 맞춤: B = 위(가운데), C = 오른쪽
	case EPTBActionType::ActionB:
		Offset = FVector(0, 0, 150);
		break;
	case EPTBActionType::ActionC:
		Offset = FVector(0, 200, 0);
		break;
	default:
		break;
	}

	FishLateralOffset = Offset;
	FishActor->SetTargetLocation(BaseLoc + FishLateralOffset);
}

void APTBFSMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
}

void APTBFSMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
	if (!Character)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("캐릭터 유효하지 않음"));
		return;
	}
	if (!FishActor)return;
	if (Result.Reason == EPTBJudgementReason::EmptyInput) return;
	if (Result.Reason == EPTBJudgementReason::EarlyRelease)
	{
		OnFishSlipped.Broadcast(1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		return;
	}
	switch (Result.JudgementType)
	{
	case EPTBJudgementType::HighPerfect:
		FishLateralOffset = FVector::ZeroVector;
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		OnFishingJudgement.Broadcast(Result.JudgementType);
		CurrentComboCount++;
		CorrectCount++;
		OnFishingComboChanged.Broadcast(CurrentComboCount);
		break;
	case EPTBJudgementType::Perfect:
		FishLateralOffset = FVector::ZeroVector;
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		OnFishingJudgement.Broadcast(Result.JudgementType);
		CurrentComboCount++;
		CorrectCount++;
		OnFishingComboChanged.Broadcast(CurrentComboCount);
		break;
	case EPTBJudgementType::Good:
		FishLateralOffset = FVector::ZeroVector;
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		OnFishingJudgement.Broadcast(Result.JudgementType);
		CurrentComboCount++;
		CorrectCount++;
		OnFishingComboChanged.Broadcast(CurrentComboCount);
		break;
	case EPTBJudgementType::Miss:
		PTB_WARNING(LogPTBMiniGames, TEXT("[FS] Miss Reason: %d"), static_cast<int32>(Result.Reason));
		FishLateralOffset = FVector::ZeroVector;
		FishActor->SetTargetLocation(FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance));
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		if (Result.Reason == EPTBJudgementReason::ExpiredNote)
		{
			ApplyDistanceDelta(StepDistance * 1.0f);
			OnFishSlipped.Broadcast(1.0f);
		}
		CurrentComboCount = 0;
		OnFishingComboChanged.Broadcast(CurrentComboCount);
		OnFishingJudgement.Broadcast(Result.JudgementType);
		if (MissCameraShakeClass)
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->ClientStartCameraShake(MissCameraShakeClass);
			}
		}
		break;
	default:
		break;
	}
}

FPTBMiniGameResultPayload APTBFSMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();
	Payload.PayloadType = TEXT("FS");

	Payload.IntValues.Add(TEXT("CorrectCount"), CorrectCount);
	Payload.FloatValues.Add(TEXT("FinalFishDistance"), FishDistance);
	Payload.IntValues.Add(TEXT("bCaught"), FishDistance <= 0.5f ? 1 : 0);
	return Payload;
}

void APTBFSMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	Super::InitializeMiniGame(Context);
}


void APTBFSMiniGame::OnAllNotesPassedFishing()
{
	OnFishRevealed.Broadcast(FishActor);
	if (FishDistance <= 0.5f)
	{
		if (ActiveFSWidget)
		{
			ActiveFSWidget->RemoveFromParent();
			ActiveFSWidget = nullptr;
		}
		if (SuccessCameraSequence)
		{
			ALevelSequenceActor* SequenceActor;
			ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(
				GetWorld(), SuccessCameraSequence, FMovieSceneSequencePlaybackSettings(), SequenceActor);
			if (Player)
			{
				Player->Play();
				PTB_WARNING(LogPTBMiniGames, TEXT("[FS] 레벨 시퀀스 재생 시작"));
			}
		}
		Character->StopAnimMontage();
		FTimerHandle FirstHandle;
		GetWorldTimerManager().SetTimer(FirstHandle, [WeakThis = TWeakObjectPtr<APTBFSMiniGame>(this)]()
		{
			if (!WeakThis.IsValid() || !WeakThis->Character || !WeakThis->SuccessAnim) return;
			WeakThis->Character->PlayAnimMontage(WeakThis->SuccessAnim);
			float MontageLength = WeakThis->SuccessAnim->GetPlayLength();
			FTimerHandle SecondHandle;
			WeakThis->GetWorldTimerManager().SetTimer(SecondHandle, [WeakThis]()
			{
				if (!WeakThis.IsValid() || !WeakThis->Character || !WeakThis->SuccessAnim) return;
				WeakThis->Character->PlayAnimMontage(WeakThis->SuccessAnim);
			}, MontageLength, false);
		}, 0.2f, false);
		if (FishActor)
			FishActor->SetActorHiddenInGame(true);
	}
	else
	{
		if (ActiveFSWidget)
		{
			ActiveFSWidget->RemoveFromParent();
			ActiveFSWidget = nullptr;
		}
		if (FailCameraSequence)
		{
			ALevelSequenceActor* SequenceActor;
			ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(
				GetWorld(), FailCameraSequence, FMovieSceneSequencePlaybackSettings(), SequenceActor);
			if (Player)
			{
				Player->Play();
				PTB_WARNING(LogPTBMiniGames, TEXT("[FS] 레벨 시퀀스 재생 시작"));
			}
		}
		Character->StopAnimMontage();
		FTimerHandle FirstHandle;
		GetWorldTimerManager().SetTimer(FirstHandle, [WeakThis = TWeakObjectPtr<APTBFSMiniGame>(this)]()
		{
			if (!WeakThis.IsValid() || !WeakThis->Character || !WeakThis->FailAnim) return;
			WeakThis->Character->PlayAnimMontage(WeakThis->FailAnim);
			float MontageLength = WeakThis->FailAnim->GetPlayLength();
			FTimerHandle SecondHandle;
			WeakThis->GetWorldTimerManager().SetTimer(SecondHandle, [WeakThis]()
			{
				if (!WeakThis.IsValid() || !WeakThis->Character || !WeakThis->FailAnim) return;
				WeakThis->Character->PlayAnimMontage(WeakThis->FailAnim);
			}, MontageLength, false);
		}, 0.2f, false);
		if (FishActor)
			FishActor->SetActorHiddenInGame(true);
		PTB_WARNING(LogPTBMiniGames, TEXT("실패!"));
	}
}

void APTBFSMiniGame::ApplyDistanceDelta(float Delta)
{
	float FishMinDistance = 0;
	float FishMaxDistance = 1;
	FishDistance = FMath::Clamp(FishDistance + Delta, FishMinDistance, FishMaxDistance);
	OnFishDistanceChanged.Broadcast(FishDistance);

	EFishingLineState NewState = CalculateLineState(FishDistance);
	if (CurrentLineState != NewState)
	{
		CurrentLineState = NewState;
		OnFishingLineStateChanged.Broadcast(CurrentLineState);
	}

	if (FishActor)
	{
		FVector TargetLoc = FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance);
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] FishActor TargetLoc: %s FishDistance: %f"), *TargetLoc.ToString(),
		            FishDistance);
		FishActor->SetTargetLocation(
			FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance) + FishLateralOffset);
	}
}

EFishingLineState APTBFSMiniGame::CalculateLineState(float Distance) const
{
	if (Distance <= 0.4f)
	{
		return EFishingLineState::Maximum;
	}
	else if (Distance <= 0.8f)
	{
		return EFishingLineState::Taut;
	}
	else
	{
		return EFishingLineState::Loose;
	}
}

void APTBFSMiniGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// (예전 키 교체 실험 코드 제거) OriginalKeyBindings가 한 번도 저장되지 않은 채로 덮어써서
	// FS를 나갈 때마다 플레이어가 설정한 키가 기본값(Z/X/C...)으로 돌아가던 문제가 있었음
	Super::EndPlay(EndPlayReason);
}

void APTBFSMiniGame::HandleRoundChartChanged()
{
	Super::HandleRoundChartChanged();
	RebuildKeyShuffleSchedule();
}

// ───────────────────────── 키 셔플 ─────────────────────────

namespace FSKeyShuffle
{
	static int32 ActionToIndex(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return 0;
		case EPTBActionType::ActionB: return 1;
		case EPTBActionType::ActionC: return 2;
		default: return INDEX_NONE;
		}
	}

	static EPTBActionType IndexToAction(int32 Index)
	{
		switch (Index)
		{
		case 0: return EPTBActionType::ActionA;
		case 1: return EPTBActionType::ActionB;
		case 2: return EPTBActionType::ActionC;
		default: return EPTBActionType::None;
		}
	}

	/** 3키 배치 6가지. 0번 = 기본 배치 */
	static const int32 Permutations[6][3] = {
		{ 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 }
	};

	/** "CAB" → {2,0,1}. 잘못된 문자열이면 false */
	static bool ParsePattern(const FString& Pattern, int32 OutKeys[3])
	{
		const FString Upper = Pattern.TrimStartAndEnd().ToUpper();
		if (Upper.Len() != 3)
		{
			return false;
		}
		bool bUsed[3] = { false, false, false };
		for (int32 i = 0; i < 3; ++i)
		{
			const int32 Key = Upper[i] - TEXT('A');
			if (Key < 0 || Key > 2 || bUsed[Key])
			{
				return false;
			}
			bUsed[Key] = true;
			OutKeys[i] = Key;
		}
		return true;
	}
}

void APTBFSMiniGame::RebuildKeyShuffleSchedule()
{
	KeySegments.Reset();
	KeySegments.AddDefaulted(1); // 0번 구간 = 기본 배치
	bKeyShuffleActive = false;
	for (int32& Lane : HeldLaneByKey)
	{
		Lane = INDEX_NONE;
	}

	const UPTBFSMiniGameRuleSet* Rule = FishRuleSet ? FishRuleSet.Get() : Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	const UPTBRhythmChartAsset* Chart = ChartAsset.Get();
	if (!Rule || !Chart || Chart->NoteEvents.IsEmpty())
	{
		return;
	}

	const bool bPracticeChart = TutorialPracticeChart && Chart == TutorialPracticeChart.Get();
	if (bPracticeChart && !Rule->bKeyShuffleInPractice)
	{
		return;
	}
	if (!Rule->KeyShuffleDifficulties.Contains(GameContext.SessionRequest.Difficulty))
	{
		return;
	}

	// 마디 길이(4분음표 박 기준) × 몇 마디마다
	const FPTBChartData& Data = Chart->ChartData;
	const float BeatsPerMeasure = Data.TimeSignatureDenominator > 0
		? static_cast<float>(Data.TimeSignatureNumerator) * 4.0f / static_cast<float>(Data.TimeSignatureDenominator)
		: 4.0f;
	KeySegmentBeats = FMath::Max(0.25f, BeatsPerMeasure * FMath::Max(1, Rule->KeyShuffleMeasures));

	// 구간별 첫 노트 / 마지막 노트(롱노트는 떼는 시점) 시간
	int32 MaxSegment = 0;
	for (const FPTBNoteEvent& Note : Chart->NoteEvents)
	{
		MaxSegment = FMath::Max(MaxSegment, FMath::FloorToInt(FMath::Max(0.0f, Note.BeatTime) / KeySegmentBeats));
	}
	MaxSegment = FMath::Min(MaxSegment, 4096);

	TArray<float> FirstMs;
	TArray<float> LastMs;
	FirstMs.Init(TNumericLimits<float>::Max(), MaxSegment + 1);
	LastMs.Init(TNumericLimits<float>::Lowest(), MaxSegment + 1);
	for (const FPTBNoteEvent& Note : Chart->NoteEvents)
	{
		const int32 Seg = FMath::Clamp(FMath::FloorToInt(FMath::Max(0.0f, Note.BeatTime) / KeySegmentBeats), 0, MaxSegment);
		const float EndMs = Note.bIsLongNote ? FMath::Max(Note.TimeMs, Note.ReleaseTimeMs) : Note.TimeMs;
		FirstMs[Seg] = FMath::Min(FirstMs[Seg], Note.TimeMs);
		LastMs[Seg] = FMath::Max(LastMs[Seg], EndMs);
	}

	// 배치 순서: 직접 지정 패턴 > 고정 시드. 노트 없는 구간은 배치 유지 (빈 마디가 있어도 순서가 밀리지 않도록 구간 번호 기준)
	FRandomStream Stream(Rule->KeyShuffleSeed);
	int32 PrevPerm = 0;
	if (Rule->KeyShufflePattern.Num() > 0)
	{
		FSKeyShuffle::ParsePattern(Rule->KeyShufflePattern[0], KeySegments[0].KeyForLane);
	}

	float PrevLastMs = LastMs[0];
	for (int32 Seg = 1; Seg <= MaxSegment; ++Seg)
	{
		FFSKeySegment& NewSegment = KeySegments.AddDefaulted_GetRef();
		const FFSKeySegment& PrevSegment = KeySegments[Seg - 1];

		// 시드 기반 다음 배치는 노트 유무와 상관없이 구간마다 뽑아서 순서 고정
		int32 RandomPerm = Stream.RandRange(0, 4);
		if (RandomPerm >= PrevPerm)
		{
			++RandomPerm; // 직전과 같은 배치 제외
		}

		const bool bHasNotes = FirstMs[Seg] < TNumericLimits<float>::Max();
		if (!bHasNotes)
		{
			FMemory::Memcpy(NewSegment.KeyForLane, PrevSegment.KeyForLane, sizeof(NewSegment.KeyForLane));
			continue;
		}

		bool bParsed = false;
		if (Rule->KeyShufflePattern.Num() > 0)
		{
			const FString& Pattern = Rule->KeyShufflePattern[Seg % Rule->KeyShufflePattern.Num()];
			bParsed = FSKeyShuffle::ParsePattern(Pattern, NewSegment.KeyForLane);
			if (!bParsed)
			{
				UE_LOG(LogPTBMiniGames, Warning, TEXT("[FS] KeyShufflePattern[%d] '%s' 형식 오류 (예: ABC, CAB)"),
					Seg % Rule->KeyShufflePattern.Num(), *Pattern);
			}
		}
		if (!bParsed)
		{
			PrevPerm = RandomPerm;
			FMemory::Memcpy(NewSegment.KeyForLane, FSKeyShuffle::Permutations[RandomPerm], sizeof(NewSegment.KeyForLane));
		}

		const bool bChanged = FMemory::Memcmp(NewSegment.KeyForLane, PrevSegment.KeyForLane, sizeof(NewSegment.KeyForLane)) != 0;
		if (bChanged)
		{
			// 이전 구간 마지막 노트 ~ 이 구간 첫 노트 사이에서 교체 연출 (노트와 겹치지 않게, 빨리 끝나도록 앞쪽 배치)
			const float Gap = FMath::Max(0.0f, FirstMs[Seg] - PrevLastMs);
			const float Duration = FMath::Min(Rule->KeyShuffleAnimMaxMs, Gap * 0.5f);
			const float Delay = FMath::Min(Rule->KeyShuffleAnimDelayMs, (Gap - Duration) * 0.5f);
			NewSegment.bHasSwap = true;
			NewSegment.SwapStartMs = (PrevLastMs > TNumericLimits<float>::Lowest() ? PrevLastMs : FirstMs[Seg] - Duration) + Delay;
			NewSegment.SwapEndMs = NewSegment.SwapStartMs + Duration;
		}
		PrevLastMs = LastMs[Seg];
	}

	bKeyShuffleActive = true;
	UE_LOG(LogPTBMiniGames, Log, TEXT("[FS] KeyShuffle ON. Segments=%d SegmentBeats=%.2f Seed=%d Pattern=%d"),
		KeySegments.Num(), KeySegmentBeats, Rule->KeyShuffleSeed, Rule->KeyShufflePattern.Num());
}

int32 APTBFSMiniGame::GetKeySegmentIndex(float BeatTime) const
{
	if (!bKeyShuffleActive || KeySegments.Num() <= 1)
	{
		return 0;
	}
	return FMath::Clamp(FMath::FloorToInt(FMath::Max(0.0f, BeatTime) / KeySegmentBeats), 0, KeySegments.Num() - 1);
}

int32 APTBFSMiniGame::GetLaneOfKey(int32 SegmentIndex, int32 KeyIndex) const
{
	if (!KeySegments.IsValidIndex(SegmentIndex))
	{
		return KeyIndex;
	}
	for (int32 Lane = 0; Lane < 3; ++Lane)
	{
		if (KeySegments[SegmentIndex].KeyForLane[Lane] == KeyIndex)
		{
			return Lane;
		}
	}
	return KeyIndex;
}

void APTBFSMiniGame::GetKeyGuideMotion(int32 KeyIndex, float VisualTimeMs, int32& OutFromLane, int32& OutToLane, float& OutAlpha) const
{
	OutFromLane = OutToLane = FMath::Clamp(KeyIndex, 0, 2);
	OutAlpha = 1.0f;
	if (!bKeyShuffleActive || KeySegments.Num() <= 1)
	{
		return;
	}

	// 시작 시간이 지난 마지막 교체 구간 찾기 (구간 수가 적어서 선형 탐색)
	int32 Current = 0;
	for (int32 Seg = 1; Seg < KeySegments.Num(); ++Seg)
	{
		if (KeySegments[Seg].bHasSwap && VisualTimeMs >= KeySegments[Seg].SwapStartMs)
		{
			Current = Seg;
		}
	}

	OutToLane = GetLaneOfKey(Current, KeyIndex);
	OutFromLane = OutToLane;
	if (Current > 0 && VisualTimeMs < KeySegments[Current].SwapEndMs)
	{
		const FFSKeySegment& Segment = KeySegments[Current];
		OutFromLane = GetLaneOfKey(Current - 1, KeyIndex);
		const float Duration = FMath::Max(1.0f, Segment.SwapEndMs - Segment.SwapStartMs);
		OutAlpha = FMath::Clamp((VisualTimeMs - Segment.SwapStartMs) / Duration, 0.0f, 1.0f);
	}
}

void APTBFSMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
	const int32 KeyIndex = FSKeyShuffle::ActionToIndex(Action);
	if (!bKeyShuffleActive || KeyIndex == INDEX_NONE || !JudgementSystem || !CanAcceptInput())
	{
		Super::HandleRhythmInput(Action, TimeMs);
		return;
	}

	// 누른 키로 맞출 수 있는 노트 = 각 칸의 대기 노트 중 "그 노트 구간에서 이 키가 그 칸"인 것
	const float JudgeTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();
	int32 BestLane = INDEX_NONE;
	float BestDelta = TNumericLimits<float>::Max();
	bool bAnyPending = false;
	for (int32 Lane = 0; Lane < 3; ++Lane)
	{
		FPTBNoteEvent Note;
		if (!JudgementSystem->FindBestPendingNote(FSKeyShuffle::IndexToAction(Lane), JudgeTimeMs, Note))
		{
			continue;
		}
		bAnyPending = true;
		const int32 Segment = GetKeySegmentIndex(Note.BeatTime);
		if (KeySegments[Segment].KeyForLane[Lane] != KeyIndex)
		{
			continue;
		}
		const float Delta = FMath::Abs(JudgeTimeMs - Note.TimeMs);
		if (Delta < BestDelta)
		{
			BestDelta = Delta;
			BestLane = Lane;
		}
	}

	if (BestLane != INDEX_NONE)
	{
		HeldLaneByKey[KeyIndex] = BestLane;
		Super::HandleRhythmInput(FSKeyShuffle::IndexToAction(BestLane), TimeMs);
		return;
	}

	// 틀린 키: 다른 칸 노트를 대신 맞추지 않도록 무시 (그 노트는 지나가면 Miss)
	if (bAnyPending)
	{
		return;
	}

	// 대기 노트가 없으면 일반 빈 입력 처리
	Super::HandleRhythmInput(Action, TimeMs);
}

void APTBFSMiniGame::HandleRhythmInputReleased(EPTBActionType Action, float TimeMs)
{
	const int32 KeyIndex = FSKeyShuffle::ActionToIndex(Action);
	if (!bKeyShuffleActive || KeyIndex == INDEX_NONE)
	{
		Super::HandleRhythmInputReleased(Action, TimeMs);
		return;
	}

	const int32 Lane = HeldLaneByKey[KeyIndex];
	HeldLaneByKey[KeyIndex] = INDEX_NONE;
	Super::HandleRhythmInputReleased(Lane != INDEX_NONE ? FSKeyShuffle::IndexToAction(Lane) : Action, TimeMs);
}

void APTBFSMiniGame::HandleActionAInput()
{
	HandleRhythmInput(EPTBActionType::ActionA);
}

void APTBFSMiniGame::HandleActionBInput()
{
	HandleRhythmInput(EPTBActionType::ActionB);
}

void APTBFSMiniGame::HandleActionCInput()
{
	HandleRhythmInput(EPTBActionType::ActionC);
}

void APTBFSMiniGame::HandleActionAInputReleased()
{
	HandleRhythmInputReleased(EPTBActionType::ActionA);
}

void APTBFSMiniGame::HandleActionBInputReleased()
{
	HandleRhythmInputReleased(EPTBActionType::ActionB);
}

void APTBFSMiniGame::HandleActionCInputReleased()
{
	HandleRhythmInputReleased(EPTBActionType::ActionC);
}

