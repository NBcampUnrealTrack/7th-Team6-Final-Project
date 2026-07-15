#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Engine/DataTable.h"
#include "PTBSRMiniGame.generated.h"

USTRUCT(BlueprintType)
struct FPTBToppingRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	FName SR_ToppingName; // 토핑 이름 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	class UStaticMesh* SR_ToppingMesh; // 토핑에 입혀질 3D 메시 에셋

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	float SR_GravityScale; // 토핑이 떨어지는 중력 속도 조절 수치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	class UMaterialInterface* SR_ToppingMaterial; // 토핑의 색상/질감 머티리얼 에셋

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	class UTexture2D* SR_ToppingIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sushi")
	TSubclassOf<AActor> SR_SushiBP;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnSushiPlateSignature, int32, ToppingType, int32, NoteId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDropToppingSignature, int32, ToppingType, int32, NoteId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSushiSuccessSignature, int32, NoteId, EPTBJudgementType, JudgementType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSushiMissSignature, int32, NoteId);

UCLASS()
class PARTTIMEBEAT_API APTBSRMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	APTBSRMiniGame();
	virtual void BeginPlay() override;
	// 블루프린트 스포너들이 이벤트 노드로 꺼내서 쓸 수 있도록 Assignable 설정
	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi")
	FOnSpawnSushiPlateSignature OnSushiPlateSpawn;

	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi")
	FOnDropToppingSignature OnToppingDrop;  // 토핑용 

	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi")
	FOnSushiSuccessSignature OnSushiSuccessDelegate;
	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi")
	FOnSushiMissSignature OnSushiMissDelegate;
	UFUNCTION(BlueprintCallable, Category = "PTB|Sushi")
	void RegisterActivePlate(int32 NoteId, AActor* PlateActor);

	/** ToppingSpawner가 토핑을 스폰한 직후 호출해서, "이 NoteId는 지금 이 토핑 액터가 담당 중"임을 등록합니다.
	 *  판정이 성공하는 즉시(물리적으로 접시에 닿았는지와 무관하게) 이 토핑을 찾아 완성 처리하는 데 사용됩니다. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Sushi")
	void RegisterActiveTopping(int32 NoteId, AActor* ToppingActor);

	/** 접시 컨베이어 이동 속도. PTBSRPlateSpawner가 접시를 스폰할 때 이 값을 그대로 적용합니다. */
	UFUNCTION(BlueprintPure, Category = "PTB|Sushi")
	float GetPlateMoveSpeed() const { return PlateMoveSpeed; }

	/** PTBSRPlate가 토핑과 물리적으로 겹쳤을 때(=바닥/컨베이어에 닿았을 때) 호출합니다.
	 *  이 NoteId가 "판정은 이미 성공했지만 아직 완성 비주얼을 안 띄운" 대기 상태라면,
	 *  바로 지금(=닿는 순간) 완성 처리를 하고 true를 반환합니다. 대기 상태가 아니었다면
	 *  (이미 처리됐거나 애초에 Miss였다면) false를 반환하니, 호출한 쪽에서 토핑을 그냥
	 *  정리(파괴)하면 됩니다. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Sushi")
	bool NotifyToppingReachedConveyor(int32 NoteId);

	// 블루프린트에서 노트 번호만 주면 셔플된 대기열에서 몇 번 토핑인지 안전하게 꺼내줍니다. (Pure 함수라 실행선 불필요)
	UFUNCTION(BlueprintPure, Category = "PTB|Sushi")
	int32 GetToppingTypeFromQueue(int32 NoteId) const;
	UFUNCTION(BlueprintPure, Category = "PTB|Sushi")
	FPTBToppingRow GetToppingData(int32 ToppingType) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi|UI")
	class UUserWidget* WBP_SR_Preview;
	UFUNCTION(BlueprintCallable, Category = "PTB|Sushi|UI")
	void RefreshPreviewUI();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PTB|Sushi")
	EPTBJudgementType GetJudgementForNote(int32 NoteId) const
	{
		if (const EPTBJudgementType* Found = NoteJudgementResults.Find(NoteId))
		{
			return *Found;
		}
		return EPTBJudgementType::Miss; // 기록이 없으면 기본값 Miss로 취급
	}
protected:
	virtual void HandleRhythmInput(EPTBActionType Action, float TimeMs = -1.0f) override;

	UPROPERTY()
	TMap<int32, EPTBJudgementType> NoteJudgementResults;
	// 연결할 기획 데이터 테이블 에셋 (DT_SR_ToppingList)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Setup")
	class UDataTable* ToppingDataTable;
	// 접시 전진 속도 (기존 BP_SR_Plate의 Move Speed 수치 통합)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Setup")
	float PlateMoveSpeed;
	// 접시/토핑이 생성될 안전한 Z축 높이 (지하 바닥 뚫림 버그 해결용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Setup")
	float SpawnerZHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi")
	int32 SuccessSushiCount = 0;

	/** 판정 성공 시 즉시 재생할 완성 VFX (물리적 접시 접촉과 무관하게 재생됩니다) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi|VFX")
	class UNiagaraSystem* CompletionVFX;

	/** 완성된 스시가 나타날 대체(fallback) 위치입니다. 레벨에 배치해둔 참조용 액터(빈 Actor나
	 *  TargetPoint 등)를 여기 지정해주세요. 정상적인 경우엔 쓰이지 않습니다 —
	 *  ★ 우선적으로는 해당 노트를 담당하며 다가오던 실제 접시(등록된 ActivePlatesMap)의
	 *    위치를 사용합니다. 접시 등록이 안 됐거나 실패했을 때만 이 값으로 대체됩니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PTB|Sushi|VFX")
	AActor* CompletionSpawnPoint = nullptr;

	/** 판정 성공 시 완성 비주얼은 원래 "토핑이 실제로 접시(컨베이어)에 닿는 순간"에 재생됩니다
	 *  (PTBSRPlate::HandleBoxBeginOverlap → NotifyToppingReachedConveyor). 혹시 토핑이 끝까지
	 *  아무 접시에도 안 닿는 예외 상황(레벨 밖으로 벗어나는 등)을 대비한 최대 대기 시간(초)입니다.
	 *  이 시간이 지나면 접촉 여부와 무관하게 강제로 완성 처리합니다. 판정 자체(점수)는 이 값과
	 *  무관하게 항상 즉시 확정됩니다. 0으로 두면 이 안전장치를 끕니다(닿을 때까지 무한정 대기). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi|VFX")
	float MaxSuccessVisualWaitSeconds = 3.0f;

	/** BP_SR_Topping 쪽 완성 스시 클래스 변수명 (리플렉션으로 읽음, 이름이 정확히 일치해야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi|VFX")
	FName ToppingSushiClassPropertyName = TEXT("MySushiClass");

	// 엔진 고유 가상 함수 오버라이드
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	virtual void BuildRuntimeState() override;
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const override;
	UFUNCTION(BlueprintCallable, Category = "MiniGame")
	virtual void InitializeMiniGame(const FPTBMiniGameContext& Context) override;

	UFUNCTION(BlueprintCallable, Category = "MiniGame")
	FPTBRoundResult FinishMiniGame(EPTBRoundEndReason Reason) override;
	UPROPERTY(BlueprintReadWrite, Category = "PTB|Sushi")
	int32 CurrentNoteIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Sushi")
	TArray<int32> ToppingQueue;
private:

	UPROPERTY() TMap<int32, AActor*> ActivePlatesMap;
	UPROPERTY() TMap<int32, AActor*> ActiveToppingsMap;

	/** 판정은 성공했지만 아직 완성 비주얼(VFX/모델 스폰)을 안 띄운, "토핑이 컨베이어에
	 *  닿기를 기다리는 중"인 NoteId 목록입니다. */
	UPROPERTY() TSet<int32> PendingSuccessNoteIds;

	/** 모든 키 입력마다 토핑을 무조건 스폰하기 위한 임시 키 발급용 카운터.
	 *  실제 채보 NoteId(0 이상)와 절대 겹치지 않도록 음수만 사용합니다. */
	int32 NextTempToppingKey = -1;

	/** 이번 HandleRhythmInput 호출에서 방금 스폰한 토핑의 (아직 판정 전) 임시 키.
	 *  Super::HandleRhythmInput 안에서 동기적으로 HandleJudgementResult가 호출될 때,
	 *  이 키로 방금 스폰한 토핑을 찾아 진짜 NoteId로 다시 태깅합니다. */
	int32 PendingInputToppingKey = INDEX_NONE;

	/** MaxSuccessVisualWaitSeconds가 지나도 여전히 대기 중이면 강제로 완성 처리하는 안전장치 */
	void ResolveSuccessVisualIfStillPending(int32 NoteId);

	/** 판정 성공 시, 등록된 토핑을 찾아 완성 VFX + 완성 스시 모델 스폰 후 토핑을 제거합니다.
	 *  접시(Plate)와의 물리적 Overlap 여부와 완전히 무관하게 동작합니다. */
	void ResolveSuccessVisual(int32 NoteId);
};