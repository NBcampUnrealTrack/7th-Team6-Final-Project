// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBFSMiniGameRuleSet.generated.h"

/**
 * 
 */
UCLASS()
class PARTTIMEBEAT_API UPTBFSMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	// ── 키 셔플 기믹 ─────────────────────────────
	// 화면 위치(왼/가운데/오른쪽)는 그대로, 각 위치를 맞추는 키가 정해진 순서대로 바뀐다.
	// 노트마다 "그 노트가 속한 구간"의 배치로 판정하므로, 교체 연출 타이밍과 상관없이 채보를 외우면 퍼펙트 가능.

	/** 키 셔플을 켤 난이도 (기본: 발광모드만) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle")
	TArray<EPTBDifficulty> KeyShuffleDifficulties = { EPTBDifficulty::Insane };

	/** 몇 마디마다 배치를 바꿀지 (박자표 기준 마디) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle", meta = (ClampMin = "1"))
	int32 KeyShuffleMeasures = 1;

	/** 배치 순서를 만드는 고정 시드. 같은 시드면 매번 같은 순서 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle")
	int32 KeyShuffleSeed = 20261021;

	/**
	 * 직접 지정하는 배치 순서 (비어 있으면 시드로 생성).
	 * 각 항목 = 왼/가운데/오른쪽 칸을 맞추는 키 3글자. 예) "ABC"(기본), "CAB" = 왼쪽 C키, 가운데 A키, 오른쪽 B키
	 * 0번이 첫 구간, 끝까지 쓰면 처음부터 반복
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle")
	TArray<FString> KeyShufflePattern;

	/** 키 안내 UI 교체 연출 최대 시간(ms). 노트 간격이 짧으면 간격의 절반까지 자동으로 줄어듦 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle", meta = (ClampMin = "0.0"))
	float KeyShuffleAnimMaxMs = 180.0f;

	/** 구간 마지막 노트 이후 교체 연출 시작까지 최대 대기(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle", meta = (ClampMin = "0.0"))
	float KeyShuffleAnimDelayMs = 80.0f;

	/** 튜토리얼 연습에도 키 셔플 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FS|KeyShuffle")
	bool bKeyShuffleInPractice = false;
};
