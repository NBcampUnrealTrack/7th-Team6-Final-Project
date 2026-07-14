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
	case EPTBActionType::ActionB:
		Offset = FVector(0, 200, 0);
		break;
	case EPTBActionType::ActionC:
		Offset = FVector(0, 0, 150);
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
	if (UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance()))
	{
		GI->CachedSettings.RhythmKeys = OriginalKeyBindings;
	}
	Super::EndPlay(EndPlayReason);
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

