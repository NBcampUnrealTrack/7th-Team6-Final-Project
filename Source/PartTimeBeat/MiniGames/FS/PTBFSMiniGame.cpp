// Fill out your copyright notice in the Description page of Project Settings.
#include "PTBFSMiniGame.h"

#include "EngineUtils.h"
#include "FishActor.h"
#include "FSWidget.h"
#include "PTBFSCharacter.h"
#include "PTBFSMiniGameRuleSet.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Camera/CameraActor.h"

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
		PTB_ERROR(LogPTBMiniGames,TEXT("FS 위젯클래스 비엇따"));
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
	if (bMovingCamera1 && SuccessCamera1)
	{
		CameraElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(CameraElapsedTime / 3.0f, 0.0f, 1.0f);
		FVector NewLoc = FMath::Lerp(Camera1StartLocation, Camera1EndLocation, Alpha);
		SuccessCamera1->SetActorLocation(NewLoc);

		if (Alpha >= 1.0f)
		{
			bMovingCamera1 = false;
		}
	}

	if (bMovingCamera2 && SuccessCamera2)
	{
		CameraElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(CameraElapsedTime / 3.0f, 0.0f, 1.0f);
		FVector NewLoc = FMath::Lerp(Camera2StartLocation, Camera2EndLocation, Alpha);
		SuccessCamera2->SetActorLocation(NewLoc);

		if (Alpha >= 1.0f)
		{
			bMovingCamera2 = false;
		}
	}

}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
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
	CharacterLocation = Character->GetActorLocation();
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

	PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] StepDistance: %f,TotalNoteCount: %d"),
	            StepDistance, TotalNoteCount);

	Character->SetFishLineTarget(FishActor);
	Character->OnPlayCastAnimMontage();
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] Camera found: %s"), *It->GetName());
		if (It->ActorHasTag(FName("SuccessCamera1")))
			SuccessCamera1 = *It;
		else if ((It->ActorHasTag(FName("SuccessCamera2"))))
			SuccessCamera2 = *It;
	}	
	GI->CachedSettings.RhythmKeys.ActionA = EKeys::Left;
	GI->CachedSettings.RhythmKeys.ActionB = EKeys::Right;
	GI->CachedSettings.RhythmKeys.ActionC = EKeys::Up;
}

void APTBFSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] HandleNoteCue 호출됨 MovingCount: %d FishDistance: %f"), MovingCount,
	            FishDistance);
	if (!FishRuleSet)return;	
	float ActualLeadTime = 0.0f;
	if (RhythmConductor)
	{
		 ActualLeadTime = RhythmConductor->CueLeadTimeMs; // 또는 프로젝트에서 정의한 리드타임 변수명
	}
	OnFishingPromptCue.Broadcast(Note.ActionType, Note.bIsLongNote,Note.TimeMs,ActualLeadTime);
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
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] EarlyRelease NoteId: %d ActionType: %d"), Result.NoteId,
		            static_cast<int32>(Result.ActionType));
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
		FishLateralOffset = FVector::ZeroVector;
		FishActor->SetTargetLocation(FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance));
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		if (Result.Reason == EPTBJudgementReason::ExpiredNote)
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] Miss NoteId: %d ActionType: %d Reason: %d DeltaMs: %f"),
			            Result.NoteId,
			            static_cast<int32>(Result.ActionType),
			            static_cast<int32>(Result.Reason),
			            Result.DeltaMs);
			PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] OnFishSlipped 브로드캐스트"));
			ApplyDistanceDelta(StepDistance * 1.0f);
			OnFishSlipped.Broadcast(1.0f);
		}
		CurrentComboCount =0;
		OnFishingComboChanged.Broadcast(CurrentComboCount);	
		OnFishingJudgement.Broadcast(Result.JudgementType);
		break;
	}
}

FPTBMiniGameResultPayload APTBFSMiniGame::BuildResultPayload() const
{
	return Super::BuildResultPayload();
}

void APTBFSMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	Super::InitializeMiniGame(Context);
}

void APTBFSMiniGame::PlaySuccessCameraSequence()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (SuccessCamera1)
	{
		Camera1StartLocation = SuccessCamera1->GetActorLocation();
		Camera1EndLocation = Camera1StartLocation + FVector(0, 0, 100);
		CameraElapsedTime = 0.0f;
		bMovingCamera1 = true;
		PC->SetViewTargetWithBlend(SuccessCamera1, 0.5f);
	}

	GetWorldTimerManager().SetTimer(
		CameraTimer,
		[WeakThis = TWeakObjectPtr<APTBFSMiniGame>(this)]()
		{
			if (!WeakThis.IsValid()) return;
        
			APlayerController* PC = WeakThis->GetWorld()->GetFirstPlayerController();
			if (!PC) return;

			if (WeakThis->SuccessCamera2)
			{
				WeakThis->Camera2StartLocation = WeakThis->SuccessCamera2->GetActorLocation();
				WeakThis->Camera2EndLocation = WeakThis->Camera2StartLocation + FVector(0, 0, -100);
				WeakThis->CameraElapsedTime = 0.0f;
				WeakThis->bMovingCamera2 = true;
				PC->SetViewTargetWithBlend(WeakThis->SuccessCamera2, 0.5f);
			}
		},
		5.0f,
		false
	);
}

void APTBFSMiniGame::OnAllNotesPassedFishing()
{
	OnFishRevealed.Broadcast(FishActor);
	// 원래는 캐릭터를 가져와서 해야함 Character->PlayAnim SuccessAnim;
	if (FishDistance <= 0.3f) // 성공 조건
	{
		PlaySuccessCameraSequence();
		PTB_WARNING(LogPTBMiniGames, TEXT("성공!"));
	}
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("실패!"));
	}
}

void APTBFSMiniGame::ApplyDistanceDelta(float Delta)
{
	FishDistance = FMath::Clamp(FishDistance + Delta, 0.0f, 1.0f);
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

void APTBFSMiniGame::OnStartFishingGame()
{
	FPTBMiniGameContext Context;
	UPTBGameFlowSubsystem* FlowSys = GetGameInstance()->GetSubsystem<UPTBGameFlowSubsystem>();
    
	EPTBDifficulty Difficulty = EPTBDifficulty::Standard;
	if (FlowSys && !FlowSys->PendingSessionRequest.MiniGameId.IsNone())
	{
		Difficulty = FlowSys->PendingSessionRequest.Difficulty;
	}
    
	PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] Difficulty: %d"), static_cast<int32>(Difficulty));
    
	Context.SessionRequest.Difficulty = Difficulty;
	Context.SessionRequest.MiniGameId = FName("FishMiniGame");
	InitializeMiniGame(Context);
	StartMiniGame();
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

void APTBFSMiniGame::HandleActionDInput()
{
	HandleRhythmInput(EPTBActionType::ActionD);
}

void APTBFSMiniGame::HandleActionEInput()
{
	HandleRhythmInput(EPTBActionType::ActionE);
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

void APTBFSMiniGame::HandleActionDInputReleased()
{
	HandleRhythmInputReleased(EPTBActionType::ActionD);
}

void APTBFSMiniGame::HandleActionEInputReleased()
{
	HandleRhythmInputReleased(EPTBActionType::ActionE);
}