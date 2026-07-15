#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBSRPlate.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/**
 * SR(초밥) 미니게임의 이동하는 접시(컨베이어) 액터입니다.
 *
 * 기존 BP_SR_Plate의 EventGraph(Tick 이동 + Box Overlap 연출)를 C++로 대체합니다.
 * 이 클래스로 BP_SR_Plate의 부모(Reparent)를 바꿔서 사용하세요.
 *
 * 중요: 성공/실패 판정과 완성 비주얼(VFX, 완성 스시 모델 스폰)은
 *       더 이상 여기서 처리하지 않습니다. 전부 입력 판정 시점에
 *       APTBSRMiniGame::HandleJudgementResult → ResolveSuccessVisual 에서
 *       접시와의 물리적 접촉 여부와 완전히 무관하게 즉시 처리됩니다.
 *       이 액터는 순수하게 "컨베이어 이동 + 지나가는 토핑 정리"만 담당합니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBSRPlate : public AActor
{
	GENERATED_BODY()

public:
	APTBSRPlate();

	virtual void Tick(float DeltaTime) override;

	/** 이 접시가 담당하는 노트 ID. PTBSRPlateSpawner가 스폰 직후 설정해줍니다.
	 *  판정 성공 시 MiniGame이 이 NoteId로 "그 노트를 담당하던 바로 그 접시"를 찾아 완성시킵니다. */
	UPROPERTY(BlueprintReadWrite, Category = "PTB|SR|Plate")
	int32 AssignedNoteId = INDEX_NONE;

	/** 이 접시를 그 자리에 고정시켜 더 이상 컨베이어를 타고 움직이지 않게 합니다.
	 *  판정이 성공한 "그 즉시" 호출해야 합니다 — 완성 비주얼(VFX/모델 스폰)은
	 *  SuccessVisualDelaySeconds만큼 나중에 실행되는데, 그동안 접시가 계속 움직여버리면
	 *  판정 당시 위치가 아닌 엉뚱한(더 진행된) 위치에서 완성 비주얼이 나타나게 됩니다. */
	UFUNCTION(BlueprintCallable, Category = "PTB|SR|Plate")
	void FreezeMovement() { bMovementFrozen = true; }

	/** 접시 컨베이어 이동 속도 (로컬 Y축 기준). PTBSRPlateSpawner가 스폰 직후
	 *  APTBSRMiniGame::GetPlateMoveSpeed() 값으로 세팅해줍니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate")
	float MoveSpeed = -100.0f;

protected:
	virtual void BeginPlay() override;

	/** 겹침 감지용 콜리전. 기존 BP_SR_Plate의 "Box" 컴포넌트와 이름이 같아야
	 *  Reparent 시 기존에 붙여둔 콜리전 크기/오프셋 등이 그대로 유지됩니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|SR|Plate")
	TObjectPtr<UBoxComponent> Box;

	/** 겹친 액터가 "토핑"인지 식별하기 위해 확인할 프로퍼티 이름
	 *  (BP_SR_Topping 쪽 NoteId 변수명과 일치해야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate|Reflection")
	FName ToppingNoteIdPropertyName = TEXT("NoteId");

	/** Box 콜리전 Overlap 콜백 — 순수 연출 정리용 (판정에는 영향 없음) */
	UFUNCTION()
	void HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bMovementFrozen = false;
};