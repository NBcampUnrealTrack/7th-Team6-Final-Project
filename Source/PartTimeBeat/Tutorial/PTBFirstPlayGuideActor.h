#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBFirstPlayGuideActor.generated.h"

class UStaticMeshComponent;
class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPTBFirstPlayGuideWidget;

UENUM(BlueprintType)
enum class EPTBGuideArrowPlacement : uint8
{
	/** 대상 앞(플레이어 쪽) 눈높이에 떠서 대상을 가리킴 — 높은 기둥에 추천 */
	InFrontOfTarget UMETA(DisplayName = "대상 앞 (눈높이)"),
	/** 대상 꼭대기 위에서 아래를 가리킴 */
	AboveTarget     UMETA(DisplayName = "대상 위"),
};

/**
 * 첫 플레이 안내 (허브 맵에 1개 배치).
 *
 * 활성 프로필에 미니게임 플레이 기록이 하나도 없을 때만:
 *  ① 화면 상단 안내 문구 (천천히 깜빡임, 기둥 근처에 가면 숨김)
 *  ② 안내 대상(TargetActor) 위에 둥실거리는 3D 화살표
 *  ③ (선택) 안내 대상에 강조 머티리얼(OverlayMaterial) 덧씌우기
 * 어떤 미니게임이든 한 판 결과 화면까지 가면(성공·실패 무관) 다시는 표시 안 함.
 *
 * 사용법: 맵에 이 액터를 놓고 Details의 Target Actor에 CH 보라색 기둥을 지정. 액터 위치는 아무 데나 상관없음.
 */
UCLASS()
class PARTTIMEBEAT_API APTBFirstPlayGuideActor : public AActor
{
	GENERATED_BODY()

public:
	APTBFirstPlayGuideActor();

	virtual void Tick(float DeltaSeconds) override;

	/** 안내할 대상 (CH 보라색 기둥). 맵에 놓인 액터를 스포이드로 지정 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	TObjectPtr<AActor> TargetActor;

	// ── ① 화면 문구 ───────────────────────────────────────────────

	/** 안내 문구 위젯. 비워두면 기본 C++ 레이아웃 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Hint")
	TSubclassOf<UPTBFirstPlayGuideWidget> HintWidgetClass;

	/** 플레이어가 대상에 이만큼(cm) 가까워지면 화면 문구를 숨김 (상호작용 키 안내와 겹치지 않게) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Hint")
	float HideHintWithinDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Hint")
	int32 HintZOrder = 5;

	// ── ② 3D 화살표 ───────────────────────────────────────────────

	/** 화살표 (기본: 엔진 원뿔을 거꾸로 세운 것). 메시는 컴포넌트 Details에서 바꿔도 됨 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|FirstPlayGuide|Arrow")
	TObjectPtr<UStaticMeshComponent> ArrowMesh;

	/** 화살표 위치 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	EPTBGuideArrowPlacement ArrowPlacement = EPTBGuideArrowPlacement::InFrontOfTarget;

	/** [대상 앞] 대상 바닥에서 화살표까지 높이(cm). 캐릭터 눈높이 근처 추천 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	float ArrowHeightFromTargetBottom = 160.f;

	/** [대상 앞] 대상 겉면에서 플레이어 쪽으로 띄울 거리(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	float ArrowDistanceFromTarget = 90.f;

	/** [대상 위] 대상 꼭대기에서 화살표까지 높이(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	float ArrowHeightAboveTarget = 80.f;

	/** 움직이는 폭(cm) / 주기(초). [대상 위]는 위아래, [대상 앞]은 대상 쪽으로 콕콕 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	float ArrowBobAmplitude = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow", meta = (ClampMin = "0.1"))
	float ArrowBobPeriodSec = 1.2f;

	/** 제자리 회전 속도(도/초). 0이면 회전 안 함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	float ArrowSpinDegPerSec = 90.f;

	/** 기본 원뿔 머티리얼에 넣을 색 (메시를 바꾸면 무시될 수 있음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Arrow")
	FLinearColor ArrowColor = FLinearColor(1.f, 0.85f, 0.1f, 1.f);

	// ── ③ 대상 강조 ───────────────────────────────────────────────

	/**
	 * 대상의 모든 메시에 덧씌울 머티리얼 (Overlay Material). 비워두면 강조 안 함.
	 * 반투명·Unlit에 Time 기반으로 밝기가 오르내리는 머티리얼을 추천.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Highlight")
	TObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

	/**
	 * 강조 색. 머티리얼에 이름이 HighlightColor인 Vector Parameter가 있으면 이 색이 들어감.
	 * 보라색 기둥과 대비: 노랑/금색 추천
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Highlight")
	FLinearColor HighlightColor = FLinearColor(1.f, 0.8f, 0.05f, 1.f);

	// ── ④ 화면 밖 방향 표시 ───────────────────────────────────────

	/** 대상이 화면 밖(뒤쪽 포함)이면 화면 가장자리에 방향 화살표 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Offscreen")
	bool bShowOffscreenIndicator = true;

	/** 화면 가장자리에서 이만큼 안쪽(0~0.5, 화면 비율)까지 들어와야 "화면 안"으로 봄 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide|Offscreen", meta = (ClampMin = "0", ClampMax = "0.4"))
	float OnScreenInnerMargin = 0.05f;

	/** 안내 표시 여부를 지금 다시 판단 */
	UFUNCTION(BlueprintCallable, Category = "PTB|FirstPlayGuide")
	void RefreshGuide();

	/** 활성 프로필에 플레이 기록이 없어서 안내가 필요한지 */
	UFUNCTION(BlueprintPure, Category = "PTB|FirstPlayGuide")
	bool ShouldShowGuide() const;

	UFUNCTION(BlueprintPure, Category = "PTB|FirstPlayGuide")
	bool IsGuideActive() const { return bGuideActive; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetGuideActive(bool bActive);
	void ApplyHighlight(bool bEnable);
	void UpdateArrow(float DeltaSeconds);
	void UpdateHintVisibility();
	void UpdateOffscreenIndicator();
	FVector GetTargetTopLocation() const;

	UPROPERTY(Transient)
	TObjectPtr<UPTBFirstPlayGuideWidget> HintWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ArrowMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HighlightMaterial;

	/** 강조 전 원래 Overlay Material (끌 때 복원) */
	TMap<TWeakObjectPtr<UMeshComponent>, TObjectPtr<UMaterialInterface>> SavedOverlayMaterials;

	bool bGuideActive = false;
	float AnimTime = 0.f;
	float RecheckTimer = 0.f;
};
