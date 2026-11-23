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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUserSettingsApplied);
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

	// 지원 언어 목록 (컬처 이름, 예: "ko", "en", "ja", "zh-Hans")
	// 하드코딩하지 않고 실제로 존재하는 게임 번역 데이터에서 자동으로 구성됨.
	// → Localization Dashboard에서 언어 추가 + Project Settings > Packaging > Localizations to Package 체크만 하면 자동 반영.
	// 첫 항목은 항상 원문 언어(Native, ko).
	UFUNCTION(BlueprintPure, Category = "Localization")
	static TArray<FString> GetSupportedLanguages();

	// 언어 코드("zh") 또는 컬처 이름("zh-Hans", "ko-KR")에 해당하는 지원 컬처 이름. 없으면 빈 문자열.
	static FString FindSupportedCulture(const FString& LanguageOrCulture);

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
	virtual void Shutdown() override;

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PTB|Settings")
	FOnUserSettingsApplied OnUserSettingsApplied;

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

	/** 그래픽 품질 최대값 (0=낮음, 1=중간, 2=높음, 3=최고) */
	static constexpr int32 MaxGraphicsQuality = 3;

	/** 첫 실행 자동 설정이 고를 수 있는 최대 품질 (기본 2=높음. '최고'는 플레이어가 직접 선택) */
	static constexpr int32 MaxAutoGraphicsQuality = 2;

private:
	void ApplyDefaultLanguage();

	/**
	 * 첫 실행이면 하드웨어 벤치마크로 기본 그래픽 품질을 정한다.
	 * 새 세이브에서만 측정하고, 기존 세이브는 플레이어가 고른 값을 유지한다.
	 */
	void AutoDetectGraphicsIfNeeded(bool bIsNewSave);

	/** 측정 결과로 권장 품질(0~MaxAutoGraphicsQuality) 계산 */
	static int32 RunGraphicsBenchmark();

	/** 에디터(PIE)에서 게임 언어 미리보기로 적용 중인 언어. 에디터 자체 언어는 바꾸지 않기 위함 */
	FString EditorPreviewCulture;
};
