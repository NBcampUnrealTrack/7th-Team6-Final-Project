// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBResultPopupWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBPopupButtonEvent);

/**
 * 미니게임 결과 팝업 위젯 베이스
 * InitPopup() 으로 결과 데이터를 받으면 C++이 텍스트를 직접 채우고, 그 다음 OnPopupDataSet() 을 BP에 전달(추가 연출용)
 *
 * Designer 위젯 (이름·타입 일치 시 자동 연결, 모두 선택):
 *  Text_MiniGameName, Text_PlayMode, Text_Difficulty, Text_Rank, Text_ResultMessage,
 *  Text_Score, Text_EarnedMoney, Text_TotalMoney (Text Block)
 *  Btn_Retry (재도전: GameFlow RetryLastGame), Btn_Map (맵으로: GameFlow ReturnToMiniGameSelect) (Button)
 * 문구 형식은 Class Defaults의 "PTB|UI|Popup|Format" 항목에서 수정
 * 결과 문구/등급/점수/재화/미니게임 정보를 Getter로 제공
 * 재도전/맵으로 버튼 클릭은 델리게이트로 외부에 전달
 *
 * 확장 방법:
 *   ResultMessageMap  — Grade별 표시 문구를 BP Class Defaults에서 편집
 *   MiniGameDisplayNameMap — MiniGameId → 표시 이름을 BP Class Defaults에서 편집
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBResultPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|Popup")
	void InitPopup(const FPTBRoundResult& InResult, const FPTBRewardSummary& InReward);

	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|Popup")
	void OnPopupDataSet();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PTB|UI|Event")
	FPTBPopupButtonEvent OnRetryClicked;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PTB|UI|Event")
	FPTBPopupButtonEvent OnMapClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup")
	TMap<EPTBGradeType, FText> ResultMessageMap;

	// ── 미니게임 표시 이름 매핑 ──────────────────────────────────
	// BP Class Defaults에서 MiniGameId → 화면 표시 이름 설정
	// 예) "MiniGame_JJ" → "편의점 알바"
	// Map에 없으면 MiniGameId 문자열 그대로 표시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup")
	TMap<FName, FText> MiniGameDisplayNameMap;

	// ── 표시 형식 ({} 안 이름은 그대로 두고 앞뒤 문구만 수정) ──────────
	/** {Score} = 965,239 / {Percent} = 96.52% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText ScoreFormat = NSLOCTEXT("PTBResultPopup", "ScoreFormat", "점수 : {Score} ({Percent})");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText RankFormat = NSLOCTEXT("PTBResultPopup", "RankFormat", "랭크 : {Rank}");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText EarnedMoneyFormat = NSLOCTEXT("PTBResultPopup", "EarnedMoneyFormat", "획득 알바비 : {EarnedMoney} 골드");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText TotalMoneyFormat = NSLOCTEXT("PTBResultPopup", "TotalMoneyFormat", "소지금 : {TotalMoney} 골드");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText DifficultyFormat = NSLOCTEXT("PTBResultPopup", "DifficultyFormat", "난이도 : {Difficulty}");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup|Format")
	FText PlayModeFormat = NSLOCTEXT("PTBResultPopup", "PlayModeFormat", "{PlayMode}");

	/** 재도전/맵으로 버튼을 C++이 처리 (BP의 버튼 OnClicked 이벤트는 지워야 두 번 실행되지 않음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|Popup")
	bool bHandleButtonsInCode = true;

	// ── Getter (BP에서 OnPopupDataSet 구현 시 사용) ──────────────
	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetResultMessage() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetRankText() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	int32 GetScore() const { return RoundResult.Score; }

	/** 점수 % 숫자 (예: 96.52) */
	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	float GetScorePercent() const { return RoundResult.ScorePercent; }

	/** "96.52%" */
	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetScorePercentText() const;

	/** "965,239 (96.52%)" */
	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetScoreWithPercentText() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	int32 GetEarnedMoney() const { return RewardSummary.EarnedMoney; }

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	int32 GetTotalMoney() const { return RewardSummary.TotalMoney; }

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetMiniGameName() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetDifficultyText() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetPlayModeText() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_MiniGameName;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_PlayMode;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Difficulty;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Rank;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_ResultMessage;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Score;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_EarnedMoney;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_TotalMoney;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> Btn_Retry;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> Btn_Map;

	/** 결과 데이터로 텍스트 전부 채우기 */
	void RefreshTexts();

	UFUNCTION() void HandleRetryButtonClicked();
	UFUNCTION() void HandleMapButtonClicked();

private:
	FPTBRoundResult   RoundResult;
	FPTBRewardSummary RewardSummary;

	static FText GradeToText(EPTBGradeType Grade);
	static FText DifficultyToText(EPTBDifficulty Difficulty);
	static FText PlayModeToText(EPTBPlayMode PlayMode);
};