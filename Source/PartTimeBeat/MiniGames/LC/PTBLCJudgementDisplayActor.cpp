#include "PTBLCJudgementDisplayActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/PTBGameModeBase.h"
#include "Debug/PTBLogChannels.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APTBLCJudgementDisplayActor::APTBLCJudgementDisplayActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	LampMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMeshComponent"));
	LampMeshComponent->SetupAttachment(SceneRoot);

	JudgementLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("JudgementLightComponent"));
	JudgementLightComponent->SetupAttachment(SceneRoot);
	JudgementLightComponent->SetLightColor(DefaultLightColor);

	JudgementTextureComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JudgementTextureComponent"));
	JudgementTextureComponent->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		JudgementTextureComponent->SetStaticMesh(PlaneMeshFinder.Object);
	}

	JudgementLightColors.Add(EPTBJudgementType::HighPerfect, FLinearColor(1.0f, 0.84f, 0.0f));
	JudgementLightColors.Add(EPTBJudgementType::Perfect, FLinearColor(1.0f, 0.41f, 0.70f));
	JudgementLightColors.Add(EPTBJudgementType::Good, FLinearColor(0.4f, 0.86f, 0.4f));
	JudgementLightColors.Add(EPTBJudgementType::Miss, FLinearColor(0.9f, 0.1f, 0.1f));
}

void APTBLCJudgementDisplayActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] BeginPlay Actor=%s World=%s SourceMiniGame=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetWorld()),
		*GetNameSafe(SourceMiniGame.Get()));

	if (LampMeshComponent && LampMeshComponent->GetMaterial(0))
	{
		LampDynamicMaterial = LampMeshComponent->CreateDynamicMaterialInstance(0);
	}

	if (JudgementTextureComponent)
	{
		if (JudgementTextureMaterial)
		{
			JudgementTextureDynamicMaterial = JudgementTextureComponent->CreateDynamicMaterialInstance(0, JudgementTextureMaterial);
		}
		else if (JudgementTextureComponent->GetMaterial(0))
		{
			JudgementTextureDynamicMaterial = JudgementTextureComponent->CreateDynamicMaterialInstance(0);
		}
	}

	UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] MaterialReady Actor=%s LampMID=%s TextureMID=%s TextureMaterial=%s TextureParam=%s"),
		*GetNameSafe(this),
		*GetNameSafe(LampDynamicMaterial.Get()),
		*GetNameSafe(JudgementTextureDynamicMaterial.Get()),
		*GetNameSafe(JudgementTextureMaterial.Get()),
		*JudgementTextureParameterName.ToString());

	ResetJudgementDisplay();
	TryBindSourceMiniGame();

	if (APTBGameModeBase* GameMode = Cast<APTBGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnGameStarted.AddUniqueDynamic(this, &APTBLCJudgementDisplayActor::HandleGameStarted);
	}
}

void APTBLCJudgementDisplayActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		World->GetTimerManager().ClearTimer(ResetTimerHandle);
	}

	if (SourceMiniGame)
	{
		UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] Unbind Actor=%s SourceMiniGame=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SourceMiniGame.Get()));
		SourceMiniGame->OnMiniGameJudgement.RemoveDynamic(this, &APTBLCJudgementDisplayActor::HandleMiniGameJudgement);
	}

	if (APTBGameModeBase* GameMode = Cast<APTBGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnGameStarted.RemoveDynamic(this, &APTBLCJudgementDisplayActor::HandleGameStarted);
	}

	Super::EndPlay(EndPlayReason);
}

void APTBLCJudgementDisplayActor::HandleGameStarted()
{
	// 이전(파괴된) 미니게임 바인딩을 정리하고 새 ActiveMiniGame을 다시 조회한다
	if (SourceMiniGame)
	{
		SourceMiniGame->OnMiniGameJudgement.RemoveDynamic(this, &APTBLCJudgementDisplayActor::HandleMiniGameJudgement);
	}
	SourceMiniGame = nullptr;

	ResetJudgementDisplay();
	TryBindSourceMiniGame();
}

void APTBLCJudgementDisplayActor::ApplyJudgementResult(const FPTBJudgementResult& Result)
{
	UE_LOG(LogPTBMiniGames, Verbose, TEXT("[LCJudgementDisplay] ApplyJudgement Actor=%s NoteId=%d Judgement=%d Reason=%d Light=%s Texture=%s"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		*GetNameSafe(JudgementLightComponent.Get()),
		*GetNameSafe(JudgementTextureComponent.Get()));

	const FLinearColor* JudgementColor = JudgementLightColors.Find(Result.JudgementType);
	const FLinearColor AppliedColor = JudgementColor ? *JudgementColor : DefaultLightColor;
	if (JudgementLightComponent)
	{
		UE_LOG(LogPTBMiniGames, Verbose, TEXT("[LCJudgementDisplay] ApplyLight Actor=%s Judgement=%d HasColor=%d Color=(%.3f, %.3f, %.3f, %.3f)"),
			*GetNameSafe(this),
			static_cast<int32>(Result.JudgementType),
			JudgementColor != nullptr,
			AppliedColor.R,
			AppliedColor.G,
			AppliedColor.B,
			AppliedColor.A);
		JudgementLightComponent->SetLightColor(AppliedColor);
	}

	if (LampDynamicMaterial)
	{
		LampDynamicMaterial->SetVectorParameterValue(LampColorParameterName, AppliedColor);
	}

	if (JudgementTextureComponent)
	{
		TObjectPtr<UTexture2D>* JudgementTexture = JudgementTextures.Find(Result.JudgementType);
		UTexture2D* AppliedTexture = JudgementTexture ? JudgementTexture->Get() : DefaultJudgementTexture.Get();
		UE_LOG(LogPTBMiniGames, Verbose, TEXT("[LCJudgementDisplay] ApplyTexture Actor=%s Judgement=%d HasTexture=%d Texture=%s"),
			*GetNameSafe(this),
			static_cast<int32>(Result.JudgementType),
			JudgementTexture != nullptr && JudgementTexture->Get() != nullptr,
			*GetNameSafe(AppliedTexture));
		if (JudgementTextureDynamicMaterial && AppliedTexture)
		{
			JudgementTextureDynamicMaterial->SetTextureParameterValue(JudgementTextureParameterName, AppliedTexture);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetTimerHandle);
		if (ResetDelaySeconds > 0.0f)
		{
			World->GetTimerManager().SetTimer(ResetTimerHandle, this, &APTBLCJudgementDisplayActor::ResetJudgementDisplay, ResetDelaySeconds, false);
		}
	}
}

void APTBLCJudgementDisplayActor::HandleMiniGameJudgement(FPTBJudgementResult Result)
{
	UE_LOG(LogPTBMiniGames, Verbose, TEXT("[LCJudgementDisplay] ReceiveJudgement Actor=%s SourceMiniGame=%s NoteId=%d Judgement=%d Reason=%d"),
		*GetNameSafe(this),
		*GetNameSafe(SourceMiniGame.Get()),
		Result.NoteId,
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason));
	ApplyJudgementResult(Result);
}

void APTBLCJudgementDisplayActor::TryBindSourceMiniGame()
{
	SourceMiniGame = ResolveSourceMiniGame();
	if (SourceMiniGame)
	{
		SourceMiniGame->OnMiniGameJudgement.AddUniqueDynamic(this, &APTBLCJudgementDisplayActor::HandleMiniGameJudgement);
		UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] BindSuccess Actor=%s SourceMiniGame=%s Class=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SourceMiniGame.Get()),
			*GetNameSafe(SourceMiniGame->GetClass()));
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		}
		return;
	}

	if (UWorld* World = GetWorld())
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[LCJudgementDisplay] BindPending Actor=%s GameMode=%s"),
			*GetNameSafe(this),
			*GetNameSafe(UGameplayStatics::GetGameMode(this)));
		World->GetTimerManager().SetTimer(BindRetryTimerHandle, this, &APTBLCJudgementDisplayActor::TryBindSourceMiniGame, 0.1f, false);
	}
}

APTBBaseMiniGame* APTBLCJudgementDisplayActor::ResolveSourceMiniGame() const
{
	if (SourceMiniGame)
	{
		return SourceMiniGame.Get();
	}

	const APTBGameModeBase* GameMode = Cast<APTBGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[LCJudgementDisplay] ResolveSource failed: GameMode is not APTBGameModeBase. Actor=%s GameMode=%s"),
			*GetNameSafe(this),
			*GetNameSafe(UGameplayStatics::GetGameMode(this)));
		return nullptr;
	}

	UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] ResolveSource GameMode=%s ActiveMiniGame=%s ActiveClass=%s"),
		*GetNameSafe(GameMode),
		*GetNameSafe(GameMode->ActiveMiniGame),
		GameMode->ActiveMiniGame ? *GetNameSafe(GameMode->ActiveMiniGame->GetClass()) : TEXT("None"));
	return GameMode->ActiveMiniGame;
}

void APTBLCJudgementDisplayActor::ResetJudgementDisplay()
{
	UE_LOG(LogPTBMiniGames, Log, TEXT("[LCJudgementDisplay] Reset Actor=%s DefaultTexture=%s"),
		*GetNameSafe(this),
		*GetNameSafe(DefaultJudgementTexture.Get()));

	if (JudgementLightComponent)
	{
		JudgementLightComponent->SetLightColor(DefaultLightColor);
	}

	if (LampDynamicMaterial)
	{
		LampDynamicMaterial->SetVectorParameterValue(LampColorParameterName, DefaultLightColor);
	}

	if (JudgementTextureComponent)
	{
		if (JudgementTextureDynamicMaterial && DefaultJudgementTexture)
		{
			JudgementTextureDynamicMaterial->SetTextureParameterValue(JudgementTextureParameterName, DefaultJudgementTexture);
		}
	}
}
