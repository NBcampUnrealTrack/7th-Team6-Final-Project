#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBKeyPromptWidget.generated.h"

class UTextBlock;

/**
 * "상호작용: Enter 키" 같은 키 안내 문구 위젯 베이스 (WBP_Hud_EnterGuide 등).
 *
 * PromptText에 키 토큰을 쓰면 표시할 때 현재 키 이름으로 바뀜 (PTBKeyTextLibrary::FormatKeyTokens)
 *  {Interact} 상호작용 키 / {Locker} 사물함 키 / {ActionA}~{ActionE} 리듬 키 / {Player} 닉네임
 * 예) "상호작용: {Interact} 키" → "상호작용: Enter 키"
 *
 * WBP 구성 (선택사항)
 *  - Text_Prompt : 문구를 표시할 TextBlock
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBKeyPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPTBKeyPromptWidget(const FObjectInitializer& ObjectInitializer);

	/** 문구 교체 후 다시 표시 (상점마다 다른 문구를 쓸 때) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Prompt")
	void SetPromptText(const FText& InPromptText);

	/** 현재 키 설정으로 문구 다시 계산 (키 설정 변경 후, 다시 보일 때 호출) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Prompt")
	void RefreshPromptText();

	/** 토큰 치환 전 원본 문구 */
	UFUNCTION(BlueprintPure, Category = "PTB|Prompt")
	FText GetPromptText() const { return PromptText; }

	/** 토큰이 치환된 최종 문구 */
	UFUNCTION(BlueprintPure, Category = "PTB|Prompt")
	FText GetFormattedPromptText() const;

protected:
	virtual void NativeConstruct() override;

	/** 문구 갱신 직후 (BP: Text_Prompt 대신 다른 위젯에 표시하고 싶을 때 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Prompt")
	void OnPromptRefreshed(const FText& FormattedText);

	/** 키 토큰을 포함한 원본 문구 (다국어 수집 대상) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Prompt", meta = (MultiLine = true))
	FText PromptText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Prompt = nullptr;
};
