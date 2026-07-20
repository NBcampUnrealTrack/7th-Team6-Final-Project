#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBSRPlateSpawner.generated.h"

class APTBSRMiniGame;
class APTBSRPlate;

/**
 * SR(초밥) 미니게임에서 APTBSRMiniGame::OnSushiPlateSpawn 델리게이트를 구독하여
 * 접시(BP_SR_Plate 등, PTBSRPlate 상속) 액터를 스폰하는 전용 스포너입니다.
 *
 * OnSushiPlateSpawn은 HandleNoteCue(노트가 다가올 때, 리드타임 포함)에서 브로드캐스트되므로,
 * 여기서 스폰되는 접시는 "이 노트를 담당하며 화면에 다가오는 접시"입니다.
 * 스폰 직후 AssignedNoteId를 세팅하고 MiniGame에 등록(RegisterActivePlate)해두면,
 * 나중에 판정이 성공하는 순간 MiniGame이 물리적 접촉 여부와 무관하게
 * "이 노트를 담당하던 바로 그 접시"를 찾아 완성시킬 수 있습니다.
 *
 * 기존 Blueprint(BP_SR_PlateSpawner)의 EventGraph 배선을 C++로 대체하기 위한 클래스입니다.
 * 이 클래스를 부모로 하는 Blueprint를 만들거나, 기존 BP_SR_PlateSpawner의
 * 부모 클래스(Reparent)를 이 클래스로 바꿔서 사용하세요.
 */
UCLASS()
class PARTTIMEBEAT_API APTBSRPlateSpawner : public AActor
{
	GENERATED_BODY()

public:
	APTBSRPlateSpawner();

	virtual void BeginPlay() override;

protected:
	/** 스폰할 접시 액터 클래스 (BP_SR_Plate 등, 디테일 패널에서 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Spawn")
	TSubclassOf<APTBSRPlate> PlateClass;

	/** 미니게임 액터를 아직 못 찾았을 때 재시도 간격(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|SR|Spawn")
	float BindRetryIntervalSeconds = 0.2f;

	/** OnSushiPlateSpawn 델리게이트가 브로드캐스트될 때 호출됩니다 */
	UFUNCTION()
	void HandleSushiPlateSpawn(int32 ToppingType, int32 NoteId);

private:
	/** 씬에서 APTBSRMiniGame을 찾아 델리게이트 바인딩을 시도. 실패 시 타이머로 재시도 */
	void TryBindMiniGame();

	/** 실제 접시 액터를 스폰하고 AssignedNoteId를 세팅한 뒤 MiniGame에 등록 */
	void SpawnPlate(int32 ToppingType, int32 NoteId);

	UPROPERTY()
	TObjectPtr<APTBSRMiniGame> BoundMiniGame = nullptr;

	FTimerHandle BindRetryTimerHandle;
};