#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBSRPlate.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UNiagaraSystem;
class APTBSRMiniGame;

UCLASS()
class PARTTIMEBEAT_API APTBSRPlate : public AActor
{
	GENERATED_BODY()

public:
	APTBSRPlate();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/** 겹침 감지용 콜리전. 기존 BP_SR_Plate의 "Box" 컴포넌트와 이름이 같아야
	 *  Reparent 시 기존에 붙여둔 콜리전 크기/메시 등이 그대로 유지됩니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|SR|Plate")
	TObjectPtr<UBoxComponent> Box;

	/** 접시 컨베이어 이동 속도 (기존 BP의 MoveSpeed 변수 대체, 로컬 Y축 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate")
	float MoveSpeed = -100.0f;

	/** 완성 판정(Miss가 아닐 때) 시 재생할 VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate|VFX")
	TObjectPtr<UNiagaraSystem> CompletionVFX;

	/** BP_SR_Topping 쪽 NoteId 정수 변수명 (리플렉션으로 읽음, 이름이 정확히 일치해야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate|Reflection")
	FName ToppingNoteIdPropertyName = TEXT("NoteId");

	/** BP_SR_Topping 쪽 완성 스시 클래스 변수명 (리플렉션으로 읽음, 이름이 정확히 일치해야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Plate|Reflection")
	FName ToppingSushiClassPropertyName = TEXT("MySushiClass");

	/** Box 콜리전 Overlap 콜백 */
	UFUNCTION()
	void HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** 겹친 액터(토핑)에서 NoteId 정수 프로퍼티를 리플렉션으로 읽음 */
	bool TryGetNoteIdFromTopping(AActor* ToppingActor, int32& OutNoteId) const;

	/** 겹친 액터(토핑)에서 완성될 스시 클래스(class 프로퍼티)를 리플렉션으로 읽음 */
	TSubclassOf<AActor> GetSushiClassFromTopping(AActor* ToppingActor) const;

	/** 씬에서 APTBSRMiniGame을 찾음 */
	APTBSRMiniGame* FindMiniGame() const;

	/** 완성 VFX + 완성된 스시 모델 액터 스폰 */
	void PlayCompletionEffects(TSubclassOf<AActor> SushiClass);
};