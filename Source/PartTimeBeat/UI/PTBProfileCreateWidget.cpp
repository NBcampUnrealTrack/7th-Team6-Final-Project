// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileCreateWidget.h"
#include "Components/EditableTextBox.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Core/PTBStructEnums.h"
#include "UI/PTBLoadingScreenSubsystem.h"

void UPTBProfileCreateWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

FString UPTBProfileCreateWidget::GetNickname() const
{
	if (!Input_Nickname) return FString();
	return Input_Nickname->GetText().ToString().TrimStartAndEnd();
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
		if (!ReturnLevelAfterEdit.IsNone())
		{
			UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(this, ReturnLevelAfterEdit);
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
		OnConfirmSuccess(Nickname, SelectedModelIndex);
		return true;
	}

	OnConfirmFailed(GetValidationFailMessage(Result));
	return false;
}
