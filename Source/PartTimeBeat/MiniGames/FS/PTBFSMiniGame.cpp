// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSMiniGame.h"
#include "Characters/PTBRhythmCharacterBase.h"
#include "FishActor.h"
#include "PTBFSMiniGameRuleSet.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"


// Sets default values
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
}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	FishRuleSet = Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	Character = Cast<APTBRhythmCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	TArray<AActor*> FoundActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFishActor::StaticClass(), FoundActor);
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
	FishStartLocation = FishActor->GetActorLocation();
	CharacterLocation = Character->GetActorLocation();
	int32 TotalNoteCount = ChartAsset->NoteEvents.Num();
	if (TotalNoteCount <= 0)return;
	float TotalDistance = FVector::Dist(FishActor->GetActorLocation(), Character->GetActorLocation());
	StepDistance = TotalDistance / TotalNoteCount;
}

void APTBFSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);
	if (!FishRuleSet)return;
	MovingCount++;
	ApplyDistanceDelta(StepDistance * AutoDriftMultiplier);
	OnFishingPromptShown.Broadcast(Note.ActionType);
}

void APTBFSMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
}

void APTBFSMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
}

void APTBFSMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);

	if (Result.Reason == EPTBJudgementReason::EmptyInput) return;

	switch (Result.JudgementType)
	{
	case EPTBJudgementType::HighPerfect:
		OnFishPulled.Broadcast(1.0f);
		ApplyDistanceDelta(-StepDistance * 1.0f);
		CorrectCount++;
		break;
	case EPTBJudgementType::Perfect:
		ApplyDistanceDelta(-StepDistance * 0.7f);
		OnFishPulled.Broadcast(0.7f);
		CorrectCount++;
		break;
	case EPTBJudgementType::Good:
		ApplyDistanceDelta(-StepDistance * 0.4f);
		OnFishPulled.Broadcast(0.4f);
		CorrectCount++;
		break;
	case EPTBJudgementType::Miss:
		OnFishSlipped.Broadcast(1.0f);
		break;
	}
}

void APTBFSMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
{
	Super::PlayJudgementFeedback(Result);
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
