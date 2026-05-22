#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PTBStructEnums.h"
#include "PTBGameInstance.generated.h"

class UPTBWwiseAudioManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowStateChanged, EGameFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileChanged, FPTBProfileData, NewProfile);

// 위젯 관련
class UPTBMainTitleWidget;

UCLASS()
class PARTTIMEBEAT_API UPTBGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	//Save / Profile / Audio / Flow / TeamLog 초기화
	void InitPTBSystems();	
	//저장 flush, 오디오 정리, 로그 flush
	void ShutdownPTBSystems();
	//프로필 활성화
	bool LoadProfile(const FString& ProfileId);	
	//신규 프로필 생성, ID 반환
	FString CreateProfile(const FPTBProfileData& Data);	
	//설정 저장 + Wwise / Rhythm에 전달
	void ApplyUserSettings(const FPTBUserSettings& InSettings);	
		
	//슬롯에 저장
	void SaveGame();	
	//슬롯에서 로드
	bool LoadGame();	
	//결과 화면 후 자동 저장
	void AutoSave();

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Profile")
	FOnProfileChanged OnProfileChanged;

	/** 현재 활성 프로필 ID */
	FString ActiveProfileId;	
	/** 활성 프로필 데이터 */
	FPTBProfileData ActiveProfile;	
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
};
