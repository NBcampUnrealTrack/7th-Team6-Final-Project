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

	// 블루프린트에서 노트 번호만 주면 셔플된 대기열에서 몇 번 토핑인지 안전하게 꺼내줍니다. (Pure 함수라 실행선 불필요)
	UFUNCTION(BlueprintPure, Category = "PTB|Sushi") 
	int32 GetToppingTypeFromQueue(int32 NoteId) const;
	UFUNCTION(BlueprintPure, Category = "PTB|Sushi") 
	FPTBToppingRow GetToppingData(int32 ToppingType) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Sushi|UI") 
	class UUserWidget* WBP_SR_Preview;
	UFUNCTION(BlueprintCallable, Category = "PTB|Sushi|UI")
	void RefreshPreviewUI();
protected:
	virtual void HandleRhythmInput(EPTBActionType Action, float TimeMs = -1.0f) override;


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
};
