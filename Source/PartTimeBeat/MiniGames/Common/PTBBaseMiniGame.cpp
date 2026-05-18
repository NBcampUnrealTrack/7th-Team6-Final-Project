#include "MiniGames/Common/PTBBaseMiniGame.h"

APTBBaseMiniGame::APTBBaseMiniGame()
{
 	PrimaryActorTick.bCanEverTick = true;
}

void APTBBaseMiniGame::BeginPlay()
{
	Super::BeginPlay();
}

void APTBBaseMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APTBBaseMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{

}

void APTBBaseMiniGame::PreloadAssets()
{

}

void APTBBaseMiniGame::BuildRuntimeState()
{

}

void APTBBaseMiniGame::StartMiniGame()
{

}

FPTBRoundResult APTBBaseMiniGame::FinishMiniGame(EPTBRoundEndReason Reason)
{
	return FPTBRoundResult();
}

void APTBBaseMiniGame::PauseMiniGame()
{

}

void APTBBaseMiniGame::ResumeMiniGame()
{

}

void APTBBaseMiniGame::HandleChartEvent(const FPTBNoteEvent& Note)
{

}

void APTBBaseMiniGame::HandleNoteCue(const FPTBNoteEvent& Note)
{

}

void APTBBaseMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{

}
FPTBJudgementResult APTBBaseMiniGame::EvaluateInput(EPTBActionType Action, float TimeMs)
{
	return FPTBJudgementResult();
}
void APTBBaseMiniGame::HandleJudgementResult(const FPTBJudgementResult& Result)
{

}

void APTBBaseMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
{

}

//	전용 결과(하위 override)
FPTBMiniGameResultPayload APTBBaseMiniGame::BuildResultPayload() const
{
	return FPTBMiniGameResultPayload();
}
//	AudioManager에 이벤트 요청
void APTBBaseMiniGame::RequestWwiseEvent(FName EventKey, AActor* Target = nullptr)
{

}
//	입력 가능 여부
bool APTBBaseMiniGame::CanAcceptInput() const
{
	return true;
}
//	키 - 액션 매핑(하위 override)
TMap<FKey, EPTBActionType> APTBBaseMiniGame::GetActionMapping() const
{
	return TMap<FKey, EPTBActionType>();
}