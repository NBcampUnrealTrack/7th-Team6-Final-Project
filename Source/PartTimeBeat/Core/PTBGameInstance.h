#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PTBStructEnums.h"
#include "PTBGameInstance.generated.h"

class UPTBWwiseAudioManager;
class UPTBSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowStateChanged, EGameFlowState, NewState);

// 위젯 관련
class UPTBMainTitleWidget;

UCLASS()
class PARTTIMEBEAT_API UPTBGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	//Save / Profile / Audio / Flow / TeamLog 초기화
	UFUNCTION(BlueprintCallable, Category = "PTB|Systems")
	void InitPTBSystems();
	//저장 flush, 오디오 정리, 로그 flush
	void ShutdownPTBSystems();
	//설정 저장 + Wwise / Rhythm에 전달
	void ApplyUserSettings(const FPTBUserSettings& InSettings);

	//슬롯에 저장 (프로필은 Subsystem에서 CurrentSaveGame에 먼저 반영 후 호출)
	UFUNCTION(BlueprintCallable, Category = "PTB|Save")
	void SaveGame();
	//슬롯에서 로드
	bool LoadGame();
	//결과 화면 후 자동 저장
	void AutoSave();

	/** 프로필 생성 레벨로 넘길 슬롯 인덱스 (프로필 선택 화면에서 설정) */
	UPROPERTY(BlueprintReadWrite, Category = "PTB|Profile")
	int32 PendingSlotIndex = 0;

	UFUNCTION()
	void CreateTitleWidget();

	virtual void Init() override;

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	/**	설정 캐시 */
	FPTBUserSettings CachedSettings;
	/** 현재 모드(Single Multi) */
	EPTBPlayMode CurrentPlayMode;	
	/** 전역 Wwise 매니저*/
	UPTBWwiseAudioManager* AudioManager;
	/** 현재 플로우 상태(FlowSubsystem과 동기) */
	EGameFlowState CurrentFlowState;

	// 변수 추가
	UPROPERTY(EditDefaultsOnly, Category = "PTB|UI")
	TSubclassOf<UPTBMainTitleWidget> TitleWidgetClass;

	UPROPERTY()
	TObjectPtr<UPTBMainTitleWidget> TitleWidgetInstance;

	/** 현재 로드된 세이브 오브젝트 */
	UPROPERTY()
	UPTBSaveGame* CurrentSaveGame = nullptr;
};
