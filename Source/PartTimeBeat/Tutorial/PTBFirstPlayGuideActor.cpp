#include "Tutorial/PTBFirstPlayGuideActor.h"

#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Profile/PTBProfileSubsystem.h"
#include "UI/PTBFirstPlayGuideWidget.h"
#include "UObject/ConstructorHelpers.h"

namespace PTBFirstPlayGuideInternal
{
	// 플레이 기록이 생겼는지 확인하는 간격(초). 판 끝나고 허브로 돌아오면 BeginPlay에서 바로 판단하므로 길어도 됨
	constexpr float RecheckIntervalSec = 1.0f;
}

APTBFirstPlayGuideActor::APTBFirstPlayGuideActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(SceneRoot);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetCastShadow(false);
	ArrowMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.6f));
	ArrowMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		ArrowMesh->SetStaticMesh(ConeMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		ArrowMesh->SetMaterial(0, ShapeMaterial.Object);
	}
}

void APTBFirstPlayGuideActor::BeginPlay()
{
	Super::BeginPlay();

	if (ArrowMesh)
	{
		// 기본 원뿔 색 지정 (BasicShapeMaterial의 "Color" 파라미터)
		ArrowMaterial = ArrowMesh->CreateDynamicMaterialInstance(0);
		if (ArrowMaterial)
		{
			ArrowMaterial->SetVectorParameterValue(TEXT("Color"), ArrowColor);
		}
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FirstPlayGuide] %s: Target Actor가 비어 있어서 화살표/강조 없이 문구만 표시함"), *GetName());
	}

	RefreshGuide();
}

void APTBFirstPlayGuideActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetGuideActive(false);
	Super::EndPlay(EndPlayReason);
}

bool APTBFirstPlayGuideActor::ShouldShowGuide() const
{
	const UGameInstance* GI = GetGameInstance();
	const UPTBProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	if (!Profiles)
	{
		return false;
	}

	bool bHasActive = false;
	const FPTBProfileData Profile = Profiles->GetActiveProfile(bHasActive);
	if (!bHasActive)
	{
		return false; // 프로필 선택 전에는 표시 안 함
	}

	// 어떤 미니게임·난이도든 결과 화면까지 간 기록이 하나라도 있으면 안내 끝
	for (const TPair<FName, int32>& Pair : Profile.PlayCountByDifficulty)
	{
		if (Pair.Value > 0)
		{
			return false;
		}
	}
	return true;
}

void APTBFirstPlayGuideActor::RefreshGuide()
{
	SetGuideActive(ShouldShowGuide());
}

void APTBFirstPlayGuideActor::SetGuideActive(bool bActive)
{
	if (bGuideActive == bActive)
	{
		return;
	}
	bGuideActive = bActive;
	AnimTime = 0.f;

	// ② 화살표
	if (ArrowMesh)
	{
		ArrowMesh->SetHiddenInGame(!bActive || !TargetActor);
	}

	// ③ 강조
	ApplyHighlight(bActive);

	// ① 화면 문구
	if (bActive)
	{
		if (!HintWidget)
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
			const TSubclassOf<UPTBFirstPlayGuideWidget> WidgetClass = HintWidgetClass ? HintWidgetClass : TSubclassOf<UPTBFirstPlayGuideWidget>(UPTBFirstPlayGuideWidget::StaticClass());
			HintWidget = PC ? CreateWidget<UPTBFirstPlayGuideWidget>(PC, WidgetClass) : nullptr;
		}
		if (HintWidget && !HintWidget->IsInViewport())
		{
			HintWidget->AddToViewport(HintZOrder);
		}
		UpdateHintVisibility();
	}
	else if (HintWidget)
	{
		HintWidget->RemoveFromParent();
		HintWidget = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("[FirstPlayGuide] 첫 플레이 안내 %s"), bActive ? TEXT("표시") : TEXT("숨김"));
}

void APTBFirstPlayGuideActor::ApplyHighlight(bool bEnable)
{
	if (bEnable)
	{
		if (!HighlightOverlayMaterial || !TargetActor)
		{
			return;
		}
		TArray<UMeshComponent*> Meshes;
		TargetActor->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			if (Mesh && Mesh->IsVisible())
			{
				SavedOverlayMaterials.Add(Mesh, Mesh->GetOverlayMaterial());
				Mesh->SetOverlayMaterial(HighlightOverlayMaterial);
			}
		}
		return;
	}

	for (const TPair<TWeakObjectPtr<UMeshComponent>, TObjectPtr<UMaterialInterface>>& Pair : SavedOverlayMaterials)
	{
		if (UMeshComponent* Mesh = Pair.Key.Get())
		{
			Mesh->SetOverlayMaterial(Pair.Value);
		}
	}
	SavedOverlayMaterials.Reset();
}

FVector APTBFirstPlayGuideActor::GetTargetTopLocation() const
{
	if (!TargetActor)
	{
		return GetActorLocation();
	}
	FVector Origin;
	FVector Extent;
	TargetActor->GetActorBounds(/*bOnlyCollidingComponents*/ false, Origin, Extent);
	return FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z);
}

void APTBFirstPlayGuideActor::UpdateArrow(float DeltaSeconds)
{
	if (!ArrowMesh || !TargetActor)
	{
		return;
	}

	const float Bob = ArrowBobAmplitude * FMath::Sin(AnimTime * 2.f * PI / ArrowBobPeriodSec);
	const FVector Location = GetTargetTopLocation() + FVector(0.f, 0.f, ArrowHeightAboveTarget + Bob);

	// 원뿔이 아래(대상)를 가리키도록 뒤집고, 제자리에서 천천히 회전
	const FRotator Rotation(180.f, FMath::Fmod(AnimTime * ArrowSpinDegPerSec, 360.f), 0.f);
	ArrowMesh->SetWorldLocationAndRotation(Location, Rotation);
}

void APTBFirstPlayGuideActor::UpdateHintVisibility()
{
	if (!HintWidget)
	{
		return;
	}

	bool bNear = false;
	if (TargetActor && HideHintWithinDistance > 0.f)
	{
		if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			bNear = FVector::Dist2D(Pawn->GetActorLocation(), TargetActor->GetActorLocation()) <= HideHintWithinDistance;
		}
	}

	const ESlateVisibility Wanted = bNear ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;
	if (HintWidget->GetVisibility() != Wanted)
	{
		HintWidget->SetVisibility(Wanted);
		if (!bNear)
		{
			HintWidget->RefreshHintText(); // 숨겨진 사이 키 설정이 바뀌었을 수 있음
		}
	}
}

void APTBFirstPlayGuideActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 프로필 변경·플레이 기록 생성 감지 (가벼운 확인을 1초마다)
	RecheckTimer -= DeltaSeconds;
	if (RecheckTimer <= 0.f)
	{
		RecheckTimer = PTBFirstPlayGuideInternal::RecheckIntervalSec;
		RefreshGuide();
	}

	if (!bGuideActive)
	{
		return;
	}

	AnimTime += DeltaSeconds;
	UpdateArrow(DeltaSeconds);
	UpdateHintVisibility();
}
