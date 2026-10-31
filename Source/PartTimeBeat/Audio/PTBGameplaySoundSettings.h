#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PTBGameplaySoundSettings.generated.h"

class USoundBase;

/**
 * 모든 미니게임 공통 입력/판정 효과음.
 * 에디터: Edit → Project Settings → Game → "PTB 게임플레이 사운드"
 * 저장 위치: Config/DefaultGame.ini
 *
 * - 사운드 설정의 "효과음(입력·판정)" 체크로 켜고 끔 (기본 켬). 튜토리얼 메트로놈은 이 설정과 무관하게 항상 재생
 * - 미니게임별로 다른 소리를 쓰려면 RuleSet의 PressSoundOverride / HitSoundOverride / MissSoundOverride
 * - 볼륨 = 아래 볼륨 × 사용자 설정 Master × SFX
 *
 * 패키징 주의: Config의 소프트 참조는 쿡 대상으로 자동 포함되지 않음.
 * 사운드가 들어있는 폴더를 Project Settings → Packaging → Additional Asset Directories to Cook에 추가할 것.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB 게임플레이 사운드"))
class PARTTIMEBEAT_API UPTBGameplaySoundSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** 키를 눌렀을 때 (판정 대상 노트가 없는 입력. bPressSoundOnlyWhenNotJudged 참고) */
	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> PressSound;

	/** 판정 성공 (HighPerfect / Perfect / Good) */
	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> HitSound;

	/** 판정 실패 (Miss. 노트를 놓쳐서 생긴 Miss 포함) */
	UPROPERTY(Config, EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> MissSound;

	UPROPERTY(Config, EditAnywhere, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float PressVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float HitVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MissVolume = 1.0f;

	/**
	 * true(기본): 누른 입력이 판정을 만들면 Press 소리는 생략하고 Hit/Miss 소리만 (소리 겹침 방지)
	 * false: 누를 때마다 Press 소리 + 판정 소리를 같이 재생
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Behavior")
	bool bPressSoundOnlyWhenNotJudged = true;
};
