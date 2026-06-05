#include "PTBSRMiniGame.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Debug/PTBLogChannels.h"

APTBSRMiniGame::APTBSRMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
	ToppingDataTable = nullptr;
}

// 아래 방향키(Down) 입력을 ActionA 판정 레인으로 강제 링킹
TMap<FKey, EPTBActionType> APTBSRMiniGame::GetActionMapping() const
{
	// 기본 A, S, D, F 구조를 무시하고 오직 아래 방향키 하나만 사용하도록 설정
	return 
	{
		{ EKeys::Down, EPTBActionType::ActionA }
	};
}

void APTBSRMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	// 1. 기본 방어 코드: 채보 에셋(노트)이 비어있다면 탈출
	if (!ChartAsset || ChartAsset->NoteEvents.IsEmpty())
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] ChartAsset이 비어있어 토핑 셔플을 스킵합니다."), *GetNameSafe(this));
		return;
	}

	// 2. 핵심 방어 코드: 에디터에서 데이터 테이블을 깜빡하고 안 넣었는지 체크
	if (!ToppingDataTable)
	{
		UE_LOG(LogRhythm, Error, TEXT("[%s] ToppingDataTable(데이터 테이블)이 지정되지 않았습니다! BP 디테일 패널을 확인하세요."), *GetNameSafe(this));
		return;
	}

	ToppingQueue.Reset();

	// 3. 기획자님이 데이터 테이블에 적어둔 모든 행 이름(Row Name: 1, 2, 3, 4...)을 자동으로 다 긁어옵니다.
	TArray<FName> RowNames = ToppingDataTable->GetRowNames();

	if (RowNames.IsEmpty())
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] 데이터 테이블이 완전히 비어있습니다."), *GetNameSafe(this));
		return;
	}

	int32 TotalNotes = ChartAsset->NoteEvents.Num();

	// 4. 음악 노트 개수만큼 반복문을 돌면서 무작위로 토핑을 섞어서 큐에 담습니다.
	for (int32 i = 0; i < TotalNotes; ++i)
	{
		// 데이터 테이블 행 이름 중 하나를 랜덤하게 선택 (예: FName "3")
		int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
		FName SelectedRowName = RowNames[RandomIndex];

		// 행 이름("1", "2" 등) 문자열을 정수(int32)로 안전하게 변환
		int32 ToppingTypeInt = FCString::Atoi(*SelectedRowName.ToString());

		// 💡 만약 행 이름을 문자로 지었거나 해서 정수 변환에 실패하면(0 리턴) 기본값 1번(계란)으로 방어 처리
		if (ToppingTypeInt == 0 && !SelectedRowName.ToString().Equals(TEXT("0")))
		{
			ToppingTypeInt = 1;
		}

		ToppingQueue.Add(ToppingTypeInt);
	}

	UE_LOG(LogRhythm, Log, TEXT("[%s] 데이터 테이블 기반 총 %d개의 초밥 토핑 무작위 셔플 완료!"), *GetNameSafe(this), ToppingQueue.Num());
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

	if (Result.JudgementType == EPTBJudgementType::Perfect )
	{
		
			OnSushiSuccessDelegate.Broadcast(Result.NoteId, Result.JudgementType);
		
	}
	else if (Result.JudgementType == EPTBJudgementType::Miss)
	{
		
			OnSushiMissDelegate.Broadcast(Result.NoteId);
		
	}
}

void APTBSRMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
{
	Super::PlayJudgementFeedback(Result);
}
