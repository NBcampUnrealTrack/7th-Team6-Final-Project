#include "MiniGames/PC/PTBPCMiniGame.h"

APTBPCMiniGame::APTBPCMiniGame()
{}

void APTBPCMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();


}

void APTBPCMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    SpawnCueForNote(Note);
}

void APTBPCMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);

}

void APTBPCMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);

}

void APTBPCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);

    ClearCueForJudgement(Result);

}

void APTBPCMiniGame::SpawnCueForNote(const FPTBNoteEvent& Note)
{
}

void APTBPCMiniGame::ClearCueForJudgement(const FPTBJudgementResult& Result)
{
}