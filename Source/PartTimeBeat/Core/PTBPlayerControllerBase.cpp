#include "Core/PTBPlayerControllerBase.h"
#include "Core/PTBGameModeBase.h"
#include "Core/PTBGameInstance.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Kismet/GameplayStatics.h"

bool APTBPlayerControllerBase::InputKey(const FInputKeyEventArgs& EventArgs)
{
	// ── ESC: 일시정지 토글 ─────────────────────────────────────────
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

	// ── 리듬 입력: 활성 미니게임으로 라우팅 ───────────────────────
	if (TryRouteRhythmInput(EventArgs))
	{
		return true;
	}

	return Super::InputKey(EventArgs);
}

bool APTBPlayerControllerBase::TryRouteRhythmInput(const FInputKeyEventArgs& EventArgs)
{
	// Press / Release 만 처리 (Repeat 무시)
	if (EventArgs.Event != IE_Pressed && EventArgs.Event != IE_Released)
	{
		return false;
	}

	// 바인딩된 키인지 먼저 확인 (리듬 키가 아니면 조기 리턴)
	const FPTBRhythmKeyBindings Bindings = GetRhythmKeyBindings();
	const EPTBActionType Action = Bindings.ResolveKey(EventArgs.Key);
	if (Action == EPTBActionType::None)
	{
		return false;
	}

	// 활성 미니게임 탐색: GameMode → 없으면 월드에서 직접 검색 (레벨 직접 실행 시 폴백)
	APTBBaseMiniGame* MiniGame = nullptr;
	if (APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>())
	{
		MiniGame = GM->ActiveMiniGame;
	}
	if (!MiniGame)
	{
		MiniGame = Cast<APTBBaseMiniGame>(
			UGameplayStatics::GetActorOfClass(GetWorld(), APTBBaseMiniGame::StaticClass()));
	}

	if (!MiniGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PTBPlayerController] 리듬 입력 무시: 활성 미니게임 없음 (Key=%s)"),
			*EventArgs.Key.ToString());
		return false;
	}

	// 미니게임 내부에서 CanAcceptInput() 및 SupportedActions 검사가 수행됨
	if (EventArgs.Event == IE_Pressed)
	{
		MiniGame->HandleRhythmInput(Action);
	}
	else
	{
		MiniGame->HandleRhythmInputReleased(Action);
	}

	return true;
}

FPTBRhythmKeyBindings APTBPlayerControllerBase::GetRhythmKeyBindings() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI))
		{
			return PTBGI->CachedSettings.RhythmKeys;
		}
	}
	return FPTBRhythmKeyBindings{};
}
