#include "Core/PTBGameFlowSubsystem.h"

void UPTBGameFlowSubsystem::SetFlowState(EGameFlowState NewState)
{
}
bool UPTBGameFlowSubsystem::SelectProfile(const FString& ProfileId) 
{
	return true;
}
void UPTBGameFlowSubsystem::SelectPlayMode(EPTBPlayMode InMode) 
{
}
bool UPTBGameFlowSubsystem::SelectMiniGame(FName InId) 
{
	return true;
}
void UPTBGameFlowSubsystem::SelectDifficulty(EPTBDifficulty InDiff)
{
}
void UPTBGameFlowSubsystem::StartGameplay() 
{
}
void UPTBGameFlowSubsystem::FinishGameplay(const FPTBRoundResult & Result)
{
}
void UPTBGameFlowSubsystem::ReturnToMiniGameSelect() {
}
void UPTBGameFlowSubsystem::OpenSettings() {
}
void UPTBGameFlowSubsystem::CloseSettings() {
}