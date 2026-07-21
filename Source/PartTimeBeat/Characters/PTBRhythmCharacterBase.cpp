// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBRhythmCharacterBase.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

#include "AkComponent.h"
#include "AkAudioEvent.h"

APTBRhythmCharacterBase::APTBRhythmCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	AkComponent->SetupAttachment(GetRootComponent());
}

void APTBRhythmCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		AnimInstance = MeshComponent->GetAnimInstance();
	}
}

void APTBRhythmCharacterBase::PlaySuccessAnim()
{
	PlayCustomAnim(SuccessMontage);

	if (SuccessAkEvent && AkComponent)
	{
		AkComponent->PostAkEvent(SuccessAkEvent);
	}
}

void APTBRhythmCharacterBase::PlayFailAnim()
{
	PlayCustomAnim(FailMontage);

	if (FailAkEvent && AkComponent)
	{
		AkComponent->PostAkEvent(FailAkEvent);
	}
}

void APTBRhythmCharacterBase::PlayIdleAnim()
{
	PlayCustomAnim(IdleMontage);
}

void APTBRhythmCharacterBase::PlayCustomAnim(UAnimMontage* Montage)
{
	if (!AnimInstance || !Montage)
	{
		return;
	}

	AnimInstance->Montage_Play(Montage);
}

void APTBRhythmCharacterBase::PostCharacterSFX(FName EventKey)
{
	if (!AkComponent)
	{
		return;
	}

	// TODO:
	// EventKey 기반으로 캐릭터 SFX 테이블 또는 맵에서 UAkAudioEvent를 찾아 재생
	// 예: Success, Fail, Hit, Miss 등

	if (EventKey == TEXT("Success") && SuccessAkEvent)
	{
		AkComponent->PostAkEvent(SuccessAkEvent);
		return;
	}

	if (EventKey == TEXT("Fail") && FailAkEvent)
	{
		AkComponent->PostAkEvent(FailAkEvent);
		return;
	}
}

void APTBRhythmCharacterBase::HandleInput(FKey Key)
{
	if (TFunction<void()>* Fn = Actions.Find(Key))
	{
		(*Fn)();
	}
}

void APTBRhythmCharacterBase::BindAction(FKey Key, TFunction<void()> Fn)
{
	Actions.Add(Key, MoveTemp(Fn));
}

// 베이스 _Implementation은 비워둠 — 서브클래스에서 채움
void APTBRhythmCharacterBase::MoveLeft_Implementation() {}
void APTBRhythmCharacterBase::MoveRight_Implementation() {}
void APTBRhythmCharacterBase::MoveUp_Implementation() {}
void APTBRhythmCharacterBase::MoveDown_Implementation() {}
void APTBRhythmCharacterBase::Interact_Implementation() {}
void APTBRhythmCharacterBase::HitNoteA_Implementation() {}
void APTBRhythmCharacterBase::HitNoteW_Implementation() {}
void APTBRhythmCharacterBase::HitNoteS_Implementation() {}
void APTBRhythmCharacterBase::HitNoteD_Implementation() {}
void APTBRhythmCharacterBase::HitNoteEnter_Implementation() {}