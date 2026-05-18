#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AkInclude.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseAudioManager.generated.h"

class UAkComponent;
class UAkAudioEvent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBGMFinished, int32, PlayingId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatCallback, float, Beat);

UCLASS()
class PARTTIMEBEAT_API UPTBWwiseAudioManager : public UObject
{
	GENERATED_BODY()
public: 
	// 글로벌 AkComponent(월드 위치 없는 글로벌 사운드)
	UAkComponent* MainAkComponent;
	//	로드된 SoundBank 목록
	TSet<FName> LoadedBanks;
	//	이벤트 키 → 실제 이벤트 매핑
	TMap<FName, UAkAudioEvent*> EventMap;
	//	Master Bus ID
	AkUniqueID MasterBusID;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBGMFinished OnBGMFinished;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBeatCallback OnBeatCallback;

	bool LoadSoundBank(FName BankName);
	void UnloadSoundBank(FName BankName);
	void UnloadAllBanks();

	//	일반 이벤트, PlayingId 반환
	int32 PostEvent(FName EventKey, AActor* Target);
	//	BGM(PlayingId 보존)
	int32 PostBGMEvent(FName EventKey);
	//	SFX
	int32 PostSFXEvent(FName EventKey, AActor* Target);
	//	BGM 정지
	void StopBGM(float FadeOutMs);
	//	BGM 일시정지 
	void PauseBGM();
	// BGM 재개
	void ResumeBGM();

	// RTPC 설정
	void SetRTPC(FName Name, float Value, AActor* Target);
	//	Wwise State
	void SetState(FName Group, FName State);
	//	Wwise Switch
	void SetSwitch(FName Group, FName Switch, AActor* Target);
	//	설정 전체 적용(볼륨 RTPC 등)
	void ApplySettings(const FPTBUserSettings& Settings);

	// 재생 위치(Conductor 핵심)
	float GetPlaybackPositionMs(int32 PlayingId) const;
	//	재생 중 여부
	bool IsEventPlaying(int32 PlayingId) const;
};
