#include "PTBSRToppingSpawner.h"
#include "PTBSRMiniGame.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Debug/PTBLogChannels.h"

APTBSRToppingSpawner::APTBSRToppingSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APTBSRToppingSpawner::BeginPlay()
{
	Super::BeginPlay();

	TryBindMiniGame();
}

void APTBSRToppingSpawner::TryBindMiniGame()
{
	APTBSRMiniGame* MiniGame = Cast<APTBSRMiniGame>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APTBSRMiniGame::StaticClass()));

	if (!IsValid(MiniGame))
	{
		// 아직 미니게임 액터가 스폰되지 않은 경우, 잠시 후 재시도
		GetWorldTimerManager().SetTimer(
			BindRetryTimerHandle,
			this,
			&APTBSRToppingSpawner::TryBindMiniGame,
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
	MiniGame->OnToppingDrop.AddDynamic(this, &APTBSRToppingSpawner::HandleSushiPlateSpawn);

	UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] OnSushiPlateSpawn bound to %s"),
		*GetNameSafe(this), *GetNameSafe(MiniGame));
}

void APTBSRToppingSpawner::HandleSushiPlateSpawn(int32 AssignedTopping, int32 NoteId)
{
	SpawnTopping(AssignedTopping);
}

void APTBSRToppingSpawner::SpawnTopping(int32 AssignedTopping)
{
	if (!ToppingClass)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] SpawnTopping aborted: ToppingClass not set"), *GetNameSafe(this));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(GetActorLocation());

	// Deferred 스폰으로 Expose-on-Spawn 변수(ToppingID)를 BeginPlay 전에 세팅합니다.
	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(
		ToppingClass, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!SpawnedActor)
	{
		UE_LOG(LogPTBMiniGames, Error, TEXT("[%s] SpawnTopping failed to spawn %s"),
			*GetNameSafe(this), *GetNameSafe(ToppingClass));
		return;
	}

	if (FIntProperty* ToppingIdProp = FindFProperty<FIntProperty>(SpawnedActor->GetClass(), ToppingIdPropertyName))
	{
		ToppingIdProp->SetPropertyValue_InContainer(SpawnedActor, AssignedTopping);
	}
	else
	{
		UE_LOG(LogPTBMiniGames, Warning,
			TEXT("[%s] SpawnTopping: property '%s' not found on %s. ToppingID was not set."),
			*GetNameSafe(this), *ToppingIdPropertyName.ToString(), *GetNameSafe(ToppingClass));
	}

	SpawnedActor->FinishSpawning(SpawnTransform);
}