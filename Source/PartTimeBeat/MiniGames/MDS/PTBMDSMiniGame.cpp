#include "PTBMDSMiniGame.h"


APTBMDSMiniGame::APTBMDSMiniGame()
{

}

void APTBMDSMiniGame::BeginPlay()
{
}

void APTBMDSMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();
}

void APTBMDSMiniGame::PreloadAudioAssets()
{
    Super::PreloadAudioAssets();
}

void APTBMDSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    GetRandomFoodData();
}

void APTBMDSMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);
}

void APTBMDSMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);
}

void APTBMDSMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);
}

TMap<FKey, EPTBActionType> APTBMDSMiniGame::GetActionMapping() const
{
    return {
        { EKeys::Z, EPTBActionType::ActionA }
    };
}

FPTBMDS_FoodRow APTBMDSMiniGame::GetRandomFoodData()
{
    if (!FoodDataTable) return FPTBMDS_FoodRow();

    // 모든 Row 이름 가져오기
    TArray<FName> RowNames = FoodDataTable->GetRowNames();

    // 랜덤 인덱스 선택
    int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
    FName RandomRowName = RowNames[RandomIndex];

    // 3. 해당 Row 데이터 추출
    FPTBMDS_FoodRow* RowData = FoodDataTable->FindRow<FPTBMDS_FoodRow>(RandomRowName, TEXT(""));

    return RowData ? *RowData : FPTBMDS_FoodRow();
}
