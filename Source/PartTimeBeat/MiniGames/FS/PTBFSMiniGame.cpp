// Fill out your copyright notice in the Description page of Project Settings.
#include "PTBFSMiniGame.h"
#include "FishActor.h"
#include "PTBFSCharacter.h"
#include "PTBFSMiniGameRuleSet.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"



APTBFSMiniGame::APTBFSMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
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
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] RhythmConductor 없음"));
	}

	FPTBMiniGameContext Context;
	Context.SessionRequest.MiniGameId = FName("FishMiniGame");
	Context.SessionRequest.Difficulty = EPTBDifficulty::Standard;
	InitializeMiniGame(Context);
	StartMiniGame();
	if (Character)
	{
		Character->OnPlayCastAnimMontage();
	}
}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	FishRuleSet = Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	Character = Cast<APTBFSCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	TArray<AActor*> FoundActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFishActor::StaticClass(), FoundActor);
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (FoundActor.Num() > 0)
	{
		FishActor = Cast<AFishActor>(FoundActor[0]);
	}

	if (!Character)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("캐릭터 유요하지않음"));
		return;
	}
	if (!FishActor)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("물고기 유요하지않음"));
		return;
	}
	if (!FishRuleSet)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("룰셋데이터에셋이 유효하지않음"));
		return;
	}
	if (!ChartAsset)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("차트에셋 유효하지않음"));
		return;
	}
	
	if (!GI)
	{
		PTB_WARNING(LogPTBMiniGames,TEXT("게임인스턴스 유효하지않음"));
		return;
	}
	FishStartLocation = FishActor->GetActorLocation();
	CharacterLocation = Character->GetActorLocation();
	int32 TotalNoteCount = ChartAsset->NoteEvents.Num();
	if (TotalNoteCount <= 0)return;

	StepDistance =  1.0f / TotalNoteCount;
	
	if (JudgementSystem)
	{
		JudgementSystem->HitWindowMissMs = 500.0f;
		JudgementSystem->HitWindowGoodMs = 350.0f;
		JudgementSystem->HitWindowPerfectMs = 200.0f;
		JudgementSystem->HitWindowHighPerfectMs = 80.0f;
	}
	
	PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] StepDistance: %f,TotalNoteCount: %d"), 
	StepDistance,TotalNoteCount);
	
	Character->SetFishLineTarget(FishActor);
	
	GI->CachedSettings.RhythmKeys.ActionA = EKeys::Q;
	GI->CachedSettings.RhythmKeys.ActionB = EKeys::W;
	GI->CachedSettings.RhythmKeys.ActionC = EKeys::E;
	GI->CachedSettings.RhythmKeys.ActionD = EKeys::R;
	GI->CachedSettings.RhythmKeys.ActionE = EKeys::F;
}

void APTBFSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);
	PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] HandleNoteCue 호출됨 MovingCount: %d FishDistance: %f"), MovingCount, FishDistance);
	if (!FishRuleSet)return;
	MovingCount++;
}

void APTBFSMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
}

void APTBFSMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
	OnFishingPromptShown.Broadcast(Note.ActionType,Note.bIsLongNote);
}

void APTBFSMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);

	if (Result.Reason == EPTBJudgementReason::EmptyInput) return;
	if (Result.Reason == EPTBJudgementReason::EarlyRelease)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] EarlyRelease NoteId: %d ActionType: %d"), Result.NoteId, static_cast<int32>(Result.ActionType));
		OnFishSlipped.Broadcast(1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		return;
	}
	switch (Result.JudgementType)
	{
	case EPTBJudgementType::HighPerfect:
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		CorrectCount++;
		break;
	case EPTBJudgementType::Perfect:
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		CorrectCount++;
		break;
	case EPTBJudgementType::Good:
		ApplyDistanceDelta(-StepDistance * 1.0f);
		OnFishingPromptReleased.Broadcast(Result.ActionType);
		OnFishPulled.Broadcast(2.0f);
		Character->OnPlayRealAnimMontage();
		CorrectCount++;
		break;
	case EPTBJudgementType::Miss:
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] Miss NoteId: %d ActionType: %d Reason: %d DeltaMs: %f"), 
		  Result.NoteId, 
		  static_cast<int32>(Result.ActionType),
		  static_cast<int32>(Result.Reason),
		  Result.DeltaMs);	
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] OnFishSlipped 브로드캐스트"));
		OnFishSlipped.Broadcast(1.0f);
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

void APTBFSMiniGame::OnAllNotesPassedFishing()
{
	OnFishRevealed.Broadcast(FishActor);
	// 원래는 캐릭터를 가져와서 해야함 Character->PlayAnim SuccessAnim;
	PTB_WARNING(LogPTBMiniGames, TEXT("성공애니메이션 재생"));
	TWeakObjectPtr<APTBFSMiniGame> WeakThis(this);
	GetWorldTimerManager().SetTimer(
		FishTimer,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->FinishMiniGame(EPTBRoundEndReason::Completed);
			}
		},
		2.0f,
		false
	);
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
		PTB_WARNING(LogPTBMiniGames, TEXT("[Fishing] FishActor TargetLoc: %s FishDistance: %f"), *TargetLoc.ToString(), FishDistance);
		FishActor->SetTargetLocation(FMath::Lerp(CharacterLocation, FishStartLocation, FishDistance));
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
