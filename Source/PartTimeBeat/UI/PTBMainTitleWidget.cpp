// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBMainTitleWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Button.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UPTBMainTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();
    // 버튼 이벤트 연결 초기화
    InitializeView();

    // 3초 후 자동으로 타이틀 화면으로 전환
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TransitionTimerHandle,
            this,
            &UPTBMainTitleWidget::TransitionToTitleScreen,
            3.0f,
            false
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTBMainTitleWidget] NativeConstruct: World is null, timer not set"));
    }

	// 마우스 커서 표시 및 UI 전용 입력 모드 설정
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
}

void UPTBMainTitleWidget::NativeDestruct()
{
    Super::NativeDestruct();

    // 1번 문제 수정: 위젯 종료 시 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }
}

void UPTBMainTitleWidget::InitializeView()
{
    // 각 버튼이 유효한지 확인 후 클릭 이벤트 연결
    if (ButtonStart)
        ButtonStart->OnClicked.AddUniqueDynamic(this, &UPTBMainTitleWidget::OnStartClicked);
    if (ButtonSettings)
        ButtonSettings->OnClicked.AddUniqueDynamic(this, &UPTBMainTitleWidget::OnSettingsClicked);
    if (ButtonQuit)
        ButtonQuit->OnClicked.AddUniqueDynamic(this, &UPTBMainTitleWidget::OnQuitClicked);
    if (ButtonAchievement)
        ButtonAchievement->OnClicked.AddUniqueDynamic(this, &UPTBMainTitleWidget::OnAchievementClicked);
}

// 시작 버튼 클릭 시 프로필 선택 화면으로 이동
void UPTBMainTitleWidget::OnStartClicked()
{
    UPTBGameFlowSubsystem* FlowSystem = GetGameInstance()->GetSubsystem<UPTBGameFlowSubsystem>();
    if (FlowSystem)
        FlowSystem->SetFlowState(EGameFlowState::ProfileSelect);
}

// 설정 버튼 클릭 시 설정 화면으로 이동
void UPTBMainTitleWidget::OnSettingsClicked()
{
    UPTBGameFlowSubsystem* FlowSystem = GetGameInstance()->GetSubsystem<UPTBGameFlowSubsystem>();
    if (FlowSystem)
        FlowSystem->OpenSettings();
}

// 종료 버튼 클릭 시 게임 종료
void UPTBMainTitleWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

// 업적 버튼 클릭 시 추후 업적 화면 연동 예정
void UPTBMainTitleWidget::OnAchievementClicked()
{
    // TODO: 업적 화면 연동
}

void UPTBMainTitleWidget::TransitionToTitleScreen()
{
    if (!TitleScreenWidgetClass) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    UUserWidget* TitleScreen = CreateWidget<UUserWidget>(PC, TitleScreenWidgetClass);
    if (!TitleScreen) return;

    TitleScreen->AddToViewport();
    RemoveFromParent();
}
