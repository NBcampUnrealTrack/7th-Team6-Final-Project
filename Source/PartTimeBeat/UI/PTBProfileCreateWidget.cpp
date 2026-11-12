// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileCreateWidget.h"
#include "Components/EditableTextBox.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Core/PTBStructEnums.h"
#include "UI/PTBLoadingScreenSubsystem.h"
#include "Components/Button.h"
#include "Core/PTBGameInstance.h"
#include "Flow/PTBGameFlowSubsystem.h"

void UPTBProfileCreateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Confirm) { Btn_Confirm->OnClicked.AddUniqueDynamic(this, &UPTBProfileCreateWidget::HandleConfirmClicked); }
	if (Btn_Back)    { Btn_Back->OnClicked.AddUniqueDynamic(this, &UPTBProfileCreateWidget::GoBack); }
	if (Btn_Model0)  { Btn_Model0->OnClicked.AddUniqueDynamic(this, &UPTBProfileCreateWidget::HandleModel0Clicked); }
	if (Btn_Model1)  { Btn_Model1->OnClicked.AddUniqueDynamic(this, &UPTBProfileCreateWidget::HandleModel1Clicked); }

	SetKeyboardFocus();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	// 조회 창에서 "프로필 수정"으로 넘어온 경우: 예약된 슬롯으로 수정 모드 자동 전환
	if (!bEditMode)
	{
		if (UPTBProfileSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBProfileSubsystem>() : nullptr)
		{
			int32 PendingSlot = INDEX_NONE;
			if (PS->ConsumePendingEditSlot(PendingSlot))
			{
				BeginEditProfile(PendingSlot);
			}
		}
	}

	// 초기 모델 선택 상태 동기화
	OnModelSelectionChanged(SelectedModelIndex);
}

FReply UPTBProfileCreateWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	// EditableTextBox가 포커스를 가지면 키 이벤트가 여기까지 오지 않음
	// → 텍스트 입력 중엔 모델 선택 키가 동작하지 않음

	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Left)
	{
		SelectPrevModel();
		return FReply::Handled();
	}

	if (Key == EKeys::Right)
	{
		SelectNextModel();
		return FReply::Handled();
	}

	if (Key == EKeys::Enter)
	{
		TryConfirm();
		return FReply::Handled();
	}

	if (Key == EKeys::Escape)
	{
		GoBack();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPTBProfileCreateWidget::SelectPrevModel()
{
	SelectedModelIndex = (SelectedModelIndex - 1 + ModelCount) % ModelCount;
	OnModelSelectionChanged(SelectedModelIndex);
}

void UPTBProfileCreateWidget::SelectNextModel()
{
	SelectedModelIndex = (SelectedModelIndex + 1) % ModelCount;
	OnModelSelectionChanged(SelectedModelIndex);
}

void UPTBProfileCreateWidget::SelectModel(int32 ModelIndex)
{
	if (ModelIndex < 0 || ModelIndex >= ModelCount)
	{
		return;
	}
	SelectedModelIndex = ModelIndex;
	// 같은 모델을 다시 눌러도 BP 갱신(강조 연출 등)이 일관되게 호출되도록 항상 알림
	OnModelSelectionChanged(SelectedModelIndex);
}

void UPTBProfileCreateWidget::HandleModel0Clicked()
{
	SelectModel(0);
}

void UPTBProfileCreateWidget::HandleModel1Clicked()
{
	SelectModel(1);
}

FString UPTBProfileCreateWidget::GetNickname() const
{
	if (!Input_Nickname) return FString();
	return Input_Nickname->GetText().ToString().TrimStartAndEnd();
}

void UPTBProfileCreateWidget::HandleConfirmClicked()
{
	TryConfirm();
}

void UPTBProfileCreateWidget::GoBack()
{
	// 생성/수정 모두 저장하지 않고 나감 (수정 모드는 TryConfirm에서만 저장되므로 여기선 아무것도 반영 안 됨)
	OnBackRequested();
	if (!ProfileSelectLevelName.IsNone())
	{
		UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, ProfileSelectLevelName);
	}
}

EPTBGender UPTBProfileCreateWidget::GetGenderForModelIndex(int32 ModelIndex) const
{
	return ModelGenders.IsValidIndex(ModelIndex) ? ModelGenders[ModelIndex] : EPTBGender::Unset;
}

bool UPTBProfileCreateWidget::BeginEditProfile(int32 SlotIndex)
{
	UPTBProfileSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	if (!PS)
	{
		return false;
	}

	bool bFound = false;
	const FPTBProfileData Profile = PS->GetProfileBySlot(SlotIndex, bFound);
	if (!bFound)
	{
		return false;
	}

	bEditMode = true;
	EditingSlotIndex = SlotIndex;
	EditingProfileId = Profile.ProfileId;

	const int32 GenderIndex = ModelGenders.IndexOfByKey(Profile.Gender);
	SelectedModelIndex = GenderIndex != INDEX_NONE ? GenderIndex : 0;

	if (Input_Nickname)
	{
		Input_Nickname->SetText(FText::FromString(Profile.Nickname));
	}

	OnModelSelectionChanged(SelectedModelIndex);
	OnEditModeStarted(Profile.Nickname, SelectedModelIndex);
	return true;
}

FString UPTBProfileCreateWidget::GetValidationFailMessage(EPTBNicknameValidationResult Result) const
{
	switch (Result)
	{
	case EPTBNicknameValidationResult::Empty:
		return TEXT("닉네임을 입력해주세요.");
	case EPTBNicknameValidationResult::TooShort:
		return FString::Printf(TEXT("닉네임은 %d자 이상이어야 합니다."), MinNicknameLength);
	case EPTBNicknameValidationResult::TooLong:
		return FString::Printf(TEXT("닉네임은 %d자 이하여야 합니다."), MaxNicknameLength);
	case EPTBNicknameValidationResult::AlreadyTaken:
		return TEXT("이미 사용 중인 닉네임입니다.");
	case EPTBNicknameValidationResult::ContainsForbiddenWord:
		return TEXT("사용할 수 없는 닉네임입니다.");
	default:
		return TEXT("닉네임이 유효하지 않습니다.");
	}
}

bool UPTBProfileCreateWidget::TryConfirm()
{
	const FString Nickname = GetNickname();

	UPTBProfileSubsystem* PS = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		PS = GI->GetSubsystem<UPTBProfileSubsystem>();
	}

	// ── 수정 모드: 검사 + 저장까지 여기서 처리 ──
	if (bEditMode)
	{
		if (!PS)
		{
			OnConfirmFailed(TEXT("프로필 정보를 불러올 수 없습니다."));
			return false;
		}

		const EPTBNicknameValidationResult Result =
			PS->UpdateProfileIdentity(EditingProfileId, Nickname, GetGenderForModelIndex(SelectedModelIndex));
		if (Result != EPTBNicknameValidationResult::Valid)
		{
			OnConfirmFailed(GetValidationFailMessage(Result));
			return false;
		}

		OnEditConfirmed(Nickname, SelectedModelIndex);
		if (!ProfileSelectLevelName.IsNone())
		{
			UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, ProfileSelectLevelName);
		}
		return true;
	}

	// ── 생성 모드 (기존 동작: 검사만 하고 저장은 BP) ──
	if (!PS)
	{
		// 서브시스템 취득 실패 — 최소한의 빈 문자열 검사만 수행
		if (Nickname.IsEmpty())
		{
			OnConfirmFailed(TEXT("닉네임을 입력해주세요."));
			return false;
		}
		OnConfirmSuccess(Nickname, SelectedModelIndex);
		return true;
	}

	const EPTBNicknameValidationResult Result = PS->ValidateNickname(Nickname);
	if (Result == EPTBNicknameValidationResult::Valid)
	{
		if (!bCreateAndStartGame)
		{
			// 예전 방식: 생성·이동은 BP의 OnConfirmSuccess에서
			OnConfirmSuccess(Nickname, SelectedModelIndex);
			return true;
		}

		// 생성 → 활성화 → 바로 게임 시작
		const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance());
		const int32 TargetSlot = PTBGI ? PTBGI->PendingSlotIndex : 0;
		const FGuid NewId = PS->CreateProfile(Nickname, GetGenderForModelIndex(SelectedModelIndex), TargetSlot);
		if (!NewId.IsValid())
		{
			OnConfirmFailed(TEXT("프로필을 만들 수 없습니다. (슬롯이 이미 사용 중이거나 가득 찼습니다)"));
			return false;
		}

		PS->SetActiveProfile(NewId);
		OnProfileCreated(Nickname, SelectedModelIndex);

		if (UPTBGameFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBGameFlowSubsystem>() : nullptr)
		{
			Flow->ReturnToMiniGameSelect();
		}
		else
		{
			UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, TEXT("L_GameMap"));
		}
		return true;
	}

	OnConfirmFailed(GetValidationFailMessage(Result));
	return false;
}
