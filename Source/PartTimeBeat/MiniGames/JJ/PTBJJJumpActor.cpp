#include "MiniGames/JJ/PTBJJJumpActor.h"

APTBJJJumpActor::APTBJJJumpActor()
{
	PrimaryActorTick.bCanEverTick = true;

	JumpRoot = CreateDefaultSubobject<USceneComponent>(TEXT("JumpRoot"));
	RootComponent = JumpRoot;
}

void APTBJJJumpActor::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = JumpRoot->GetRelativeLocation();
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

void APTBJJJumpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsJumping)
	{
		return;
	}

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
