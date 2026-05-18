// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBHUDBase.generated.h"

class UTextBlock;
class UProgressBar;

UENUM(BlueprintType)
enum class EPTBActionType : uint8
{
	None UMETA(DisplayName = "None"),

	ActionA UMETA(DisplayName = "Action A"),
	ActionB UMETA(DisplayName = "Action B"),
	ActionC UMETA(DisplayName = "Action C"),
	ActionD UMETA(DisplayName = "Action D"),

	MoveLeft UMETA(DisplayName = "Move Left"),
	MoveRight UMETA(DisplayName = "Move Right"),
	MoveUp UMETA(DisplayName = "Move Up"),
	MoveDown UMETA(DisplayName = "Move Down"),

	Confirm UMETA(DisplayName = "Confirm"),
	Cancel UMETA(DisplayName = "Cancel")
};

UENUM(BlueprintType)
enum class EPTBJudgementType : uint8
{
	HighPerfect UMETA(DisplayName = "High Perfect"),
	Perfect UMETA(DisplayName = "Perfect"),
	Good UMETA(DisplayName = "Good"),
	Miss UMETA(DisplayName = "Miss")
};

USTRUCT(BlueprintType)
struct FPTBJudgementResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	int32 NoteId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	EPTBActionType ActionType = EPTBActionType::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	EPTBJudgementType JudgementType = EPTBJudgementType::Miss;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	float DeltaMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	int32 ScoreDelta = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Judgement")
	bool bBreaksCombo = false;
};

/**
 * 인게임 HUD 계열 위젯의 공통 베이스 클래스
 * 점수, 콤보, 판정, 진행도 표시 담당
 * 점수 계산/판정 계산은 담당하지 않음
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBHUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void ShowHud();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void HideHud();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void UpdateScore(int32 NewScore);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void UpdateCombo(int32 NewCombo);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void ShowJudgement(const FPTBJudgementResult& Result);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void UpdateProgress(float ProgressRatio);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> ComboText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> JudgementText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UProgressBar> ProgressBar = nullptr;
};
