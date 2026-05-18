#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTBStructEnums.h"
#include "PTBProfileSubsystem.generated.h"

UCLASS()
class PARTTIMEBEAT_API UPTBProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/**최대 프로필 슬롯 */
	int32 MaxProfiles = 5;
	/**전체 프로필 목록 */
	TArray<FPTBProfileData> AllProfiles;

	//신규 생성
	FPTBProfileData CreateProfile(const FString& Nickname, int32 Gender, FDateTime Birthday);
	// 삭제
	bool DeleteProfile(const FString& ProfileId);
	//	조회
	FPTBProfileData GetProfile(const FString& ProfileId) const;
	//	현재 슬롯 사용 수
	int32 GetProfileCount() const;
	//	길이 / 금칙어 검증
	bool ValidateNickname(const FString& Name) const;
	//	첫 튜토리얼 미완료 여부
	bool IsFirstTimeProfile(const FString& ProfileId) const;
	//	튜토리얼 완료 마킹
	void MarkTutorialComplete(const FString& ProfileId, FName GameId);
};
