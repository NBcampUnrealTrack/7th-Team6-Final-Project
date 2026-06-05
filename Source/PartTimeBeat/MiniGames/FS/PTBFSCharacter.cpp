// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSCharacter.h"

#include "Debug/PTBTeamLog.h"


// Sets default values
APTBFSCharacter::APTBFSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void APTBFSCharacter::OnPlayCastAnimMontage()
{
	if (CastAnimMontage)
	{
		PlayAnimMontage(CastAnimMontage);
	}
}

void APTBFSCharacter::OnPlayRealAnimMontage()
{
	if (RealAnimMontage)
	{
		PlayAnimMontage(RealAnimMontage);
	}
}



