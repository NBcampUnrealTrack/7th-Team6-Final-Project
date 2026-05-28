#include "Core/PTBPlayerControllerBase.h"
#include "Core/PTBGameModeBase.h"

bool APTBPlayerControllerBase::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Key == EKeys::Escape && EventArgs.Event == IE_Pressed)
	{
		APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>();

		// ── 진단 로그 (문제 해결 후 제거) ──────────────────────────
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
				FString::Printf(TEXT("[PTBPlayerController] ESC 감지 | GM=%s | bIsGameActive=%s | bIsPaused=%s"),
					GM ? TEXT("OK") : TEXT("NULL"),
					GM ? (GM->bIsGameActive ? TEXT("true") : TEXT("false")) : TEXT("?"),
					GM ? (GM->bIsPaused    ? TEXT("true") : TEXT("false")) : TEXT("?")));
		}
		// ────────────────────────────────────────────────────────────

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