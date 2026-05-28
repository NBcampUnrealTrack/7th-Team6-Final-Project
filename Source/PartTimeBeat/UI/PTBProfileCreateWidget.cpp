// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileCreateWidget.h"
#include "Components/EditableTextBox.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Core/PTBStructEnums.h"

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

bool UPTBProfileCreateWidget::TryConfirm()
{
	const FString Nickname = GetNickname();

	UPTBProfileSubsystem* PS = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		PS = GI->GetSubsystem<UPTBProfileSubsystem>();
	}

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

	switch (PS->ValidateNickname(Nickname))
	{
	case EPTBNicknameValidationResult::Valid:
		OnConfirmSuccess(Nickname, SelectedModelIndex);
		return true;

	case EPTBNicknameValidationResult::Empty:
		OnConfirmFailed(TEXT("닉네임을 입력해주세요."));
		break;

	case EPTBNicknameValidationResult::TooShort:
		OnConfirmFailed(FString::Printf(
			TEXT("닉네임은 %d자 이상이어야 합니다."), MinNicknameLength));
		break;

	case EPTBNicknameValidationResult::TooLong:
		OnConfirmFailed(FString::Printf(
			TEXT("닉네임은 %d자 이하여야 합니다."), MaxNicknameLength));
		break;

	case EPTBNicknameValidationResult::AlreadyTaken:
		OnConfirmFailed(TEXT("이미 사용 중인 닉네임입니다."));
		break;

	case EPTBNicknameValidationResult::ContainsForbiddenWord:
		OnConfirmFailed(TEXT("사용할 수 없는 닉네임입니다."));
		break;

	default:
		OnConfirmFailed(TEXT("닉네임이 유효하지 않습니다."));
		break;
	}

	return false;
}