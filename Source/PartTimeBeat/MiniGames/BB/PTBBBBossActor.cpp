#include "MiniGames/BB/PTBBBBossActor.h"

#include "MiniGames/BB/PTBBBMiniGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInterface.h"
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
		BBMiniGame->OnBBBossDefeated.RemoveDynamic(this, &APTBBBBossActor::HandleBBBossDefeated);
	}

	BBMiniGame = InMiniGame;
	bDeathMontagePlayed = false;
	InMiniGame->OnBBNoteCue.AddUniqueDynamic(this, &APTBBBBossActor::HandleBBNoteCue);
	InMiniGame->OnBBParrySuccess.AddUniqueDynamic(this, &APTBBBBossActor::HandleBBParrySuccess);
	InMiniGame->OnBBBossDefeated.AddUniqueDynamic(this, &APTBBBBossActor::HandleBBBossDefeated);

	ApplyDifficultyMaterial(InMiniGame->GetDifficulty());
}

void APTBBBBossActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBNoteCue.RemoveDynamic(this, &APTBBBBossActor::HandleBBNoteCue);
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &APTBBBBossActor::HandleBBParrySuccess);
		BBMiniGame->OnBBBossDefeated.RemoveDynamic(this, &APTBBBBossActor::HandleBBBossDefeated);
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

void APTBBBBossActor::PlayDeathMontage_Implementation()
{
	if (bDeathMontagePlayed) return;
	bDeathMontagePlayed = true;

	if (!DeathMontage) { PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] DeathMontage 미설정")); return; }
	if (!BossMesh) return;

	UAnimInstance* AnimInst = BossMesh->GetAnimInstance();
	if (!AnimInst) { PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] PlayDeathMontage: AnimInstance 없음. AnimBP 확인 필요.")); return; }

	AnimInst->Montage_Play(DeathMontage);

	// 몽타주가 끝나며 자연스럽게 블렌드아웃되면 그 아래 AnimGraph(로코모션)가 다시 드러나
	// "쓰러졌다가 일어나는" 것처럼 보인다. 블렌드아웃이 "시작되는" 순간(아직 포즈가
	// 거의 그대로인 시점) 랙돌로 전환해서 그 자리에 쓰러진 채로 남게 한다.
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &APTBBBBossActor::HandleDeathMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DeathMontage);
}

void APTBBBBossActor::ApplyDifficultyMaterial_Implementation(EPTBDifficulty Difficulty)
{
	if (!BossMesh)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] ApplyDifficultyMaterial: BossMesh가 없습니다."));
		return;
	}

	const TObjectPtr<UMaterialInterface>* FoundMaterial = BossMaterialByDifficulty.Find(Difficulty);
	if (!FoundMaterial && Difficulty != EPTBDifficulty::Standard)
	{
		FoundMaterial = BossMaterialByDifficulty.Find(EPTBDifficulty::Standard);
	}

	if (!FoundMaterial || !FoundMaterial->Get())
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBBossActor] ApplyDifficultyMaterial: Difficulty=%d에 대한 머티리얼이 설정되지 않았습니다."), static_cast<int32>(Difficulty));
		return;
	}

	const int32 NumMaterials = BossMesh->GetNumMaterials();
	if (NumMaterials <= 0)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] ApplyDifficultyMaterial: BossMesh에 머티리얼 슬롯이 없습니다."));
		return;
	}

	if (MaterialSlotIndex < 0 || MaterialSlotIndex >= NumMaterials)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBBossActor] ApplyDifficultyMaterial: MaterialSlotIndex=%d가 유효 범위(0~%d)를 벗어났습니다."), MaterialSlotIndex, NumMaterials - 1);
		return;
	}

	BossMesh->SetMaterial(MaterialSlotIndex, FoundMaterial->Get());
}

void APTBBBBossActor::HandleDeathMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!BossMesh) return;

	BossMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	BossMesh->SetSimulatePhysics(true);
	BossMesh->SetAllBodiesSimulatePhysics(true);
	BossMesh->bBlendPhysics = true;
	BossMesh->WakeAllRigidBodies();
}

void APTBBBBossActor::HandleBBNoteCue(FPTBNoteEvent Note)
{
	PlayAttackMontage(Note);
}

void APTBBBBossActor::HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent)
{
	PlayHitReactMontage(Result);
}

void APTBBBBossActor::HandleBBBossDefeated()
{
	PlayDeathMontage();
}
