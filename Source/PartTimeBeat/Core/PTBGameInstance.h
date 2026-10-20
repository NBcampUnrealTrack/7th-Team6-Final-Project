#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PTBStructEnums.h"
#include "PTBGameInstance.generated.h"

class UPTBWwiseAudioManager;
class UPTBSaveGame;
class UPTBMiniGameIntroData;
class UPTBSceneWidgetBase;
class USoundBase;

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


	// 옵션창에서 호출. 즉시 적용 + 저장
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetGameLanguage(const FString& Culture);

	// 현재 언어 코드 ("ko", "en")
	UFUNCTION(BlueprintPure, Category = "Localization")
	FString GetGameLanguage() const;

	// 지원 언어 목록 (언어 추가 시 여기만 수정)
	UFUNCTION(BlueprintPure, Category = "Localization")
	static TArray<FString> GetSupportedLanguages() { return { TEXT("ko"), TEXT("en") }; }

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

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void CreateTitleWidget();

	virtual void Init() override;

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	/**	설정 캐시 */
	UPROPERTY(BlueprintReadWrite, Category = "PTB|Settings")
	FPTBUserSettings CachedSettings;

	/** 판정 오프셋 즉시 적용 (캘리브레이션 테스트용) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void SetJudgementOffsetMs(float OffsetMs) { CachedSettings.JudgementOffsetMs = OffsetMs; }
	/** 현재 모드(Single Multi) */
	EPTBPlayMode CurrentPlayMode;	
	/** 전역 Wwise 매니저*/
	UPROPERTY()
	TObjectPtr<UPTBWwiseAudioManager> AudioManager;
	/** 현재 플로우 상태(FlowSubsystem과 동기) */
	EGameFlowState CurrentFlowState;

	/** 마지막 라운드 결과 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Result")
	FPTBRoundResult LastRoundResult;

	/** 마지막 보상 요약 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Result")
	FPTBRewardSummary LastRewardSummary;

	// 변수 추가
	UPROPERTY(EditDefaultsOnly, Category = "PTB|UI")
	TSubclassOf<UPTBMainTitleWidget> TitleWidgetClass;

	UPROPERTY()
	TObjectPtr<UPTBMainTitleWidget> TitleWidgetInstance;

	// ── 미니게임 스토리/튜토리얼 공용 기본값 ───────────────────────

	/** 미니게임에 IntroData가 없거나 시작 스토리/튜토리얼 테이블이 비었을 때 쓰는 공용 템플릿 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PTB|Scene")
	TObjectPtr<UPTBMiniGameIntroData> DefaultIntroData;

	/** 스토리 위젯 (시작 스토리/결과 직전 스토리 공용). 비어 있으면 스토리 단계는 건너뜀 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PTB|Scene")
	TSubclassOf<UPTBSceneWidgetBase> StoryWidgetClass;

	/** 튜토리얼 위젯. 비어 있으면 튜토리얼 단계는 건너뜀 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PTB|Scene")
	TSubclassOf<UPTBSceneWidgetBase> TutorialWidgetClass;

	/** 튜토리얼 연습 노트 타이밍 소리 공용 기본값 (예: 싱크 조절 메트로놈 소리) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PTB|Scene")
	TObjectPtr<USoundBase> DefaultTutorialNoteSound;

	/** 튜토리얼 연습 메트로놈 소리 공용 기본값. 비우면 박 틱 없음 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PTB|Scene")
	TObjectPtr<USoundBase> DefaultTutorialMetronomeSound;

	/** 현재 로드된 세이브 오브젝트 */
	UPROPERTY()
	UPTBSaveGame* CurrentSaveGame = nullptr;

private:
	void ApplyDefaultLanguage();
};
