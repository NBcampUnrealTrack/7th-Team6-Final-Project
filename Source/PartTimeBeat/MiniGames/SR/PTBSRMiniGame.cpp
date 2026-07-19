#include "PTBSRMiniGame.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "SRWidget.h"

APTBSRMiniGame::APTBSRMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
	ToppingDataTable = nullptr;

	PlateMoveSpeed = -300.0f;
	SpawnerZHeight = 50.0f;
}

void APTBSRMiniGame::BeginPlay()
{
	Super::BeginPlay();
}

TMap<FKey, EPTBActionType> APTBSRMiniGame::GetActionMapping() const
{
	return {
		{ GameContext.UserSettings.RhythmKeys.ActionA, EPTBActionType::ActionA }
	};
}

void APTBSRMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	Super::InitializeMiniGame(Context);
	CurrentNoteIndex = 0;
}

FPTBRoundResult APTBSRMiniGame::FinishMiniGame(EPTBRoundEndReason Reason)
{
	FPTBRoundResult FinalResult = Super::FinishMiniGame(Reason);

	FinalResult.MiniGameId = FName("SR");
	FinalResult.Difficulty = GameContext.SessionRequest.Difficulty;
	FinalResult.PlayMode = GameContext.SessionRequest.PlayMode;

	int32 TotalSuccessSushi = SuccessSushiCount;

	// 초밥 1개당 1,000점 / 초밥 1개당 1,900원 계산 / 26개
	FinalResult.Score = TotalSuccessSushi * 1000;

	FPTBRewardSummary RewardSummary;
	RewardSummary.EarnedMoney = TotalSuccessSushi * 1900;

	if (FinalResult.Score >= 26000)       FinalResult.Grade = EPTBGradeType::S;
	else if (FinalResult.Score >= 20000)  FinalResult.Grade = EPTBGradeType::A;
	else if (FinalResult.Score >= 13000)  FinalResult.Grade = EPTBGradeType::B;
	else if (FinalResult.Score >= 5000)   FinalResult.Grade = EPTBGradeType::C;
	else                                  FinalResult.Grade = EPTBGradeType::Fail;

	RoundResult = FinalResult;

	if (OnMiniGameFinished.IsBound())
	{
		OnMiniGameFinished.Broadcast(FinalResult);
	}

	return FinalResult;
}

void APTBSRMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	PTB_WARNING(LogPTBMiniGames, TEXT("빌드 런타임 실행됨."));

	if (!ChartAsset || ChartAsset->NoteEvents.IsEmpty()) return;
	if (!ToppingDataTable) return;

	ToppingQueue.Reset();
	TArray<FName> RowNames = ToppingDataTable->GetRowNames();

	if (RowNames.IsEmpty()) return;

	int32 TotalNotes = ChartAsset->NoteEvents.Num();

	for (int32 i = 0; i < TotalNotes; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
		FName SelectedRowName = RowNames[RandomIndex];

		int32 ToppingTypeInt = FCString::Atoi(*SelectedRowName.ToString());

		if (ToppingTypeInt == 0 && !SelectedRowName.ToString().Equals(TEXT("0")))
		{
			ToppingTypeInt = 1;
		}

		ToppingQueue.Add(ToppingTypeInt);
	}

	UE_LOG(LogRhythm, Log, TEXT("[%s] 데이터 테이블 기반 총 %d개의 초밥 채보 토핑 무작위 셔플 완료!"), *GetNameSafe(this), ToppingQueue.Num());
}

int32 APTBSRMiniGame::GetToppingTypeFromQueue(int32 NoteId) const
{
	if (ToppingQueue.IsValidIndex(NoteId))
	{
		return ToppingQueue[NoteId];
	}
	return 1;
}

FPTBToppingRow APTBSRMiniGame::GetToppingData(int32 ToppingType) const
{
	FPTBToppingRow DefaultRow;
	DefaultRow.SR_ToppingName = TEXT("기본 계란");
	DefaultRow.SR_ToppingMesh = nullptr;
	DefaultRow.SR_GravityScale = 1.0f;
	DefaultRow.SR_ToppingMaterial = nullptr;
	DefaultRow.SR_ToppingIcon = nullptr;

	if (!ToppingDataTable) return DefaultRow;

	FName RowName = *FString::FromInt(ToppingType);

	// 🚨 incorrect type 에러 차단을 위해 TEXT("") 적용
	FPTBToppingRow* FoundRow = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));

	if (FoundRow)
	{
		return *FoundRow;
	}

	return DefaultRow;
}

void APTBSRMiniGame::RefreshPreviewUI()
{
	if (!WBP_SR_Preview)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] RefreshPreviewUI: WBP_SR_Preview 위젯 변수가 nullptr입니다!"), *GetNameSafe(this));
		return;
	}
	/*
	if (!ToppingDataTable) return;

	int32 NextToppingType = ToppingQueue.IsValidIndex(CurrentNoteIndex) ? ToppingQueue[CurrentNoteIndex] : 1;
	FName RowName = *FString::FromInt(NextToppingType);

	// 🚨 [수정] 이 부분도 TEXT("")로 변경하여 무조건 안전하게 에러를 방어합니다!
	FPTBToppingRow* ToppingData = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));

	if (ToppingData)
	{
		if (!ToppingData->SR_ToppingIcon) return;

		UImage* TargetImage = Cast<UImage>(WBP_SR_Preview->GetWidgetFromName(TEXT("ImgNextTopping")));
		if (!TargetImage)
		{
			TargetImage = Cast<UImage>(WBP_SR_Preview->GetWidgetFromName(TEXT("Img_NextTopping")));
		}

		if (TargetImage)
		{
			TargetImage->SetBrushFromTexture(ToppingData->SR_ToppingIcon);
		}
	}*/
	USRWidget* MySushiUI = Cast<USRWidget>(WBP_SR_Preview);
	if (!MySushiUI || !ToppingDataTable) return;

	int32 NextToppingType = ToppingQueue.IsValidIndex(CurrentNoteIndex) ? ToppingQueue[CurrentNoteIndex] : 1;
	FName RowName = *FString::FromInt(NextToppingType);

	FPTBToppingRow* ToppingData = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));
	if (ToppingData && ToppingData->SR_ToppingIcon)
	{
		MySushiUI->UpdatePreviewImage(ToppingData->SR_ToppingIcon);
	}
}

void APTBSRMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
	const int32 NoteIndexForThisInput = CurrentNoteIndex;

	Super::HandleRhythmInput(Action, TimeMs);
	// SR은 ActionA(Z)만 지원하므로, 그 외 키 입력은 무시
	if (Action != EPTBActionType::ActionA)
	{
		return;
	}

	// 토핑(스시)은 키를 누를 때마다 스폰 — 캡처해둔 인덱스를 그대로 사용
	if (ToppingQueue.IsValidIndex(NoteIndexForThisInput))
	{
		int32 AssignedTopping = ToppingQueue[NoteIndexForThisInput];
		OnToppingDrop.Broadcast(AssignedTopping, NoteIndexForThisInput); // 델리게이트 이름 변경
	}
}



void APTBSRMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	if (Note.ActionType == EPTBActionType::ActionA)
	{
		int32 AssignedTopping = ToppingQueue.IsValidIndex(Note.NoteId) ? ToppingQueue[Note.NoteId] : 1;
		OnSushiPlateSpawn.Broadcast(AssignedTopping, Note.NoteId);
	}
}

void APTBSRMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
	NoteJudgementResults.Add(Result.NoteId, Result.JudgementType); // 결과 저장

	if (Result.JudgementType != EPTBJudgementType::Miss)
	{
		++SuccessSushiCount;
		OnSushiSuccessDelegate.Broadcast(Result.NoteId, Result.JudgementType);
	}
	else
	{
		OnSushiMissDelegate.Broadcast(Result.NoteId);
	}

	CurrentNoteIndex++;
	RefreshPreviewUI();
}