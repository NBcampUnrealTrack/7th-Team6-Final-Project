// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/PTBStructEnums.h"
#include "FSWidget.generated.h"

class UImage;
class UTextBlock;
class UWidget;
class UCanvasPanel;
class UProgressBar;
class APTBFSMiniGame;

/** 화면에 날아오는 노트 화살표 1개 (풀링) */
struct FFSActiveArrow
{
	int32 PoolIndex = INDEX_NONE;
	int32 Lane = 0;            // 0=왼쪽(ArrowLeft), 1=가운데(ArrowUp), 2=오른쪽(ArrowRight)
	float TargetTimeMs = 0.0f;
	float CueLeadTimeMs = 1.0f;
};

/**
 * FS 미니게임 HUD.
 *
 * 위치는 전부 Designer 배치 기준 (해상도·앵커와 무관하게 동작):
 *  - 노트 시작점  : ArrowLeft / ArrowUp / ArrowRight 를 배치한 위치 (이 3개는 "템플릿"으로 숨겨지고, 노트마다 복제본이 생성됨)
 *  - 노트 도착점  : Image_left / Image_up / Image_right (회색 판정 화살표) 의 위치
 *  - 키 안내 칸   : KeyGuideA / B / C 를 배치한 위치 = 왼/가운데/오른쪽 칸 (키 셔플 시 이 3칸 사이를 이동)
 *  - 물고기 거리 마커 : FishdisTance 가 DistanceGauge 의 위(잡힘)~아래(멀어짐) 사이를 이동
 *                      (실제 화면 위치 기준이라 어떤 컨테이너·계층·앵커에 넣어도 동작. 이동은 Render Translation으로 처리)
 */
UCLASS()
class PARTTIMEBEAT_API UFSWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ── 키 안내 UI (전부 선택사항, 이름이 같으면 자동 연결) ──
	// KeyGuideA/B/C   : A/B/C 키 안내 묶음 (Canvas Panel 바로 아래 자식). Designer에 놓은 위치가 왼/가운데/오른쪽 칸이 됨
	// KeyGuideTextA/B/C : 키 이름 텍스트 (설정한 키 이름이 자동으로 들어감)
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideA = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideB = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideC = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextA = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextB = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextC = nullptr;

	/** 교체 중 곡선 이동 높이 (오른쪽으로 가는 키는 위로, 왼쪽으로 가는 키는 아래로 돌아서 겹치지 않음) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|KeyGuide")
	float KeyGuideSwapArcHeight = 40.0f;

	/** 키 배치 교체 연출이 시작될 때 (BP: 효과음, 깜빡임 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "FS|KeyGuide")
	void OnKeyGuideSwapStarted();

	/** 설정된 키 이름으로 텍스트 갱신 */
	UFUNCTION(BlueprintCallable, Category = "FS|KeyGuide")
	void RefreshKeyGuideLabels();

	void UpdateKeyGuide(float VisualTimeMs, const FVector2D& CanvasSize);

	bool bKeyGuideWasSwapping = false;

	// ── 키 입력 피드백 (누르는 동안 KeyGuide 강조, 떼면 복귀) ──
	// KeyGuidePressA/B/C : 눌림 표시용 위젯(테두리 이미지 등). KeyGuideA/B/C 안에 넣고 이름을 맞추면 자동 연결. 없어도 됨

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuidePressA = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuidePressB = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuidePressC = nullptr;

	/** 누르는 동안 KeyGuide 크기 (1.0 = 변화 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|KeyGuide", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float KeyGuidePressedScale = 1.15f;

	/** 기본 처리(크기·강조 위젯) 후 호출. BP에서 색 변경 등 추가 연출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FS|KeyGuide")
	void OnKeyGuidePressedChanged(EPTBActionType Action, bool bPressed);

	UFUNCTION()
	void HandleRhythmKeyStateChanged(EPTBActionType Action, bool bPressed);

	void ResetKeyGuidePressed();

	// ── 노트 화살표 ──
	// ArrowLeft/Up/Right = 템플릿(시작 위치·크기·이미지·회전). 실제 노트는 복제본이 날아감 → 노트 간격이 짧아도 여러 개 동시에 보임
	// 템플릿의 Brush를 BP에서 바꾸면(콤보 피버 화살표 등) 날아가는 중인 복제본에도 반영됨

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowLeft;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowRight;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowUp;

	/** 노트 도착 지점 (회색 판정 화살표). 없으면 KeyGuide 칸 위치를 사용 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* Image_left = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* Image_up = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* Image_right = nullptr;

	/** 노트 화살표 복제본이 화면에 떠 있는 동안 생성된 것 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FS|Arrow")
	void OnArrowSpawned(UImage* ArrowWidget, EPTBActionType VisualAction);

	// ── 물고기 거리 마커 ──

	/**
	 * 켜면 FishdisTance 위치를 C++에서 DistanceGauge 기준으로 갱신 (BP의 SetPosition 하드코딩 대체).
	 * 마커의 Designer 배치 위치(X 포함)는 그대로 두고, 세로 위치만 Render Translation으로 게이지 위~아래에 맞춘다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|Distance")
	bool bAutoPlaceFishMarker = true;

	/** 게이지 위/아래 끝에서 안쪽으로 띄울 여백(px) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|Distance")
	float FishMarkerEdgePadding = 0.0f;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UProgressBar* DistanceGauge = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* FishdisTance = nullptr;

	UPROPERTY()
	APTBFSMiniGame* FSMiniGame;

public:
	void InitializeWidget(APTBFSMiniGame* InGame);

	UFUNCTION()
	void OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs);

private:
	// ── Designer 배치 → 캔버스 좌표 변환 (앵커/정렬 반영) ──

	/** 슬롯 배치 데이터(앵커·오프셋·정렬)로 캔버스 로컬 사각형 계산 */
	static bool ComputeRectFromLayout(const FAnchorData& Layout, const FVector2D& CanvasSize, FVector2D& OutTopLeft, FVector2D& OutSize);

	/** 위젯의 현재 슬롯 기준 중심점 (캔버스 로컬) */
	static bool GetWidgetCenter(const UWidget* Widget, const FVector2D& CanvasSize, FVector2D& OutCenter, FVector2D* OutSize = nullptr);

	/** 위젯 중심이 CanvasCenter에 오도록 슬롯 Position 설정 (앵커 유지). 늘이기(Stretch) 앵커면 false */
	static bool SetWidgetCenter(UWidget* Widget, const FVector2D& CanvasSize, const FVector2D& CanvasCenter);

	/** Designer에 놓인 KeyGuideA/B/C 원래 배치 (칸 위치 계산용). NativeConstruct에서 저장 */
	FAnchorData KeyGuideDesignLayout[3];
	bool bHasKeyGuideDesignLayout[3] = { false, false, false };

	FVector2D LastCanvasSize = FVector2D::ZeroVector;

	UImage* GetArrowTemplate(int32 Lane) const;
	UWidget* GetArrowTarget(int32 Lane) const;
	bool GetLaneCenter(int32 Lane, const FVector2D& CanvasSize, FVector2D& OutCenter) const;

	int32 AcquireArrowFromPool(int32 Lane);
	void ReleaseArrow(int32 PoolIndex);
	void UpdateArrows(float CurrentTimeMs, const FVector2D& CanvasSize);
	void UpdateFishMarker();

	UPROPERTY()
	TArray<TObjectPtr<UImage>> ArrowPool;

	TArray<bool> ArrowPoolInUse;
	TArray<FFSActiveArrow> ActiveArrows;

	/** 캔버스 크기를 알기 전(첫 Tick 전)에 들어온 노트 */
	TArray<FFSActiveArrow> PendingSpawnArrows;
};
