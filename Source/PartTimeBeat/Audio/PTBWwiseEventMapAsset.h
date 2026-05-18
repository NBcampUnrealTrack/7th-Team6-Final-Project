#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseEventMapAsset.generated.h"

class UAkAudioEvent;

UCLASS()
class PARTTIMEBEAT_API UPTBWwiseEventMapAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** BGM_<GameCode> 매핑 */
	TMap<FName, UAkAudioEvent*> BGMEvents;
	/** 판정별 SFX */
	TMap<EPTBJudgementType, UAkAudioEvent*> JudgeEventMap;
	/** UI SFX */
	TMap<FName, UAkAudioEvent*> UIEvents;
	/** 미니게임 전용 SFX */
	TMap<FName, UAkAudioEvent*> MiniGameSFXMap;
};
