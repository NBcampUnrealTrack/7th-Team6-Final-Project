#include "UI/PTBProfileInfoDialogWidget.h"

#include "Components/Button.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "UI/PTBConfirmDialogWidget.h"
#include "UI/PTBDialogHostWidget.h"
#include "UI/PTBLoadingScreenSubsystem.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"

void UPTBProfileInfoDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_StartGame)     { Btn_StartGame->OnClicked.AddUniqueDynamic(this, &UPTBProfileInfoDialogWidget::StartGame); }
	if (Btn_EditProfile)   { Btn_EditProfile->OnClicked.AddUniqueDynamic(this, &UPTBProfileInfoDialogWidget::RequestEdit); }
	if (Btn_DeleteProfile) { Btn_DeleteProfile->OnClicked.AddUniqueDynamic(this, &UPTBProfileInfoDialogWidget::RequestDelete); }
	if (Btn_Close)         { Btn_Close->OnClicked.AddUniqueDynamic(this, &UPTBProfileInfoDialogWidget::CloseDialog); }

	// SetupForSlot이 화면에 붙기 전에 호출됐어도 BP 갱신을 한 번 더 보장
	if (bSetupDone)
	{
		OnSummaryUpdated(Summary);
	}
}

UPTBProfileSubsystem* UPTBProfileInfoDialogWidget::GetProfileSubsystem() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
}

bool UPTBProfileInfoDialogWidget::SetupForSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;

	UPTBProfileSubsystem* PS = GetProfileSubsystem();
	if (!PS)
	{
		return false;
	}

	Summary = PS->GetProfileSummaryBySlot(InSlotIndex);
	bSetupDone = true;
	OnSummaryUpdated(Summary);
	return Summary.bHasProfile;
}

FText UPTBProfileInfoDialogWidget::GetProgressText() const
{
	return FText::FromString(FString::Printf(TEXT("%d / %d"), Summary.ClearedMiniGameCount, Summary.TotalMiniGameCount));
}

void UPTBProfileInfoDialogWidget::StartGame_Implementation()
{
	UPTBProfileSubsystem* PS = GetProfileSubsystem();
	if (!PS || !Summary.bHasProfile)
	{
		return;
	}

	if (!PS->SetActiveProfile(Summary.ProfileId))
	{
		PTB_WARNING(LogPTBUI, TEXT("[ProfileInfo] 게임 시작 실패: 프로필 활성화 실패 (Slot=%d)"), SlotIndex);
		return;
	}

	PTB_RECORD(LogPTBUI, TEXT("[ProfileInfo] 게임 시작: Slot=%d %s"), SlotIndex, *Summary.Nickname);

	if (!StartLevelNameOverride.IsNone())
	{
		UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, StartLevelNameOverride);
		return;
	}

	// 기본: 허브(미니게임 선택 맵)로 이동 — Flow 상태 갱신 + 로딩 화면 포함
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBGameFlowSubsystem* Flow = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			Flow->ReturnToMiniGameSelect();
			return;
		}
	}
	UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, TEXT("L_GameMap"));
}

void UPTBProfileInfoDialogWidget::RequestEdit()
{
	if (!Summary.bHasProfile)
	{
		return;
	}

	OnEditRequested.Broadcast(SlotIndex);

	if (!ProfileEditLevelName.IsNone())
	{
		if (UPTBProfileSubsystem* PS = GetProfileSubsystem())
		{
			PS->SetPendingEditSlot(SlotIndex);
		}
		PTB_RECORD(LogPTBUI, TEXT("[ProfileInfo] 프로필 수정: Slot=%d → %s"), SlotIndex, *ProfileEditLevelName.ToString());
		UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, ProfileEditLevelName);
	}
}

void UPTBProfileInfoDialogWidget::RequestDelete()
{
	if (!Summary.bHasProfile)
	{
		return;
	}

	UPTBDialogHostWidget* Host = UPTBConfirmDialogWidget::FindDialogHostOf(this);
	if (!DeleteConfirmDialogClass || !Host)
	{
		ConfirmDelete();
		return;
	}

	UPTBConfirmDialogWidget* Confirm = CreateWidget<UPTBConfirmDialogWidget>(GetOwningPlayer(), DeleteConfirmDialogClass);
	if (!Confirm)
	{
		return;
	}
	Confirm->OnConfirmed.AddUniqueDynamic(this, &UPTBProfileInfoDialogWidget::ConfirmDelete);
	Host->PushDialog(Confirm);

	if (bHideWhileDeleteConfirmOpen)
	{
		ActiveDeleteConfirm = Confirm;
		HideForDeleteConfirm();
	}
}

void UPTBProfileInfoDialogWidget::HideForDeleteConfirm()
{
	if (bHiddenForDeleteConfirm)
	{
		return;
	}
	bHiddenForDeleteConfirm = true;
	SavedRenderOpacity = GetRenderOpacity();
	SavedVisibility = GetVisibility();

	// Collapsed/Hidden은 Tick이 멈춰서 확인 창이 닫혔는지 감지할 수 없음 → 투명 + 클릭 불가로 숨김
	SetRenderOpacity(0.f);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPTBProfileInfoDialogWidget::RestoreAfterDeleteConfirm()
{
	if (!bHiddenForDeleteConfirm)
	{
		return;
	}
	bHiddenForDeleteConfirm = false;
	ActiveDeleteConfirm.Reset();
	SetRenderOpacity(SavedRenderOpacity);
	SetVisibility(SavedVisibility);
}

void UPTBProfileInfoDialogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 확인 창이 닫혔으면(아니오 / ESC / 닫기 버튼 등 어떤 방법이든) 다시 보이게.
	// "예"로 삭제한 경우엔 이 창도 같은 프레임에 닫히므로 다시 보이지 않음.
	if (bHiddenForDeleteConfirm)
	{
		const UPTBConfirmDialogWidget* Confirm = ActiveDeleteConfirm.Get();
		if (!Confirm || (!Confirm->GetParent() && !Confirm->IsInViewport()))
		{
			RestoreAfterDeleteConfirm();
		}
	}
}

void UPTBProfileInfoDialogWidget::ConfirmDelete()
{
	UPTBProfileSubsystem* PS = GetProfileSubsystem();
	if (!PS || !Summary.bHasProfile)
	{
		return;
	}

	const int32 DeletedSlot = SlotIndex;
	if (PS->DeleteProfileBySlot(DeletedSlot))
	{
		PTB_RECORD(LogPTBUI, TEXT("[ProfileInfo] 프로필 삭제: Slot=%d"), DeletedSlot);
		Summary = FPTBProfileSummary();
		OnProfileDeleted.Broadcast(DeletedSlot);
		CloseDialog();
	}
}

void UPTBProfileInfoDialogWidget::CloseDialog()
{
	OnInfoDialogClosed.Broadcast();

	if (UPTBDialogHostWidget* Host = UPTBConfirmDialogWidget::FindDialogHostOf(this))
	{
		if (Host->IsDialogOpen(GetClass()))
		{
			Host->CloseTopDialog();
			return;
		}
	}
	RemoveFromParent();
}
