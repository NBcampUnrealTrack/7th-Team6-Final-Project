#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/CH/PTBCHTypes.h"
#include "PTBCHMiniGame.generated.h"

class UPTBCHMiniGameRuleSet;
class UWrapperWidget;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBCHNoteEvent, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBCHJudgementEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBCHNoteClearedEvent, int32, NoteId, EPTBJudgementType, JudgementType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBCHHoldStartedEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);

/**
 * 커스텀 햄버거 미니게임 Actor입니다
 *
 * 이 클래스는 채보 이벤트, 공통 Action 입력, Hold/Release 정보, 판정 결과 생성을 확인하는 데 사용합니다
 */
UCLASS()
class PARTTIMEBEAT_API APTBCHMiniGame : public APTBBaseMiniGame
{
    GENERATED_BODY()

public:
    /** 노트 Cue 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHNoteEvent OnCHNoteCue;

    /** 노트 Arm 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHNoteEvent OnCHNoteArm;

    /** 노트 정시점 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHNoteEvent OnCHNoteReached;

    /** 판정 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHJudgementEvent OnCHJudgement;

    /** Hold 시작 입력 성공 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHHoldStartedEvent OnCHHoldStarted;

    /** 노트 제거 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "PTB|CH")
    FPTBCHNoteClearedEvent OnCHNoteCleared;

    /** 오브젝트 / 상태 구성 */
    virtual void BuildRuntimeState() override;

    /** 채보 이벤트 처리 */
    virtual void HandleChartEvent(FPTBNoteEvent Note) override;

    /** 판정 등록 가능 상태 진입 */
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;

    /** 선행 비주얼 큐 */
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;

    /** 점수 / HUD / SFX 반영 */
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

    /** 시작 준비 완료 처리 */
    virtual void HandleReadyToStart() override;

    virtual void HandleRhythmInput(EPTBActionType Action, float TimeMs = -1.0f) override;

    UPROPERTY()
    EPTBActionType LastPressedAction = EPTBActionType::None;

    /** CH Action 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleCHInput(EPTBActionType Action, float TimeMs = -1.0f);

    /** CH Action 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleCHInputReleased(EPTBActionType Action, float TimeMs = -1.0f);

    /** ActionA 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionAInput(float TimeMs = -1.0f);

    /** ActionA 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionAReleased(float TimeMs = -1.0f);

    /** ActionB 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionBInput(float TimeMs = -1.0f);

    /** ActionB 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionBReleased(float TimeMs = -1.0f);

    /** ActionC 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionCInput(float TimeMs = -1.0f);

    /** ActionC 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionCReleased(float TimeMs = -1.0f);

    /** ActionD 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionDInput(float TimeMs = -1.0f);

    /** ActionD 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionDReleased(float TimeMs = -1.0f);

    /** ActionE 입력 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionEInput(float TimeMs = -1.0f);

    /** ActionE 입력 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "PTB|CH")
    void HandleActionEReleased(float TimeMs = -1.0f);

    /** HUD 위젯 클래스 (에디터에서 설정) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TSubclassOf<UUserWidget> HUDWidgetClass;

protected:
    /** Hold 시작 입력 판정 */
    virtual FPTBJudgementResult EvaluateHoldInput(EPTBActionType Action, float TimeMs) override;

    /** CH RuleSet 조회 */
    const UPTBCHMiniGameRuleSet* GetCHRuleSet() const;

    /** 노트 추적 */
    void TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note);

    /** 노트 추적 해제 */
    bool RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId);

    /** 추적 노트 조회 */
    bool FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const;

    /** 노트 디버그 로그 출력 */
    void LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const;

    /** 판정 디버그 로그 출력 */
    void LogJudgementDebug(const FPTBJudgementResult& Result) const;

    /** Cue 누적 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 CueCount = 0;

    /** Arm 누적 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 ArmCount = 0;

    /** NoteEvent 누적 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 NoteEventCount = 0;

    /** LongNote Cue 누적 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 LongNoteCueCount = 0;

    /** 판정 누적 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 JudgementCount = 0;

    /** Cue 상태 노트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TArray<FPTBNoteEvent> CuedNotes;

    /** NoteId → 재료 매핑된 ActionType (Arm/Reached 이벤트에 일관성 있게 전달하기 위함) */
    TMap<int32, EPTBActionType> NoteIdToIngredientAction;

    /** Arm 상태 노트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TArray<FPTBNoteEvent> ArmedNotes;

    /** 정시점 도달 노트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TArray<FPTBNoteEvent> ReachedNotes;

    /** 현재 커서 위치 (0~8) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 CurrentCursorIndex = 0;

    /** 현재 손님 인덱스 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 CurrentCustomerIndex = 0;

    /** 현재 손님의 재료 인덱스 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    int32 CurrentIngredientIndex = 0;

    /** 현재 손님 주문 목록 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TArray<FPTBCHCustomerOrder> CustomerOrders;

    /** 현재까지 선택한 재료 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TArray<EPTBCHIngredientType> PlacedIngredients;

    /** 재료 Blueprint 클래스 (에디터에서 설정) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_BreadBottom;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_BreadTop;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_Lettuce;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_Patty;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_Cheese;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH|Ingredients")
    TSubclassOf<AActor> IngredientClass_Tomato;

    /** 현재 쌓인 높이 트래킹 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    float StackHeight = 50.0f;

    /** 스폰된 재료 Actor 목록 */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedIngredients;

    /** 재료 스폰 함수 */
    void SpawnIngredient(EPTBCHIngredientType IngredientType, bool bIsLastBread = false);

    /** 완성된 햄버거 목록 (각 햄버거의 재료 Actor 배열) */
    TArray<TArray<TObjectPtr<AActor>>> CompletedHamburgers;

    /** 접시 Blueprint 클래스 (에디터에서 설정) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    TSubclassOf<AActor> PlateActorClass;

    /** 완성된 햄버거의 접시 목록 */
    TArray<TObjectPtr<AActor>> CompletedPlates;

    /** 이동 속도 */
    float SlideSpeed = 800.0f;

    /** 슬라이드 그룹 (접시 + 위에 쌓인 재료들을 한 단위로 이동) */
    struct FCHSlideGroup
    {
        AActor* Plate = nullptr;
        TArray<AActor*> Ingredients;
        float TargetX = 0.0f;
        float Delay = 0.0f;
    };
    TArray<FCHSlideGroup> SlideGroups;

    /** 드롭 애니메이션 (위에서 떨어지는 효과 + 바운스) */
    struct FCHDropAnim
    {
        AActor* Ingredient = nullptr;
        float TargetZ = 0.0f;
        float Velocity = 0.0f;
        bool bDone = false;
    };
    TArray<FCHDropAnim> DroppingIngredients;
    float DropStartOffset = 80.0f;
    float DropGravity = 2000.0f;    // 낙하 가속도
    float DropRestitution = 0.3f;   // 바운스 반발계수 (0=없음, 1=완전탄성)

    /** 생성된 HUD 위젯 인스턴스 */
    UPROPERTY(BlueprintReadOnly, Category = "PTB|CH")
    TObjectPtr<UUserWidget> HUDWidget;

    /** HUD 생성 및 화면에 추가 */
    void CreateAndAddHUD();

    virtual void Tick(float DeltaTime) override;
};