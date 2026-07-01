#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTBMiniGamePreloadAssetSet.generated.h"

/**
 * 미니게임 시작 전 로딩 화면에서 준비할 에셋 목록입니다.
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBMiniGamePreloadAssetSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 사전 로드 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Preload")
	TArray<TSoftObjectPtr<UObject>> Assets;
};
