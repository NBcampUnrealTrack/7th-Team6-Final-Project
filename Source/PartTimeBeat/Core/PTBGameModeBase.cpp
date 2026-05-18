#include "Core/PTBGameModeBase.h"

void APTBGameModeBase::StartGameFlow(const FPTBGameSessionRequest& Request)
{

}
TSubclassOf<APTBBaseMiniGame> APTBGameModeBase::ResolveMiniGameClass(FName Id) const
{
	return NULL;
}
APTBBaseMiniGame* APTBGameModeBase::SpawnMiniGame(TSubclassOf<APTBBaseMiniGame> Cls)
{
	return NULL;
}

void APTBGameModeBase::BeginRound()
{

}
void APTBGameModeBase::PauseGame()
{

}
void APTBGameModeBase::ResumeGame()
{

}

void APTBGameModeBase::SubmitRoundResult(const FPTBRoundResult& Result)
{

}

void APTBGameModeBase::RetryGame()
{

}

void APTBGameModeBase::ExitToMenu()
{

}