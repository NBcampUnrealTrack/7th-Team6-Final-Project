#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "MiniGames/Common/PTBMiniGameIntroData.h"
#include "PTBSceneWidgetBase.generated.h"

class APTBBaseMiniGame;
class UDataTable;

/**
 * 미니게임 씬(스토리/튜토리얼) 위젯 공통 베이스.
 *
 * 미니게임이 위젯을 생성 → SetupScene() → BP의 OnSceneSetup()에서 내용 표시 시작.
 * 끝나거나 스킵되면 FinishScene() 한 번만 호출하면 미니게임이 다음 단계로 진행한다.
 *
 * 리듬 키(Z/X/C/V/B 등)는 PlayerController → 미니게임 → OnSceneAction()으로 전달되므로
 * 튜토리얼 입력 대기나 "아무 리듬 키로 대사 넘기기"는 OnSceneAction에서 처리하면 된다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBSceneWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 미니게임에서 호출. 데이터 주입 후 OnSceneSetup() 실행 */
	void SetupScene(APTBBaseMiniGame* InOwnerMiniGame, EPTBMiniGameScenePhase InPhase, UDataTable* InTable);

	/** 미니게임에서 호출. 리듬 키 입력 전달 */
	void HandleSceneAction(EPTBActionType Action);

	/** 씬 종료. 정상 종료든 스킵이든 이걸 호출하면 미니게임이 다음 단계로 넘어감 (중복 호출 무시) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	void FinishScene(bool bSkipped);

	/** 스킵 버튼용 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	void SkipScene() { FinishScene(true); }

	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	EPTBMiniGameScenePhase GetScenePhase() const { return ScenePhase; }

	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	UDataTable* GetSceneTable() const { return SceneTable; }

	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	APTBBaseMiniGame* GetOwnerMiniGame() const { return OwnerMiniGame.Get(); }

	/** 스토리 대사 목록 (테이블 행 순서). Row 타입이 다르면 false */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	bool GetDialogueLines(TArray<FPTBDialogueLine>& OutLines) const;

	/** 튜토리얼 스텝 목록 (테이블 행 순서). Row 타입이 다르면 false */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	bool GetTutorialSteps(TArray<FPTBTutorialStep>& OutSteps) const;

	/** {ActionA}~{ActionE} 토큰을 현재 키 이름으로 치환 */
	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	FText FormatSceneText(const FText& SourceText) const;

protected:
	/** 데이터 주입 완료. 여기서 첫 대사/첫 스텝 표시 시작 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Scene")
	void OnSceneSetup();

	/** 리듬 키 입력 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Scene")
	void OnSceneAction(EPTBActionType Action);

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Scene")
	EPTBMiniGameScenePhase ScenePhase = EPTBMiniGameScenePhase::None;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Scene")
	TObjectPtr<UDataTable> SceneTable = nullptr;

	TWeakObjectPtr<APTBBaseMiniGame> OwnerMiniGame;

	bool bSceneFinished = false;
};
