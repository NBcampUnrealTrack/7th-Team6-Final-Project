#include "PTBDWTextPopup.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

APTBDWTextPopup::APTBDWTextPopup()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Anim = CreateDefaultSubobject<USceneComponent>(TEXT("Anim"));
	Anim->SetupAttachment(Root);
}

void APTBDWTextPopup::BeginPlay()
{
	Super::BeginPlay();
	EnsureSlots();
}

void APTBDWTextPopup::EnsureSlots()
{
	if (Slots.Num() >= MaxSlots)
	{
		return;
	}
	UStaticMesh* Placeholder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	for (int32 i = Slots.Num(); i < MaxSlots; ++i)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
		if (!Comp) { continue; }
		Comp->SetupAttachment(Anim);
		Comp->RegisterComponent();
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Placeholder) { Comp->SetStaticMesh(Placeholder); }
		Comp->SetRelativeScale3D(GlyphScale);
		Comp->SetVisibility(false);
		UMaterialInstanceDynamic* MID = GlyphMaterial
			? Comp->CreateDynamicMaterialInstance(0, GlyphMaterial)
			: Comp->CreateDynamicMaterialInstance(0);
		Slots.Add(Comp);
		SlotMIDs.Add(MID);
	}
}

UStaticMesh* APTBDWTextPopup::GlyphMeshFor(const FString& Ch) const
{
	if (const TObjectPtr<UStaticMesh>* Found = GlyphMeshes.Find(Ch))
	{
		if (*Found) { return *Found; }
	}
	return nullptr;
}

void APTBDWTextPopup::ShowGrade(EPTBJudgementType Grade)
{
	FString Word;
	FLinearColor Color;
	switch (Grade)
	{
	case EPTBJudgementType::HighPerfect:
	case EPTBJudgementType::Perfect: Word = TEXT("PERFECT"); Color = ColorPerfect; break;
	case EPTBJudgementType::Good:    Word = TEXT("GOOD");    Color = ColorGood;    break;
	default:                         Word = TEXT("MISS");    Color = ColorMiss;    break;
	}
	ShowWord(Word, Color);
}

void APTBDWTextPopup::ShowWord(const FString& Word, const FLinearColor& Color)
{
	EnsureSlots();
	const int32 N = FMath::Min(Word.Len(), Slots.Num());
	const float TotalW = (N > 0) ? (N - 1) * GlyphAdvance : 0.0f;
	UStaticMesh* Placeholder = nullptr;
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		UStaticMeshComponent* Comp = Slots[i];
		if (!Comp) { continue; }
		if (i < N)
		{
			const FString Ch = Word.Mid(i, 1).ToUpper();
			UStaticMesh* M = GlyphMeshFor(Ch);
			if (!M)
			{
				if (!Placeholder) { Placeholder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")); }
				M = Placeholder;
			}
			if (M) { Comp->SetStaticMesh(M); }
			Comp->SetRelativeScale3D(GlyphScale);
			Comp->SetRelativeLocation(FVector(i * GlyphAdvance - TotalW * 0.5f, 0.0f, 0.0f));
			Comp->SetVisibility(true);
			if (SlotMIDs.IsValidIndex(i) && SlotMIDs[i])
			{
				SlotMIDs[i]->SetVectorParameterValue(TEXT("Color"), Color);
			}
		}
		else
		{
			Comp->SetVisibility(false);
		}
	}
	ActiveCount = N;
	AnimTime = 0.0f;
	TotalAnim = PopInSec + HoldSec + RiseSec;
	if (Anim) { Anim->SetRelativeLocation(FVector::ZeroVector); Anim->SetRelativeScale3D(FVector::OneVector); }
}

void APTBDWTextPopup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (AnimTime < 0.0f || !Anim)
	{
		return;
	}
	AnimTime += DeltaSeconds;

	float ScaleMul = 1.0f;
	float RiseOffset = 0.0f;
	float Opacity = 1.0f;

	if (AnimTime < PopInSec)
	{
		const float t = (PopInSec > 0.0f) ? (AnimTime / PopInSec) : 1.0f;
		const float s = 1.0f - FMath::Pow(1.0f - t, 3.0f);
		ScaleMul = FMath::Lerp(0.2f, 1.0f, s);
	}
	else if (AnimTime < PopInSec + HoldSec)
	{
		ScaleMul = 1.0f;
	}
	else if (AnimTime < TotalAnim)
	{
		const float t = (RiseSec > 0.0f) ? ((AnimTime - PopInSec - HoldSec) / RiseSec) : 1.0f;
		RiseOffset = t * RiseZ;
		Opacity = 1.0f - t;
	}
	else
	{
		AnimTime = -1.0f;
		for (UStaticMeshComponent* C : Slots)
		{
			if (C) { C->SetVisibility(false); }
		}
		return;
	}

	Anim->SetRelativeScale3D(FVector(ScaleMul));
	Anim->SetRelativeLocation(FVector(0.0f, 0.0f, RiseOffset));
	for (int32 i = 0; i < ActiveCount && i < SlotMIDs.Num(); ++i)
	{
		if (SlotMIDs[i]) { SlotMIDs[i]->SetScalarParameterValue(TEXT("Opacity"), Opacity); }
	}
}
