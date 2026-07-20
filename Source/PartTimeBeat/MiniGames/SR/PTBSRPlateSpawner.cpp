#include "PTBSRPlateSpawner.h"
#include "PTBSRMiniGame.h"
#include "PTBSRPlate.h"
#include "Core/PTBGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Debug/PTBLogChannels.h"

APTBSRPlateSpawner::APTBSRPlateSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APTBSRPlateSpawner::BeginPlay()
{
	Super::BeginPlay();

	TryBindMiniGame();

	if (APTBGameModeBase* GameMode = Cast<APTBGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnGameStarted.AddUniqueDynamic(this, &APTBSRPlateSpawner::HandleGameStarted);
	}
}

void APTBSRPlateSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BindRetryTimerHandle);

	if (IsValid(BoundMiniGame))
	{
		BoundMiniGame->OnSushiPlateSpawn.RemoveDynamic(this, &APTBSRPlateSpawner::HandleSushiPlateSpawn);
		BoundMiniGame = nullptr;
	}

	if (APTBGameModeBase* GameMode = Cast<APTBGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnGameStarted.RemoveDynamic(this, &APTBSRPlateSpawner::HandleGameStarted);
	}

	Super::EndPlay(EndPlayReason);
}

void APTBSRPlateSpawner::HandleGameStarted()
{
	BoundMiniGame = nullptr;
	TryBindMiniGame();
}

void APTBSRPlateSpawner::TryBindMiniGame()
{
	APTBSRMiniGame* MiniGame = Cast<APTBSRMiniGame>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APTBSRMiniGame::StaticClass()));

	if (!IsValid(MiniGame))
	{
		// 아직 미니게임 액터가 스폰되지 않은 경우, 잠시 후 재시도
		GetWorldTimerManager().SetTimer(
			BindRetryTimerHandle,
			this,
			&APTBSRPlateSpawner::TryBindMiniGame,
			BindRetryIntervalSeconds,
			false);
		return;
	}

	if (BoundMiniGame == MiniGame)
	{
		// 이미 같은 액터에 바인딩되어 있으면 중복 바인딩 방지
		return;
	}

	BoundMiniGame = MiniGame;
	MiniGame->OnSushiPlateSpawn.AddDynamic(this, &APTBSRPlateSpawner::HandleSushiPlateSpawn);

	UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] OnSushiPlateSpawn bound to %s"),
		*GetNameSafe(this), *GetNameSafe(MiniGame));
}

void APTBSRPlateSpawner::HandleSushiPlateSpawn(int32 ToppingType, int32 NoteId)
{
	SpawnPlate(ToppingType, NoteId);
}

void APTBSRPlateSpawner::SpawnPlate(int32 ToppingType, int32 NoteId)
{
	if (!PlateClass)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] SpawnPlate aborted: PlateClass not set"), *GetNameSafe(this));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform SpawnTransform(GetActorLocation());

	// Deferred 스폰으로 AssignedNoteId를 BeginPlay 전에 세팅합니다.
	APTBSRPlate* SpawnedPlate = World->SpawnActorDeferred<APTBSRPlate>(
		PlateClass, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!SpawnedPlate)
	{
		UE_LOG(LogPTBMiniGames, Error, TEXT("[%s] SpawnPlate failed to spawn %s"),
			*GetNameSafe(this), *GetNameSafe(PlateClass));
		return;
	}

	// Retry 시 정리되도록 등록 (ToppingSpawner와 동일한 패턴)
	if (IsValid(BoundMiniGame))
	{
		BoundMiniGame->RegisterSpawnedRoundActor(SpawnedPlate);
	}

	// PTBSRPlate는 우리 C++ 클래스이므로 리플렉션 없이 바로 세팅 가능
	SpawnedPlate->AssignedNoteId = NoteId;

	// ★ MiniGame에 설정된 접시 이동 속도를 그대로 적용합니다.
	//   (이전엔 이 값이 연결되어 있지 않아 Plate 기본값(-100)으로만 움직였습니다.)
	if (BoundMiniGame)
	{
		SpawnedPlate->MoveSpeed = BoundMiniGame->GetPlateMoveSpeed();
	}

	SpawnedPlate->FinishSpawning(SpawnTransform);

	// ★ 판정 성공 시, 물리적 접촉과 무관하게 "이 노트를 담당하던 바로 그 접시"를 찾을 수 있도록 등록
	if (BoundMiniGame)
	{
		BoundMiniGame->RegisterActivePlate(NoteId, SpawnedPlate);
	}
}