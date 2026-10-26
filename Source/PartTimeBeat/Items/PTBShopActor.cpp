#include "Items/PTBShopActor.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Items/PTBHubUISubsystem.h"
#include "UI/PTBItemPopupWidget.h"

APTBShopActor::APTBShopActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InteractVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractVolume"));
	InteractVolume->SetupAttachment(Root);
	InteractVolume->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	InteractVolume->SetCollisionProfileName(TEXT("Trigger"));
	InteractVolume->SetGenerateOverlapEvents(true);

	PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
	PromptWidget->SetupAttachment(Root);
	PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PromptWidget->SetDrawAtDesiredSize(true);
	PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	PromptWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PromptWidget->SetHiddenInGame(true);

	ShopName = NSLOCTEXT("PTBItem", "DefaultShopName", "상점");
}

void APTBShopActor::BeginPlay()
{
	Super::BeginPlay();

	InteractVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &APTBShopActor::HandleBeginOverlap);
	InteractVolume->OnComponentEndOverlap.AddUniqueDynamic(this, &APTBShopActor::HandleEndOverlap);

	// 레벨 시작 시 이미 범위 안에 서 있는 경우 (허브 복귀 위치가 상점 앞인 경우 등)
	TArray<AActor*> Overlapping;
	InteractVolume->GetOverlappingActors(Overlapping, APawn::StaticClass());
	for (const AActor* Actor : Overlapping)
	{
		if (IsLocalPlayerPawn(Actor))
		{
			SetFocused(true);
			break;
		}
	}
}

void APTBShopActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetFocused(false);
	Super::EndPlay(EndPlayReason);
}

bool APTBShopActor::OnHubInteract_Implementation(APlayerController* Player)
{
	UPTBHubUISubsystem* HubUI = GetWorld() ? GetWorld()->GetSubsystem<UPTBHubUISubsystem>() : nullptr;
	return HubUI && HubUI->OpenShop(ShopName, SoldCategories, ShopWidgetClassOverride);
}

void APTBShopActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsLocalPlayerPawn(OtherActor))
	{
		SetFocused(true);
	}
}

void APTBShopActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsLocalPlayerPawn(OtherActor))
	{
		SetFocused(false);
	}
}

bool APTBShopActor::IsLocalPlayerPawn(const AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return PC && PC->IsLocalController();
}

void APTBShopActor::SetFocused(bool bInFocused)
{
	if (bFocused == bInFocused)
	{
		return;
	}
	bFocused = bInFocused;

	if (UPTBHubUISubsystem* HubUI = GetWorld() ? GetWorld()->GetSubsystem<UPTBHubUISubsystem>() : nullptr)
	{
		if (bFocused)
		{
			HubUI->SetInteractTarget(this);
		}
		else
		{
			HubUI->ClearInteractTarget(this);
		}
	}

	if (PromptWidget)
	{
		PromptWidget->SetHiddenInGame(!bFocused);
	}

	OnPlayerFocusChanged(bFocused);
}
