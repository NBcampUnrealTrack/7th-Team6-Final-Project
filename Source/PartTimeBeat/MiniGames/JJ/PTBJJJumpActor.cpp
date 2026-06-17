#include "Actor/PTBJJJumpActor.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

APTBJJJumpActor::APTBJJJumpActor()
{
	PrimaryActorTick.bCanEverTick = true;

	JumpRoot = CreateDefaultSubobject<USceneComponent>(TEXT("JumpRoot"));
	RootComponent = JumpRoot;

	// 판정 색 Plane: 루트 직속(점프와 분리) → 캐릭터가 떠도 바닥에 고정
	JudgePlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JudgePlane"));
	JudgePlane->SetupAttachment(RootComponent);
	JudgePlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APTBJJJumpActor::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = JumpRoot->GetRelativeLocation();

	// 동적 머티리얼 생성(런타임 색 변경용) + 기본색 세팅
	if (JudgePlane && JudgePlane->GetMaterial(0))
	{
		PlaneMaterial = JudgePlane->CreateDynamicMaterialInstance(0);
		if (PlaneMaterial)
		{
			PlaneMaterial->SetVectorParameterValue(ColorParamName, DefaultPlaneColor);
		}
	}
}

void APTBJJJumpActor::StartJump(float AirtimeMs, float HeightScale)
{
	// 체공시간이 비정상이면 무시
	if (AirtimeMs <= 0.0f)
	{
		return;
	}

	// 이미 점프 중이어도 새 점프로 덮어쓴다(짧은 간격 연타 대응).
	// 단, 현재 위치가 아니라 항상 기준 위치에서 다시 시작해 누적 드리프트를 막는다.
	bIsJumping = true;
	Elapsed = 0.0f;
	Duration = AirtimeMs / 1000.0f;
	CurrentHeight = JumpHeight * FMath::Max(HeightScale, 0.0f);

	FVector Loc = BaseLocation;
	JumpRoot->SetRelativeLocation(Loc);

	OnJumpStarted();
}

void APTBJJJumpActor::FlashJudgementColor(EPTBJudgementType JudgementType)
{
	switch (JudgementType)
	{
	case EPTBJudgementType::HighPerfect: FlashColor = FLinearColor(1.0f, 0.84f, 0.0f); break; // 금색
	case EPTBJudgementType::Perfect:     FlashColor = FLinearColor(1.0f, 0.41f, 0.7f); break; // 분홍
	case EPTBJudgementType::Good:        FlashColor = FLinearColor(0.4f, 0.86f, 0.4f); break; // 초록
	case EPTBJudgementType::Miss:        FlashColor = FLinearColor(0.9f, 0.1f, 0.1f); break;  // 빨강
	default:                             FlashColor = DefaultPlaneColor;                break;
	}

	FlashElapsed = 0.0f;
	bFlashing = true;

	// 즉시 시작색 반영(첫 프레임부터 확 들어오게)
	if (PlaneMaterial)
	{
		PlaneMaterial->SetVectorParameterValue(ColorParamName, FlashColor);
	}
}

void APTBJJJumpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ── 점프 처리 ──
	if (bIsJumping)
	{
		Elapsed += DeltaTime;

		const float Alpha = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);

		// 포물선: 4 * h * t * (1 - t) → t=0,1 에서 0, t=0.5 에서 최고점 h
		const float ZOffset = 4.0f * CurrentHeight * Alpha * (1.0f - Alpha);

		FVector Loc = BaseLocation;
		Loc.Z += ZOffset;
		JumpRoot->SetRelativeLocation(Loc);

		if (Alpha >= 1.0f)
		{
			// 정확히 체공시간 경과 → 착지
			bIsJumping = false;
			JumpRoot->SetRelativeLocation(BaseLocation);
			OnLanded();
		}
	}

	// ── 판정 색 플래시 → 기본색 보간 ──
	if (bFlashing && PlaneMaterial)
	{
		FlashElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(FlashElapsed / FMath::Max(FlashDuration, 0.0001f), 0.0f, 1.0f);

		// 시작색(FlashColor) → 기본색으로 보간
		const FLinearColor Current = FMath::Lerp(FlashColor, DefaultPlaneColor, Alpha);
		PlaneMaterial->SetVectorParameterValue(ColorParamName, Current);

		if (Alpha >= 1.0f)
		{
			bFlashing = false;   // 복귀 완료
		}
	}
}