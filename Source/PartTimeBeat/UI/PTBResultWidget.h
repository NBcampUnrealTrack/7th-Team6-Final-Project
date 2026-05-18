// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBResultWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;

UENUM(BlueprintType)
enum class EPTBDifficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Standard UMETA(DisplayName = "Standard"),
	Insane UMETA(DisplayName = "Insane")
};

UENUM(BlueprintType)
enum class EPTBGradeType : uint8
{
	F UMETA(DisplayName = "F"),
	D UMETA(DisplayName = "D"),
	C UMETA(DisplayName = "C"),
	B UMETA(DisplayName = "B"),
	A UMETA(DisplayName = "A"),
	S UMETA(DisplayName = "S")
};

USTRUCT(BlueprintType)
struct FPTBMiniGameResultPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	FName PayloadType = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	TMap<FName, float> FloatValues;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	TMap<FName, int32> IntValues;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	TMap<FName, FString> StringValues;
};

USTRUCT(BlueprintType)
struct FPTBRoundResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	FGuid ProfileId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	FName MiniGameId = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	EPTBDifficulty Difficulty = EPTBDifficulty::Standard;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 Score = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 HighPerfectCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 PerfectCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 GoodCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 MissCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 MaxCombo = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	float AccuracyRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	EPTBGradeType Grade = EPTBGradeType::F;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 StarCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	int32 EarnedMoney = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	bool bIsNewHighScore = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Result")
	FPTBMiniGameResultPayload MiniGamePayload;
};

USTRUCT(BlueprintType)
struct FPTBRewardSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	int32 EarnedMoney = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	int32 EarnedStars = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	int32 TotalMoney = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	int32 TotalStars = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	TArray<FName> NewlyUnlockedMiniGames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	TArray<FName> NewlyUnlockedStories;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Reward")
	TArray<FName> NewlyUnlockedCostumes;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBResultWidgetEvent);

/**
 * 스테이지 종료 후 결과와 보상을 표시하는 위젯
 * 결과/보상 표시만 담당
 * 재시도/다음 진행은 외부에 요청만 전달
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnRetryRequested;

	UPROPERTY(BlueprintAssignable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnNextRequested;
	
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void SetRoundResult(const FPTBRoundResult& RoundResult);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void ShowRewardSummary(const FPTBRewardSummary& RewardSummary);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void OnRetryClicked();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void OnNextClicked();

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, Category = "PTB|UI")
	FPTBRoundResult CurrentRoundResult;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> RankText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UPanelWidget> RewardPanel = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonRetry = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;
};
