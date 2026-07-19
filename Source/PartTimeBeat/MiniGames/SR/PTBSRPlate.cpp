#include "PTBSRPlate.h"
#include "PTBSRMiniGame.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"

APTBSRPlate::APTBSRPlate()
{
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	// 기존 BP의 Box 콜리전 설정을 재현: 겹침만 감지, 물리 충돌은 없음
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	Box->InitBoxExtent(FVector(50.0f, 50.0f, 50.0f));
}

void APTBSRPlate::BeginPlay()
{
	Super::BeginPlay();

	if (Box)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &APTBSRPlate::HandleBoxBeginOverlap);
	}

	// Retry로 미니게임 Actor가 교체돼도 정리되지 않는 걸 막기 위해 스폰 시점에 자기 자신을 등록
	if (APTBSRMiniGame* MiniGame = FindMiniGame())
	{
		MiniGame->RegisterSpawnedRoundActor(this);
	}
}

void APTBSRPlate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 기존 BP: MakeVector(0, MoveSpeed * DeltaSeconds, 0) → AddActorLocalOffset
	const FVector DeltaLocation(0.0f, MoveSpeed * DeltaTime, 0.0f);
	AddActorLocalOffset(DeltaLocation, false);
}

bool APTBSRPlate::TryGetNoteIdFromTopping(AActor* ToppingActor, int32& OutNoteId) const
{
	if (!ToppingActor)
	{
		return false;
	}

	if (const FIntProperty* NoteIdProp = FindFProperty<FIntProperty>(ToppingActor->GetClass(), ToppingNoteIdPropertyName))
	{
		OutNoteId = NoteIdProp->GetPropertyValue_InContainer(ToppingActor);
		return true;
	}

	PTB_WARNING(LogPTBMiniGames, TEXT("[%s] TryGetNoteIdFromTopping: [%s]에서 [%s] 프로퍼티를 찾지 못했습니다."),
		*GetNameSafe(this), *GetNameSafe(ToppingActor), *ToppingNoteIdPropertyName.ToString());
	return false;
}

TSubclassOf<AActor> APTBSRPlate::GetSushiClassFromTopping(AActor* ToppingActor) const
{
	if (!ToppingActor)
	{
		return nullptr;
	}

	// class 타입 핀(TSubclassOf<AActor> 등)은 FClassProperty로 저장됩니다.
	if (const FClassProperty* ClassProp = FindFProperty<FClassProperty>(ToppingActor->GetClass(), ToppingSushiClassPropertyName))
	{
		UObject* ClassValue = ClassProp->GetPropertyValue_InContainer(ToppingActor);
		return TSubclassOf<AActor>(Cast<UClass>(ClassValue));
	}

	PTB_WARNING(LogPTBMiniGames, TEXT("[%s] GetSushiClassFromTopping: [%s]에서 [%s] 프로퍼티를 찾지 못했습니다."),
		*GetNameSafe(this), *GetNameSafe(ToppingActor), *ToppingSushiClassPropertyName.ToString());
	return nullptr;
}

APTBSRMiniGame* APTBSRPlate::FindMiniGame() const
{
	return Cast<APTBSRMiniGame>(UGameplayStatics::GetActorOfClass(GetWorld(), APTBSRMiniGame::StaticClass()));
}

void APTBSRPlate::PlayCompletionEffects(TSubclassOf<AActor> SushiClass)
{
	if (CompletionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			CompletionVFX,
			GetActorLocation(),
			GetActorRotation(),
			FVector(1.0f),
			/*bAutoDestroy=*/true,
			/*bAutoActivate=*/true,
			ENCPoolMethod::None,
			/*bPreCullCheck=*/true);
	}

	if (SushiClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActor* CompletedSushi = World->SpawnActor<AActor>(SushiClass, GetActorTransform(), SpawnParams);

			// Retry 시 정리되도록 등록
			if (CompletedSushi)
			{
				if (APTBSRMiniGame* MiniGame = FindMiniGame())
				{
					MiniGame->RegisterSpawnedRoundActor(CompletedSushi);
				}
			}
		}
	}
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] PlayCompletionEffects: SushiClass가 유효하지 않아 완성 스시 모델을 스폰하지 못했습니다."),
			*GetNameSafe(this));
	}
}

void APTBSRPlate::HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// ToppingNoteIdPropertyName을 가진 액터(=토핑)인지 확인
	const FIntProperty* NoteIdProp = FindFProperty<FIntProperty>(OtherActor->GetClass(), ToppingNoteIdPropertyName);
	if (!NoteIdProp)
	{
		return;
	}

	const int32 ToppingNoteId = NoteIdProp->GetPropertyValue_InContainer(OtherActor);

	// ★ 이 토핑이 "판정 성공, 완성 대기 중"이었다면 지금(=실제로 컨베이어에 닿는 순간)
	//   완성 비주얼이 트리거됩니다. 그렇지 않다면(Miss였거나 이미 처리됨) 그냥 치워줍니다.
	bool bHandledByMiniGame = false;
	if (APTBSRMiniGame* MiniGame = Cast<APTBSRMiniGame>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APTBSRMiniGame::StaticClass())))
	{
		bHandledByMiniGame = MiniGame->NotifyToppingReachedConveyor(ToppingNoteId);
	}

	if (!bHandledByMiniGame)
	{
		OtherActor->Destroy();
	}
}