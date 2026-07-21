#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "GameFramework/Actor.h"
#include "PTBLCJudgementDisplayActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;
class APTBBaseMiniGame;

/**
 * LC 미니게임에서 판정 결과를 전등 색상과 2D 이미지로 표시하는 Actor입니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBLCJudgementDisplayActor : public AActor
{
	GENERATED_BODY()

public:
	/** 기본 컴포넌트 구성 */
	APTBLCJudgementDisplayActor();

	/** 판정 결과 기준으로 전등 색상과 판정 이미지를 갱신 */
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Judgement Display")
	void ApplyJudgementResult(const FPTBJudgementResult& Result);

	/** 전등 색상과 판정 이미지를 기본 상태로 복원 */
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Judgement Display")
	void ResetJudgementDisplay();

protected:
	/** 게임 시작 처리 */
	virtual void BeginPlay() override;

	/** 게임 종료 처리 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 이동/스케일/회전 기준 루트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|LC|Judgement Display")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 전등 모양 스태틱 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|LC|Judgement Display")
	TObjectPtr<UStaticMeshComponent> LampMeshComponent;

	/** 판정에 따라 색이 바뀌는 광원 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|LC|Judgement Display")
	TObjectPtr<UPointLightComponent> JudgementLightComponent;

	/** 판정 이미지를 출력하는 평면 메시 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|LC|Judgement Display")
	TObjectPtr<UStaticMeshComponent> JudgementTextureComponent;

	/** 판정 이벤트를 받을 미니게임. 비워두면 GameMode의 ActiveMiniGame을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	TObjectPtr<APTBBaseMiniGame> SourceMiniGame;

	/** 기본 광원 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	FLinearColor DefaultLightColor = FLinearColor::White;

	/** 전등 메시 색상 파라미터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	FName LampColorParameterName = TEXT("Color");

	/** 판정 후 기본 상태로 돌아가기까지 걸리는 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ResetDelaySeconds = 0.5f;

	/** 판정별 광원 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	TMap<EPTBJudgementType, FLinearColor> JudgementLightColors;

	/** 기본 판정 이미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	TObjectPtr<UTexture2D> DefaultJudgementTexture;

	/** 판정별 표시 이미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	TMap<EPTBJudgementType, TObjectPtr<UTexture2D>> JudgementTextures;

	/** 판정 이미지 출력용 머터리얼. Texture Parameter를 가진 머터리얼을 지정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	TObjectPtr<UMaterialInterface> JudgementTextureMaterial;

	/** 판정 이미지 머터리얼 텍스처 파라미터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Judgement Display")
	FName JudgementTextureParameterName = TEXT("Texture");

private:
	UFUNCTION()
	void HandleMiniGameJudgement(FPTBJudgementResult Result);

	/** GameMode::OnGameStarted 수신. Retry로 미니게임이 교체될 때마다 재바인딩한다 */
	UFUNCTION()
	void HandleGameStarted();

	void TryBindSourceMiniGame();

	APTBBaseMiniGame* ResolveSourceMiniGame() const;

	FTimerHandle ResetTimerHandle;

	FTimerHandle BindRetryTimerHandle;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> LampDynamicMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> JudgementTextureDynamicMaterial;
};
