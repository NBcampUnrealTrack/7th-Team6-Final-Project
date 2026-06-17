#include "MiniGames/BB/PTBBBBossActor.h"

#include "MiniGames/BB/PTBBBMiniGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
#include "Debug/PTBTeamLog.h"

APTBBBBossActor::APTBBBBossActor()
{
	BossMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossMesh"));
	RootComponent = BossMesh;
}

// ── 연결 / 해제 ──────────────────────────────────────────────────

void APTBBBBossActor::BindToMiniGame(APTBBBMiniGame* InMiniGame)
{
	if (!InMiniGame)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] BindToMiniGame: null MiniGame"));
		return;
	}

	// 동일한 미니게임이면 중복 바인딩 방지
	if (IsValid(BBMiniGame) && BBMiniGame == InMiniGame)
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] BindToMiniGame: 이미 동일한 미니게임에 바인딩됨"));
		return;
	}

	// 다른 미니게임에 바인딩되어 있으면 먼저 해제
	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &APTBBBBossActor::HandleBBNoteCue);
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &APTBBBBossActor::HandleBBParrySuccess);
	}

	BBMiniGame = InMiniGame;
	InMiniGame->OnBBNoteCue.AddUniqueDynamic(this, &APTBBBBossActor::HandleBBNoteCue);
	InMiniGame->OnBBParrySuccess.AddUniqueDynamic(this, &APTBBBBossActor::HandleBBParrySuccess);
}

void APTBBBBossActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &APTBBBBossActor::HandleBBNoteCue);
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &APTBBBBossActor::HandleBBParrySuccess);
	}
	Super::EndPlay(EndPlayReason);
}

// ── 애니메이션 기본 구현 ─────────────────────────────────────────

void APTBBBBossActor::PlayAttackMontage_Implementation(const FPTBNoteEvent& Note)
{
	// ActionType에 맞는 몽타주 선택, 없으면 기본값 사용
	UAnimMontage* MontageToPlay = nullptr;
	if (const TObjectPtr<UAnimMontage>* Found = AttackMontages.Find(Note.ActionType))
	{
		MontageToPlay = Found->Get();
	}
	if (!MontageToPlay)
	{
		MontageToPlay = DefaultAttackMontage.Get();
	}

	if (MontageToPlay)
	{
		if (BossMesh)
		{
			if (UAnimInstance* AnimInst = BossMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(MontageToPlay);
			}
			else
			{
				PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] PlayAttackMontage: BossMesh에 AnimInstance가 없습니다. AnimBP가 설정되어 있는지 확인하세요."));
			}
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] PlayAttackMontage: ActionType=%d에 대한 몽타주가 설정되지 않았습니다."), static_cast<int32>(Note.ActionType));
	}

	// ActionType에 맞는 SFX 선택, 없으면 기본값 사용
	UAkAudioEvent* SFXToPlay = nullptr;
	if (const TObjectPtr<UAkAudioEvent>* Found = AttackSFXMap.Find(Note.ActionType))
	{
		SFXToPlay = Found->Get();
	}
	if (!SFXToPlay)
	{
		SFXToPlay = DefaultAttackSFX.Get();
	}

	if (SFXToPlay)
	{
		UAkGameplayStatics::PostEvent(SFXToPlay, this, 0, FOnAkPostEventCallback(), false);
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] PlayAttackMontage: ActionType=%d에 대한 SFX가 설정되지 않았습니다."), static_cast<int32>(Note.ActionType));
	}
}

void APTBBBBossActor::PlayHitReactMontage_Implementation(const FPTBJudgementResult& Result)
{
	if (HitReactMontage)
	{
		if (BossMesh)
		{
			if (UAnimInstance* AnimInst = BossMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(HitReactMontage);
			}
			else
			{
				PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] PlayHitReactMontage: BossMesh에 AnimInstance가 없습니다. AnimBP가 설정되어 있는지 확인하세요."));
			}
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] PlayHitReactMontage: HitReactMontage가 설정되지 않았습니다."));
	}

	if (HitReactSFX)
	{
		UAkGameplayStatics::PostEvent(HitReactSFX, this, 0, FOnAkPostEventCallback(), false);
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] PlayHitReactMontage: HitReactSFX가 설정되지 않았습니다."));
	}
}

// ── 델리게이트 핸들러 ────────────────────────────────────────────

void APTBBBBossActor::HandleBBNoteCue(FPTBNoteEvent Note)
{
	PlayAttackMontage(Note);
}

void APTBBBBossActor::HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent)
{
	PlayHitReactMontage(Result);
}