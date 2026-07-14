#include "MiniGames/BB/PTBBBPlayerActor.h"

#include "MiniGames/BB/PTBBBMiniGame.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"

APTBBBPlayerActor::APTBBBPlayerActor()
{
	PlayerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlayerMesh"));
	RootComponent = PlayerMesh;
}

// ── 연결 / 해제 ──────────────────────────────────────────────────

void APTBBBPlayerActor::BindToMiniGame(APTBBBMiniGame* InMiniGame)
{
	if (!InMiniGame)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] BindToMiniGame: null MiniGame"));
		return;
	}

	// 동일한 미니게임이면 중복 바인딩 방지
	if (IsValid(BBMiniGame) && BBMiniGame == InMiniGame)
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] BindToMiniGame: 이미 동일한 미니게임에 바인딩됨"));
		return;
	}

	// 다른 미니게임에 바인딩되어 있으면 먼저 해제
	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &APTBBBPlayerActor::HandleBBParrySuccess);
		BBMiniGame->OnBBParryFail.RemoveDynamic(this, &APTBBBPlayerActor::HandleBBParryFail);
	}

	BBMiniGame = InMiniGame;
	bDeathMontagePlayed = false;
	InMiniGame->OnBBParrySuccess.AddUniqueDynamic(this, &APTBBBPlayerActor::HandleBBParrySuccess);
	InMiniGame->OnBBParryFail.AddUniqueDynamic(this, &APTBBBPlayerActor::HandleBBParryFail);

	if (bUseAsViewTarget)
	{
		if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			PlayerController->SetViewTargetWithBlend(ResolvePreferredViewTarget(), ViewTargetBlendTime);
		}
	}

	// 활성 프로필에서 캐릭터 메시 자동 적용
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBProfileSubsystem* ProfileSub = GI->GetSubsystem<UPTBProfileSubsystem>())
		{
			bool bHasActive = false;
			const FPTBProfileData Profile = ProfileSub->GetActiveProfile(bHasActive);
			if (bHasActive)
			{
				ApplyProfileCharacter(Profile);
			}
			else
			{
				PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] BindToMiniGame: 활성 프로필이 없습니다. 기본 메시를 유지합니다."));
			}
		}
		else
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] BindToMiniGame: ProfileSubsystem을 가져올 수 없습니다."));
		}
	}
}

void APTBBBPlayerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(BBMiniGame))
	{
		BBMiniGame->OnBBParrySuccess.RemoveDynamic(this, &APTBBBPlayerActor::HandleBBParrySuccess);
		BBMiniGame->OnBBParryFail.RemoveDynamic(this, &APTBBBPlayerActor::HandleBBParryFail);
	}
	Super::EndPlay(EndPlayReason);
}

// ── 애니메이션 기본 구현 ─────────────────────────────────────────

void APTBBBPlayerActor::PlayAttackMontage_Implementation(const FPTBJudgementResult& Result)
{
	// ActionType에 맞는 몽타주 선택, 없으면 기본값 사용
	UAnimMontage* MontageToPlay = nullptr;
	if (const TObjectPtr<UAnimMontage>* Found = AttackMontages.Find(Result.ActionType))
	{
		MontageToPlay = Found->Get();
	}
	if (!MontageToPlay)
	{
		MontageToPlay = DefaultAttackMontage.Get();
	}

	if (MontageToPlay)
	{
		if (UAnimInstance* AnimInst = GetPlayerAnimInstance())
		{
			AnimInst->Montage_Play(MontageToPlay);
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] PlayAttackMontage: ActionType=%d에 대한 몽타주가 설정되지 않았습니다."), static_cast<int32>(Result.ActionType));
	}

	// ActionType에 맞는 SFX 선택, 없으면 기본값 사용
	UAkAudioEvent* SFXToPlay = nullptr;
	if (const TObjectPtr<UAkAudioEvent>* Found = AttackSFXMap.Find(Result.ActionType))
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
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] PlayAttackMontage: ActionType=%d에 대한 SFX가 설정되지 않았습니다."), static_cast<int32>(Result.ActionType));
	}
}

void APTBBBPlayerActor::PlayDamageMontage_Implementation(const FPTBJudgementResult& Result)
{
	if (DamageMontage)
	{
		if (UAnimInstance* AnimInst = GetPlayerAnimInstance())
		{
			AnimInst->Montage_Play(DamageMontage);
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] PlayDamageMontage: DamageMontage가 설정되지 않았습니다."));
	}

	if (DamageSFX)
	{
		UAkGameplayStatics::PostEvent(DamageSFX, this, 0, FOnAkPostEventCallback(), false);
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] PlayDamageMontage: DamageSFX가 설정되지 않았습니다."));
	}
}

void APTBBBPlayerActor::PlayDeathMontage_Implementation(const FPTBJudgementResult& Result)
{
	if (bDeathMontagePlayed) return;
	bDeathMontagePlayed = true;

	if (!DeathMontage) { PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] DeathMontage 미설정")); return; }

	UAnimInstance* AnimInst = GetPlayerAnimInstance();
	if (!AnimInst)
	{
		return;
	}

	AnimInst->Montage_Play(DeathMontage);

	// 몽타주가 끝나며 자연스럽게 블렌드아웃되면 그 아래 AnimGraph(로코모션)가 다시 드러나
	// "쓰러졌다가 일어나는" 것처럼 보인다. 블렌드아웃이 "시작되는" 순간(아직 포즈가
	// 거의 그대로인 시점) 랙돌로 전환해서 그 자리에 쓰러진 채로 남게 한다.
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &APTBBBPlayerActor::HandleDeathMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DeathMontage);
}

void APTBBBPlayerActor::HandleDeathMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!PlayerMesh) return;

	PlayerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	PlayerMesh->SetSimulatePhysics(true);
	PlayerMesh->SetAllBodiesSimulatePhysics(true);
	PlayerMesh->bBlendPhysics = true;
	PlayerMesh->WakeAllRigidBodies();
}

void APTBBBPlayerActor::ApplyProfileCharacter_Implementation(const FPTBProfileData& Profile)
{
	if (!PlayerMesh)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] ApplyProfileCharacter: PlayerMesh가 없습니다."));
		return;
	}

	// Gender에 맞는 스켈레탈 메시 적용
	if (const TObjectPtr<USkeletalMesh>* FoundMesh = CharacterMeshByGender.Find(Profile.Gender))
	{
		if (USkeletalMesh* Mesh = FoundMesh->Get())
		{
			PlayerMesh->SetSkeletalMesh(Mesh);
		}
		else
		{
			PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] ApplyProfileCharacter: Gender=%d에 대한 메시가 null입니다."), static_cast<int32>(Profile.Gender));
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] ApplyProfileCharacter: Gender=%d에 대한 메시가 설정되지 않았습니다."), static_cast<int32>(Profile.Gender));
	}

	// Gender에 맞는 AnimBP 클래스 적용
	if (const TSubclassOf<UAnimInstance>* FoundAnimBP = AnimBlueprintByGender.Find(Profile.Gender))
	{
		if (*FoundAnimBP && PlayerMesh->GetAnimClass() != *FoundAnimBP)
		{
			PlayerMesh->SetAnimInstanceClass(*FoundAnimBP);
		}
		else
		{
			PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] ApplyProfileCharacter: Gender=%d에 대한 AnimBP가 null입니다."), static_cast<int32>(Profile.Gender));
		}
	}
	else
	{
		PTB_VERBOSE(LogPTBMiniGames, TEXT("[BBPlayerActor] ApplyProfileCharacter: Gender=%d에 대한 AnimBP가 설정되지 않았습니다."), static_cast<int32>(Profile.Gender));
	}
}

// ── 내부 헬퍼 ────────────────────────────────────────────────────

UAnimInstance* APTBBBPlayerActor::GetPlayerAnimInstance() const
{
	if (!PlayerMesh)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] GetPlayerAnimInstance: PlayerMesh가 없습니다."));
		return nullptr;
	}

	UAnimInstance* AnimInst = PlayerMesh->GetAnimInstance();
	if (!AnimInst)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBPlayerActor] GetPlayerAnimInstance: AnimInstance가 없습니다. AnimBP가 설정되어 있는지 확인하세요."));
	}
	return AnimInst;
}

// ── 델리게이트 핸들러 ────────────────────────────────────────────

AActor* APTBBBPlayerActor::ResolvePreferredViewTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return const_cast<APTBBBPlayerActor*>(this);
	}

	TArray<AActor*> TaggedActors;
	UGameplayStatics::GetAllActorsWithTag(World, TEXT("BBViewTarget"), TaggedActors);
	if (!TaggedActors.IsEmpty() && IsValid(TaggedActors[0]))
	{
		return TaggedActors[0];
	}

	UGameplayStatics::GetAllActorsWithTag(World, TEXT("PTB_BB_Camera"), TaggedActors);
	if (!TaggedActors.IsEmpty() && IsValid(TaggedActors[0]))
	{
		return TaggedActors[0];
	}

	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsOfClass(World, ACameraActor::StaticClass(), CameraActors);
	if (!CameraActors.IsEmpty() && IsValid(CameraActors[0]))
	{
		return CameraActors[0];
	}

	return const_cast<APTBBBPlayerActor*>(this);
}

void APTBBBPlayerActor::HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent)
{
	PlayAttackMontage(Result);
}

void APTBBBPlayerActor::HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent)
{
	if (PlayerHPPercent <= 0.0f)
	{
		PlayDeathMontage(Result);
		return;
	}

	PlayDamageMontage(Result);
}
