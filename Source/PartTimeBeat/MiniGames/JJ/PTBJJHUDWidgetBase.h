// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"   // EPTBJudgementType, FPTBJudgementResult
#include "PTBJJHUDWidgetBase.generated.h"

class APTBJJMiniGame;

/**
 * JumpJump HUD 컨테이너의 C++ 베이스입니다.
 *
 * 역할:
 *  - 미니게임(APTBJJMiniGame)의 판정 이벤트(OnJJLanding)를 바인딩하고,
 *    판정이 날 때마다 BP로 알려준다(OnJudgement).
 *  - 지금은 판정 텍스트만 다루지만, 콤보/점수/진행바 등을 여기 얹어 확장한다.
 *
 * 하이브리드 패턴:
 *  - C++: 미니게임 참조 보관 + 델리게이트 바인딩(라이프사이클 안전) 담당.
 *  - BP(WBP_JJ_HUD): 실제 위젯 배치와 연출(OnJudgement에서 판정 위젯에 ShowJudgement 호출).
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBJJHUDWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * HUD를 특정 미니게임에 연결한다.
	 * OnJJLanding 델리게이트를 바인딩해, 이후 판정마다 OnJudgement()가 호출된다.
	 * 레벨 BP나 게임모드에서 미니게임을 스폰/캐싱한 뒤 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JJ|HUD")
	void BindToMiniGame(APTBJJMiniGame* InMiniGame);

	/** 현재 연결된 미니게임 */
	UFUNCTION(BlueprintPure, Category = "PTB|JJ|HUD")
	APTBJJMiniGame* GetMiniGame() const { return MiniGame; }

protected:
	virtual void NativeDestruct() override;

	/**
	 * 판정 발생 시 BP에서 처리.
	 * 여기서 판정 위젯의 ShowJudgement(JudgementType, CharacterIndex)를 호출하면 된다.
	 *
	 * @param CharacterIndex 좌0 / 중1 / 우2
	 * @param JudgementType  판정 타입
	 * @param Result         원본 판정 결과(콤보/점수 확장 시 활용)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JJ|HUD")
	void OnJudgement(int32 CharacterIndex, EPTBJudgementType JudgementType, const FPTBJudgementResult& Result);

private:
	/** OnJJLanding 바인딩 콜백 */
	UFUNCTION()
	void HandleLanding(int32 CharacterIndex, FPTBJudgementResult Result, FPTBNoteEvent Note);

	/** 연결된 미니게임 */
	UPROPERTY()
	TObjectPtr<APTBJJMiniGame> MiniGame = nullptr;
};