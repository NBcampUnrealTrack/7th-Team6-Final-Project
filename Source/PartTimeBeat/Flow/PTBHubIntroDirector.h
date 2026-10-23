#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBHubIntroDirector.generated.h"

class UDataTable;
class UPTBSceneWidgetBase;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBHubIntroFinishedDelegate);

/**
 * 허브 맵(L_GameMap) 첫 진입 연출 관리.
 * 레벨에 하나 배치 → 프로필별로 처음 한 번만:
 *   1) 메인 오프닝 스토리 (미니게임 스토리와 같은 대사 DataTable / 스토리 위젯)
 *   2) 기본 조작 안내 (튜토리얼 DataTable / 튜토리얼 위젯, 설명 스텝만)
 * 본 기록은 프로필 SeenSceneVersions에 저장 (Version을 올리면 다시 재생). 스킵해도 본 것으로 처리.
 */
UCLASS()
class PARTTIMEBEAT_API APTBHubIntroDirector : public AActor
{
	GENERATED_BODY()

public:
	APTBHubIntroDirector();

	// ── 오프닝 스토리 ──
	/** 대사 테이블 (Row: PTBDialogueLine) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Opening")
	TObjectPtr<UDataTable> OpeningStoryTable;

	/** 올리면 이미 본 프로필도 다시 재생 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Opening", meta = (ClampMin = "1"))
	int32 OpeningStoryVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Opening")
	FName OpeningSceneKey = TEXT("Main.Opening");

	/** 비우면 GameInstance의 StoryWidgetClass 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Opening")
	TSubclassOf<UPTBSceneWidgetBase> StoryWidgetClass;

	// ── 기본 조작 안내 ──
	/** 안내 테이블 (Row: PTBTutorialStep). Instruction/DemoImage만 사용, 연습 스텝은 자동으로 건너뜀 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Guide")
	TObjectPtr<UDataTable> GuideTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Guide", meta = (ClampMin = "1"))
	int32 GuideVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Guide")
	FName GuideSceneKey = TEXT("Main.Guide");

	/** 비우면 GameInstance의 TutorialWidgetClass 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Guide")
	TSubclassOf<UPTBSceneWidgetBase> GuideWidgetClass;

	// ── 허브 HUD ──
	/** 프로필 HUD (WBP_Hub_Profile 등, 부모: PTBHubProfileWidget). 맵 시작 시 생성, 오프닝/안내 중에는 숨김 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|HUD")
	TSubclassOf<UUserWidget> ProfileHUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|HUD")
	int32 ProfileHUDZOrder = 0;

	UFUNCTION(BlueprintPure, Category = "PTB|Hub|HUD")
	UUserWidget* GetProfileHUD() const { return ProfileHUD; }

	// ── 공통 ──
	/** 맵 로드 후 시작까지 대기(초). 페이드/로딩 화면과 겹치지 않게 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub")
	float StartDelay = 0.5f;

	/** 위젯 ZOrder (프로필 HUD 등 위에 표시) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub")
	int32 WidgetZOrder = 50;

	/** 테스트용: 본 기록과 관계없이 매번 재생 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Hub|Debug")
	bool bAlwaysPlay = false;

	/** 오프닝/안내가 모두 끝났거나 볼 게 없을 때 (레벨 BP에서 HUD 표시 등) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|Hub")
	FPTBHubIntroFinishedDelegate OnHubIntroFinished;

	/** 다시보기 메뉴용: 오프닝 스토리 강제 재생 (안내는 이미 봤으면 생략) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	void ReplayOpening();

	/** 지금 오프닝/안내 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	bool IsPlaying() const { return ActiveWidget != nullptr; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	enum class EStage : uint8 { None, Opening, Guide, Done };

	void Begin();
	void AdvanceStage();
	bool ShouldPlay(FName SceneKey, int32 Version) const;
	bool ShowScene(TSubclassOf<UPTBSceneWidgetBase> WidgetClass, UDataTable* Table, bool bTutorialPhase);
	void FinishAll();
	void EnterUIMode(UUserWidget* FocusWidget);
	void RestoreInputMode();

	UFUNCTION()
	void HandleSceneFinished(UPTBSceneWidgetBase* SceneWidget, bool bSkipped);

	UPROPERTY(Transient)
	TObjectPtr<UPTBSceneWidgetBase> ActiveWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ProfileHUD;

	void SetProfileHUDVisible(bool bVisible);

	EStage Stage = EStage::None;
	bool bForceOpening = false;
	bool bInputModeChanged = false;
	bool bPrevShowMouseCursor = false;
	FTimerHandle StartTimerHandle;
};
