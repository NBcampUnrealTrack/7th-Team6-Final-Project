#include "MiniGames/DW/PTBDWBackgroundScroller.h"

APTBDWBackgroundScroller::APTBDWBackgroundScroller()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APTBDWBackgroundScroller::BeginPlay()
{
	Super::BeginPlay();

	BaseScrollSpeed = ScrollSpeed;
	BaseLocation = GetActorLocation();
	LaneDir = GetActorForwardVector();
	SpawnChunks();
	bRunning = bAutoStart;
}

TSubclassOf<AActor> APTBDWBackgroundScroller::PickChunkClass() const
{
	TArray<TSubclassOf<AActor>> Valid;
	for (const TSubclassOf<AActor>& C : ChunkClasses)
	{
		if (C) { Valid.Add(C); }
	}
	if (Valid.Num() == 0)
	{
		return nullptr;
	}
	return Valid[FMath::RandRange(0, Valid.Num() - 1)];
}

void APTBDWBackgroundScroller::SpawnChunks()
{
	UWorld* World = GetWorld();
	if (!World || ChunkCount <= 0)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < ChunkCount; ++i)
	{
		const float Offset = (i - ChunksBehind) * ChunkLength;

		TSubclassOf<AActor> Cls = PickChunkClass();
		AActor* Chunk = Cls
			? World->SpawnActor<AActor>(Cls, BaseLocation + LaneDir * Offset, GetActorRotation(), Params)
			: nullptr;

		ActiveChunks.Add(Chunk);
		ChunkOffsets.Add(Offset);
	}
}

void APTBDWBackgroundScroller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bRunning || ActiveChunks.Num() == 0)
	{
		return;
	}

	const float Move = ScrollSpeed * DeltaTime;
	for (float& Off : ChunkOffsets)
	{
		Off -= Move;
	}

	float MaxOff = -FLT_MAX;
	for (float Off : ChunkOffsets)
	{
		MaxOff = FMath::Max(MaxOff, Off);
	}
	for (int32 i = 0; i < ChunkOffsets.Num(); ++i)
	{
		if (ChunkOffsets[i] < -(ChunksBehind + 1) * ChunkLength)
		{
			MaxOff += ChunkLength;
			ChunkOffsets[i] = MaxOff;
		}
	}

	for (int32 i = 0; i < ActiveChunks.Num(); ++i)
	{
		if (ActiveChunks[i])
		{
			ActiveChunks[i]->SetActorLocation(BaseLocation + LaneDir * ChunkOffsets[i]);
		}
	}
}
