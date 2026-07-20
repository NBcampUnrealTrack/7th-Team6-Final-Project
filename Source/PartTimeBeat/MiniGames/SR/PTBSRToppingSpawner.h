#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBSRToppingSpawner.generated.h"

class APTBSRMiniGame;

/** SR 미니게임의 OnToppingDrop 델리게이트를 구독해 토핑 액터를 스폰하는 스포너 */
UCLASS()
class PARTTIMEBEAT_API APTBSRToppingSpawner : public AActor
{
	GENERATED_BODY()

public:
	APTBSRToppingSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 스폰할 토핑 액터 클래스 (BP_SR_Topping 등, 디테일 패널에서 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Spawn")
	TSubclassOf<AActor> ToppingClass;

	/** BP_SR_Topping 쪽 Expose-on-Spawn 변수 이름과 반드시 일치해야 합니다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Spawn")
	FName ToppingIdPropertyName = TEXT("ToppingID");

	/** 미니게임 액터를 아직 못 찾았을 때 재시도 간격(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Spawn")
	float BindRetryIntervalSeconds = 0.2f;

	/** OnSushiPlateSpawn 델리게이트가 브로드캐스트될 때 호출됩니다 */
	UFUNCTION()
	void HandleSushiPlateSpawn(int32 AssignedTopping, int32 NoteId);

	/** MiniGame의 OnSushiSuccessDelegate가 브로드캐스트될 때 호출됩니다 — 성공한 경우에만
	 *  "남은 토핑" 카운트(count)를 1 감소시킵니다. */
	UFUNCTION()
	void HandleSushiSuccess(int32 NoteId, EPTBJudgementType JudgementType);

private:
	/** GameMode::OnGameStarted 수신. Retry로 미니게임이 교체될 때마다 재바인딩한다 */
	UFUNCTION()
	void HandleGameStarted();

	/** 씬에서 APTBSRMiniGame을 찾아 델리게이트 바인딩을 시도. 실패 시 타이머로 재시도 */
	void TryBindMiniGame();

	/** 실제 토핑 액터를 스폰하고 ToppingID 프로퍼티를 세팅 */
	void SpawnTopping(int32 AssignedTopping, int32 NoteId);

	UPROPERTY()
	TObjectPtr<APTBSRMiniGame> BoundMiniGame = nullptr;

	FTimerHandle BindRetryTimerHandle;
};