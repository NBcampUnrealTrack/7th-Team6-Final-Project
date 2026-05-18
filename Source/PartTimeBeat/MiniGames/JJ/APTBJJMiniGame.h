#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "APTBJJMiniGame.generated.h"

UCLASS()
class PARTTIMEBEAT_API AAPTBJJMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
public:
	//좌 / 중 / 우 캐릭터
	//TArray<APTBJJCharacterActor*> JumpCharacters; 
	//캐릭터별 속도 배율
	TArray<float> JumpSpeeds;
	//착지 피드백 허용 시간
	int32 LandingWindowMs;
	//FPTBJJJumpPattern CurrentJumpPattern;
	
	//지정 캐릭터 점프
	void TriggerCharacterJump(int32 CharacterIndex, float JumpPowerScale);
    //착지 판정 연출
	void PlayLandingFeedback(int32 CharacterIndex, const FPTBJudgementResult& Result);
	//	변속 적용
	void ApplyVariableJumpSpeed(float SpeedScale);

	//Wwise 이벤트로 Play_SFX_JJ_Jump, Play_SFX_JJ_Land, Play_SFX_JJ_Fail
};
