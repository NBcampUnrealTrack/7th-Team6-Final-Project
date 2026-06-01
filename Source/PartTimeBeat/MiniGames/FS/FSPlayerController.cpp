// Fill out your copyright notice in the Description page of Project Settings.


#include "FSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MiniGames/FS/PTBFSMiniGame.h"
#include "Kismet/GameplayStatics.h"

void APTBFSPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (FishingMappingContext)
        {
            Subsystem->AddMappingContext(FishingMappingContext, 0);
        }
    }
}

void APTBFSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (IA_ActionA) EnhancedInput->BindAction(IA_ActionA, ETriggerEvent::Started, this, &APTBFSPlayerController::OnActionA);
        if (IA_ActionB) EnhancedInput->BindAction(IA_ActionB, ETriggerEvent::Started, this, &APTBFSPlayerController::OnActionB);
        if (IA_ActionC) EnhancedInput->BindAction(IA_ActionC, ETriggerEvent::Started, this, &APTBFSPlayerController::OnActionC);
        if (IA_ActionD) EnhancedInput->BindAction(IA_ActionD, ETriggerEvent::Started, this, &APTBFSPlayerController::OnActionD);
        if (IA_ActionE) EnhancedInput->BindAction(IA_ActionE, ETriggerEvent::Started, this, &APTBFSPlayerController::OnActionE);
    }
}

void APTBFSPlayerController::OnActionA(const FInputActionValue& Value)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APTBFSMiniGame::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        if (APTBFSMiniGame* MiniGame = Cast<APTBFSMiniGame>(FoundActors[0]))
        {
            MiniGame->HandleActionAInput();
        }
    }
}

void APTBFSPlayerController::OnActionB(const FInputActionValue& Value)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APTBFSMiniGame::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        if (APTBFSMiniGame* MiniGame = Cast<APTBFSMiniGame>(FoundActors[0]))
        {
            MiniGame->HandleActionBInput();
        }
    }
}

void APTBFSPlayerController::OnActionC(const FInputActionValue& Value)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APTBFSMiniGame::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        if (APTBFSMiniGame* MiniGame = Cast<APTBFSMiniGame>(FoundActors[0]))
        {
            MiniGame->HandleActionCInput();
        }
    }
}

void APTBFSPlayerController::OnActionD(const FInputActionValue& Value)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APTBFSMiniGame::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        if (APTBFSMiniGame* MiniGame = Cast<APTBFSMiniGame>(FoundActors[0]))
        {
            MiniGame->HandleActionDInput();
        }
    }
}

void APTBFSPlayerController::OnActionE(const FInputActionValue& Value)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APTBFSMiniGame::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        if (APTBFSMiniGame* MiniGame = Cast<APTBFSMiniGame>(FoundActors[0]))
        {
            MiniGame->HandleActionEInput();
        }
    }
}