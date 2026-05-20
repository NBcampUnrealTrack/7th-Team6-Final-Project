#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PTBStructEnums.h"
#include "PTBSaveGame.generated.h"

UCLASS()
class PARTTIMEBEAT_API UPTBSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/** 전체 프로필 */
	TArray<FPTBProfileData> Profiles;

	/** 현재 활성 */
	int32 ActiveProfileIndex;

	/** 스테이지 진행 */
	TMap<FName, FPTBStageInfo> StageProgress;

	/** 스토리 시청 여부 */
	TMap<FName, bool> StoryProgress;

	/** 전역 설정 (프로필 공유) */
	FPTBUserSettings Settings;

	/** GameID → 튜토리얼 완료 */
	UPROPERTY()
	TMap<FName, bool> TutorialFlags;

	FString SaveSlotName = "PTBSave";
	
	/** 로컬 유저 인덱스 */
	int32 UserIndex = 0;

	/** 신규 세이브 기본값 */
	void InitializeDefaultSave();
	/** 프로필 추가/갱신 */
	bool UpsertProfile(const FPTBProfileData& Data);
	/** 보상 적용 */
	void ApplyRewardSummary(const FString& ProfileId, const FPTBRewardSummary& Reward);
	/** 튜토리얼 완료 */
	void MarkTutorialDone(const FString& ProfileId, FName GameId);
};
