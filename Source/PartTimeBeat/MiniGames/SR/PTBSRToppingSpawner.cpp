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
	MiniGame->OnSushiSuccessDelegate.AddDynamic(this, &APTBSRToppingSpawner::HandleSushiSuccess);

	// ★ "남은 토핑" 카운트를 여기서(=바인딩이 성공한, 즉 채보/ToppingQueue가 이미 구성된 시점에)
	//   초기화합니다. 난이도에 따라 실제로 다른 값이 들어갑니다 (기존에는 위젯의 PreConstruct가
	//   너무 이른 시점에 세팅해서 항상 예전 값(26)으로 고정되어 있었습니다).
	if (FIntProperty* CountProp = FindFProperty<FIntProperty>(GetClass(), TEXT("count")))
	{
		CountProp->SetPropertyValue_InContainer(this, MiniGame->GetTotalToppingCount());
	}
	else
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] TryBindMiniGame: 'count' 프로퍼티를 찾지 못해 남은 토핑 초기화를 못 했습니다."),
			*GetNameSafe(this));
	}

	UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] OnSushiPlateSpawn bound to %s"),
		*GetNameSafe(this), *GetNameSafe(MiniGame));
}

void APTBSRToppingSpawner::HandleSushiPlateSpawn(int32 AssignedTopping, int32 NoteId)
{
	SpawnTopping(AssignedTopping, NoteId);
}

void APTBSRToppingSpawner::SpawnTopping(int32 AssignedTopping, int32 NoteId)
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
	// NoteId도 같이 세팅
	if (FIntProperty* NoteIdProp = FindFProperty<FIntProperty>(SpawnedActor->GetClass(), TEXT("NoteId")))
	{
		NoteIdProp->SetPropertyValue_InContainer(SpawnedActor, NoteId);
	}

	SpawnedActor->FinishSpawning(SpawnTransform);

	// ★ 판정 성공 시 물리적 접시 접촉과 무관하게 이 토핑을 바로 완성 처리할 수 있도록 등록
	if (BoundMiniGame)
	{
		BoundMiniGame->RegisterActiveTopping(NoteId, SpawnedActor);
	}
}

void APTBSRToppingSpawner::HandleSushiSuccess(int32 NoteId, EPTBJudgementType JudgementType)
{
	// ★ "남은 토핑" 카운트는 판정이 성공했을 때만 감소합니다 (틀리면 토핑은 소모됐어도
	//   재료 자체는 낭비되지 않은 것으로 취급 — 게임 디자인 의도에 맞춰 조정 가능).
	if (FIntProperty* CountProp = FindFProperty<FIntProperty>(GetClass(), TEXT("count")))
	{
		const int32 CurrentCount = CountProp->GetPropertyValue_InContainer(this);
		CountProp->SetPropertyValue_InContainer(this, FMath::Max(0, CurrentCount - 1));
	}
	else
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] HandleSushiSuccess: 'count' 프로퍼티를 찾지 못해 남은 토핑 표시가 갱신되지 않습니다."),
			*GetNameSafe(this));
	}
}