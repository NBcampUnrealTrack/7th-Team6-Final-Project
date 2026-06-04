#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBActionCueWidgetBase.generated.h"

/**
 * ActionType별 키 라벨·색상 정보를 제공하는 노트 큐 위젯 공통 베이스.
 *
 * 모든 미니게임의 노트 큐 위젯(TG TapCue, HoldCue, BB 등)이 이 클래스를 상속한다.
 * BP 파생 위젯은 OnActionTypeSet()을 구현해 키 텍스트·색상을 화면에 반영한다.
 *
 * 기본 키 매핑: ActionA=Z  ActionB=X  ActionC=C  ActionD=V  ActionE=B
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBActionCueWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 큐가 나타내는 입력 액션을 설정하고 OnActionTypeSet()을 호출한다 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cue")
	void SetActionType(EPTBActionType InActionType);

	/** 현재 설정된 입력 액션 */
	UFUNCTION(BlueprintPure, Category = "PTB|Cue")
	EPTBActionType GetActionType() const { return CurrentActionType; }

	/**
	 * ActionType에 대응하는 키보드 키 라벨 반환
	 * ActionA→"Z"  ActionB→"X"  ActionC→"C"  ActionD→"V"  ActionE→"B"
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Cue")
	static FText GetActionKeyLabel(EPTBActionType InActionType);

	/**
	 * ActionType에 대응하는 UI 색상 반환.
	 * ActionA(Z)→빨강  ActionB(X)→파랑  ActionC(C)→노랑
	 * ActionD(V)→초록  ActionE(B)→보라
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Cue")
	static FLinearColor GetActionColor(EPTBActionType InActionType);

protected:
	/** 액션 타입이 설정될 때 BP에서 키 텍스트·배경색 등을 갱신 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Cue")
	void OnActionTypeSet(EPTBActionType InActionType);

	UPROPERTY(BlueprintReadWrite, Category = "PTB|Cue")
	EPTBActionType CurrentActionType = EPTBActionType::None;
};
