#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AkInclude.h"
#include "AkGameplayTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseAudioManager.generated.h"

class UAkComponent;
class UAkAudioEvent;
class UPTBWwiseEventMapAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBGMFinished, int32, PlayingId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatCallback, float, Beat);

/**
 * Wwise 이벤트 재생과 음악 재생 위치 조회를 담당하는 런타임 오디오 매니저입니다.
 *
 * 리듬 판정의 기준 시간은 이 매니저가 보관한 BGM PlayingId를 통해 Wwise에서 조회합니다.
 * 미니게임과 RhythmConductor는 직접 Wwise API를 호출하지 않고 이 매니저를 통해 BGM, SFX, RTPC, State, Switch를 요청합니다.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBWwiseAudioManager : public UObject
{
	GENERATED_BODY()
public: 
	/** 기본값 초기화 */
	UPTBWwiseAudioManager();

	/** 글로벌 AkComponent */
	UPROPERTY()
	TObjectPtr<UAkComponent> MainAkComponent = nullptr;

	/** 이벤트 매핑 DataAsset */
	UPROPERTY()
	TObjectPtr<UPTBWwiseEventMapAsset> EventMapAsset = nullptr;

	/** 로드된 SoundBank 목록 */
	TSet<FName> LoadedBanks;

	/** 이벤트 키와 Wwise 이벤트 Asset 매핑 */
	UPROPERTY(EditAnywhere, Category = "PTB|Audio")
	TMap<FName, TObjectPtr<UAkAudioEvent>> EventMap;

	/** 판정 타입과 Wwise 이벤트 Asset 매핑 */
	UPROPERTY(EditAnywhere, Category = "PTB|Audio")
	TMap<EPTBJudgementType, TObjectPtr<UAkAudioEvent>> JudgementEventMap;

	/** Master Bus ID */
	AkUniqueID MasterBusID = 0;

	/** 현재 BGM PlayingId */
	int32 CurrentBGMPlayingId = 0;

	/** BGM 재생 중 여부 */
	bool bIsBGMPlaying = false;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBGMFinished OnBGMFinished;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBeatCallback OnBeatCallback;

	/** 글로벌 AkComponent 설정 */
	void SetMainAkComponent(UAkComponent* InAkComponent);

	/** 이벤트 매핑 DataAsset 적용 */
	void ApplyEventMapAsset(UPTBWwiseEventMapAsset* InEventMapAsset);

	/** 이벤트 키 직접 등록 */
	void RegisterEvent(FName EventKey, UAkAudioEvent* Event);

	bool LoadSoundBank(FName BankName);
	void UnloadSoundBank(FName BankName);
	void UnloadAllBanks();

	/** 이벤트 데이터 준비 */
	bool PrepareEvent(FName EventKey);

	/** 일반 이벤트 재생 */
	int32 PostEvent(FName EventKey, AActor* Target);

	/** BGM 이벤트 재생 */
	int32 PostBGMEvent(FName EventKey);

	/** SFX 이벤트 재생 */
	int32 PostSFXEvent(FName EventKey, AActor* Target);

	/** 판정 SFX 이벤트 재생 */
	int32 PostJudgementEvent(EPTBJudgementType JudgementType, AActor* Target);

	/** BGM 정지 */
	void StopBGM(float FadeOutMs);

	/** BGM 일시정지 */
	void PauseBGM();

	/** BGM 재개 */
	void ResumeBGM();

	/** RTPC 설정 */
	void SetRTPC(FName Name, float Value, AActor* Target);

	/** Wwise State 설정 */
	void SetState(FName Group, FName State);

	/** Wwise Switch 설정 */
	void SetSwitch(FName Group, FName Switch, AActor* Target);

	/** 사용자 오디오 설정 적용 */
	void ApplySettings(const FPTBUserSettings& Settings);

	/** Wwise 재생 위치 조회 */
	float GetPlaybackPositionMs(int32 PlayingId) const;

	/** Wwise 재생 위치 조회 */
	bool TryGetPlaybackPositionMs(int32 PlayingId, float& OutPositionMs) const;

	/** 이벤트 재생 중 여부 */
	bool IsEventPlaying(int32 PlayingId) const;

private:
	/** BGM EndOfEvent 콜백 */
	UFUNCTION()
	void HandleBGMPostEventCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
};
