// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBResultPopupWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBPopupButtonEvent);

/**
 * 미니게임 결과 팝업 위젯 베이스
 * InitPopup() 으로 결과 데이터를 받아 OnPopupDataSet() 을 BP에 전달
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

	// ── Getter (BP에서 OnPopupDataSet 구현 시 사용) ──────────────
	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetResultMessage() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	FText GetRankText() const;

	UFUNCTION(BlueprintPure, Category = "PTB|UI|Popup")
	int32 GetScore() const { return RoundResult.Score; }

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

private:
	FPTBRoundResult   RoundResult;
	FPTBRewardSummary RewardSummary;

	static FText GradeToText(EPTBGradeType Grade);
	static FText DifficultyToText(EPTBDifficulty Difficulty);
	static FText PlayModeToText(EPTBPlayMode PlayMode);
};