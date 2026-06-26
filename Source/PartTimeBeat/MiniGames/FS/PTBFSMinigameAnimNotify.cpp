// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSMinigameAnimNotify.h"
#include "PTBFSCharacter.h"



void UPTBFSMinigameAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (APTBFSCharacter* Character = Cast<APTBFSCharacter>(MeshComp->GetOwner()))
	{
		Character->AttachFishingLine();
	}
}
