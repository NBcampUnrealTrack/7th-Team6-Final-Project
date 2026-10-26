#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBHubProfileWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 허브 맵(L_GameMap) 왼쪽 아래 프로필 HUD.
 *
 * WBP 구성 (이름이 같으면 자동 연결, 전부 선택사항):
 *  - ImagePortrait   : 캐릭터 프로필 이미지 (Image). 장착한 초상화 아이템의 Asset 우선, 비어 있으면 PortraitByGender
 *  - TextPlayerName  : 플레이어 이름 (TextBlock). 활성 프로필 닉네임
 *  - TextMoney       : 현재 소지금 (TextBlock, 누적 알바비 - 상점 사용 금액). MoneyFormat 형식으로 표시
 *
 * 사물함은 I 키, 상점은 허브의 상점 오브젝트(Enter)로 진입 → PTBHubUISubsystem
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBHubProfileWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 프로필 정보 다시 읽어서 표시 (금액 변경 후 등) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	void RefreshProfile();

	/** 활성 프로필 소지금 (누적 알바비 - 상점 사용 금액) */
	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	int32 GetCurrentMoney() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 표시 갱신 직후 (BP 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Hub")
	void OnProfileRefreshed(const FText& PlayerName, int32 Money);

	/** 성별별 프로필 이미지. 없으면 DefaultPortrait */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub")
	TMap<EPTBGender, TObjectPtr<UTexture2D>> PortraitByGender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub")
	TObjectPtr<UTexture2D> DefaultPortrait;

	/** 금액 표시 형식. {Money} = 천 단위 쉼표 숫자 (화폐 단위 바꿀 때 여기만 수정, 다국어 수집 대상) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub")
	FText MoneyFormat;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImagePortrait = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextPlayerName = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextMoney = nullptr;

public:
	UPTBHubProfileWidget(const FObjectInitializer& ObjectInitializer);

private:
	void HandleProfileListChanged();

	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	UFUNCTION()
	void HandleMoneyChanged(int32 CurrentMoney);

	FDelegateHandle ProfileListChangedHandle;
};
