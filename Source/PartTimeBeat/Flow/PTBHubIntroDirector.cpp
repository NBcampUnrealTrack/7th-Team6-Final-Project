#include "Flow/PTBHubIntroDirector.h"

#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Profile/PTBProfileSubsystem.h"
#include "TimerManager.h"
#include "UI/PTBSceneWidgetBase.h"

APTBHubIntroDirector::APTBHubIntroDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APTBHubIntroDirector::BeginPlay()
{
	Super::BeginPlay();

	if (ProfileHUDClass)
	{
		if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			ProfileHUD = CreateWidget<UUserWidget>(PC, ProfileHUDClass);
			if (ProfileHUD)
			{
				ProfileHUD->AddToViewport(ProfileHUDZOrder);
				// 오프닝 여부가 정해질 때까지 숨김 → 볼 게 없거나 다 끝나면 표시
				SetProfileHUDVisible(false);
			}
		}
	}

	if (StartDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StartTimerHandle, this, &APTBHubIntroDirector::Begin, StartDelay, false);
	}
	else
	{
		Begin();
	}
}

void APTBHubIntroDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(StartTimerHandle);
	if (ActiveWidget)
	{
		ActiveWidget->OnSceneFinished.RemoveDynamic(this, &APTBHubIntroDirector::HandleSceneFinished);
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}
	if (ProfileHUD)
	{
		ProfileHUD->RemoveFromParent();
		ProfileHUD = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void APTBHubIntroDirector::Begin()
{
	Stage = EStage::None;
	AdvanceStage();
}

void APTBHubIntroDirector::ReplayOpening()
{
	if (ActiveWidget)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(StartTimerHandle);
	bForceOpening = true;
	Begin();
}

bool APTBHubIntroDirector::ShouldPlay(FName SceneKey, int32 Version) const
{
	if (bAlwaysPlay)
	{
		return true;
	}

	const UGameInstance* GI = GetGameInstance();
	const UPTBProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	if (!Profiles || !Profiles->HasActiveProfile())
	{
		// 프로필 없이 맵을 직접 연 경우(PIE 테스트 등): 기록할 곳이 없으니 재생하지 않음
		return false;
	}
	return !Profiles->HasSeenScene(SceneKey, Version);
}

void APTBHubIntroDirector::AdvanceStage()
{
	const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance());

	if (Stage == EStage::None)
	{
		Stage = EStage::Opening;
		const bool bPlay = OpeningStoryTable && (bForceOpening || ShouldPlay(OpeningSceneKey, OpeningStoryVersion));
		bForceOpening = false;
		TSubclassOf<UPTBSceneWidgetBase> WidgetClass = StoryWidgetClass;
		if (!WidgetClass && PTBGI)
		{
			WidgetClass = PTBGI->StoryWidgetClass;
		}
		if (bPlay && ShowScene(WidgetClass, OpeningStoryTable, false))
		{
			return;
		}
	}

	if (Stage == EStage::Opening)
	{
		Stage = EStage::Guide;
		const bool bPlay = GuideTable && ShouldPlay(GuideSceneKey, GuideVersion);
		TSubclassOf<UPTBSceneWidgetBase> WidgetClass = GuideWidgetClass;
		if (!WidgetClass && PTBGI)
		{
			WidgetClass = PTBGI->TutorialWidgetClass;
		}
		if (bPlay && ShowScene(WidgetClass, GuideTable, true))
		{
			return;
		}
	}

	FinishAll();
}

bool APTBHubIntroDirector::ShowScene(TSubclassOf<UPTBSceneWidgetBase> WidgetClass, UDataTable* Table, bool bTutorialPhase)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!WidgetClass || !Table || !PC)
	{
		PTB_WARNING(LogPTBFlow, TEXT("[HubIntro] 위젯 클래스/테이블/PC 없음. 단계 건너뜀 (Tutorial=%d)"), bTutorialPhase ? 1 : 0);
		return false;
	}

	UPTBSceneWidgetBase* Widget = CreateWidget<UPTBSceneWidgetBase>(PC, WidgetClass);
	if (!Widget)
	{
		return false;
	}

	ActiveWidget = Widget;
	SetProfileHUDVisible(false);
	Widget->OnSceneFinished.AddDynamic(this, &APTBHubIntroDirector::HandleSceneFinished);
	Widget->AddToViewport(WidgetZOrder);
	EnterUIMode(Widget);

	// 미니게임 없이 사용 (OwnerMiniGame = nullptr → 끝나면 OnSceneFinished로 알림)
	Widget->SetupScene(nullptr,
		bTutorialPhase ? EPTBMiniGameScenePhase::Tutorial : EPTBMiniGameScenePhase::IntroStory,
		Table);
	return true;
}

void APTBHubIntroDirector::HandleSceneFinished(UPTBSceneWidgetBase* SceneWidget, bool bSkipped)
{
	if (SceneWidget)
	{
		SceneWidget->OnSceneFinished.RemoveDynamic(this, &APTBHubIntroDirector::HandleSceneFinished);
	}
	ActiveWidget = nullptr;

	// 끝까지 봤든 스킵했든 본 것으로 기록
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBProfileSubsystem* Profiles = GI->GetSubsystem<UPTBProfileSubsystem>())
		{
			if (Profiles->HasActiveProfile())
			{
				if (Stage == EStage::Opening)
				{
					Profiles->MarkSceneSeen(OpeningSceneKey, OpeningStoryVersion);
				}
				else if (Stage == EStage::Guide)
				{
					Profiles->MarkSceneSeen(GuideSceneKey, GuideVersion);
				}
			}
		}
	}

	PTB_RECORD(LogPTBFlow, TEXT("[HubIntro] Stage %d finished (Skipped=%d)"), static_cast<int32>(Stage), bSkipped ? 1 : 0);
	AdvanceStage();
}

void APTBHubIntroDirector::FinishAll()
{
	Stage = EStage::Done;
	RestoreInputMode();
	SetProfileHUDVisible(true);
	OnHubIntroFinished.Broadcast();
}

void APTBHubIntroDirector::SetProfileHUDVisible(bool bVisible)
{
	if (ProfileHUD)
	{
		ProfileHUD->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void APTBHubIntroDirector::EnterUIMode(UUserWidget* FocusWidget)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	if (!bInputModeChanged)
	{
		bPrevShowMouseCursor = PC->bShowMouseCursor;
		bInputModeChanged = true;
	}

	// 스토리/안내 중에는 캐릭터 이동·점프 막고, 위젯이 Enter/Space/클릭을 받도록
	FInputModeUIOnly InputMode;
	if (FocusWidget)
	{
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}

void APTBHubIntroDirector::RestoreInputMode()
{
	if (!bInputModeChanged)
	{
		return;
	}
	bInputModeChanged = false;

	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		// 원래 마우스 커서를 쓰던 맵이면(HUD 버튼 클릭 등) Game+UI, 아니면 Game 전용으로 복구
		if (bPrevShowMouseCursor)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
		PC->SetShowMouseCursor(bPrevShowMouseCursor);
	}
}
