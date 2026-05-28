#include "Core/PTBPlayerControllerBase.h"
#include "Core/PTBGameModeBase.h"

bool APTBPlayerControllerBase::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Key == EKeys::Escape && EventArgs.Event == IE_Pressed)
	{
		APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>();

if (GM)
		{
			if (GM->bIsPaused)
				GM->ResumeGame();
			else
				GM->PauseGame();
			return true;
		}
	}

	return Super::InputKey(EventArgs);
}