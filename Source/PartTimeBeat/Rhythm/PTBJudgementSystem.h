#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgementSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJudgementResult, FPTBJudgementResult, JudgementResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboBreak, int32, FinalCombo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBJudgementSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTBJudgementSystem();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//	채보로 초기화
	void Initialize(const FPTBChartData& Chart, float UserOffset);
	//	Conductor로부터 노트 수신
	void RegisterNoteEvent(const FPTBNoteEvent& Note);
	//	입력 판정
	FPTBJudgementResult EvaluateInput(EPTBActionType Action, float InputTimeMs);
	//	만료 노트 자동 Miss
	TArray<FPTBJudgementResult> ForceMissExpiredNotes(float CurrentTimeMs);
	//	카운터 초기화
	void Reset();

	float HitWindowHighPerfectMs = 21.0;
	float HitWindowPerfectMs = 50.0;
	float HitWindowGoodMs = 70.0;
	float HitWindowMissMs = 120.0;
	TArray<FPTBNoteEvent> PendingNotes;
	float JudgementOffsetMs = 0.0;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Score")
	FOnJudgementResult OnJudgementResult;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Score")
	FOnComboBreak OnComboBreak;
};
