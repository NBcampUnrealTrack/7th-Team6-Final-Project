#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseEventMapAsset.generated.h"

class UAkAudioEvent;

/**
 * Wwise 이벤트 Asset을 용도별 키로 관리하는 오디오 이벤트 매핑 DataAsset입니다.
 *
 * AudioManager와 미니게임은 문자열 키나 판정 타입을 통해 이 Asset에서 실제 Wwise 이벤트를 찾습니다.
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBWwiseEventMapAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** BGM_<GameCode> 매핑 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TMap<FName, TObjectPtr<UAkAudioEvent>> BGMEvents;

	/** 판정별 SFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TMap<EPTBJudgementType, TObjectPtr<UAkAudioEvent>> JudgeEventMap;

	/** UI SFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TMap<FName, TObjectPtr<UAkAudioEvent>> UIEvents;

	/** 미니게임 전용 SFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TMap<FName, TObjectPtr<UAkAudioEvent>> MiniGameSFXMap;
};
