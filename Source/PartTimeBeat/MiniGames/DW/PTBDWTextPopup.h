#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBDWTextPopup.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** 판정 등급 3D 글자 팝업(PERFECT/GOOD/MISS). */
UCLASS()
class PARTTIMEBEAT_API APTBDWTextPopup : public AActor
{
	GENERATED_BODY()

public:
	APTBDWTextPopup();

	/** 판정 등급을 단어로 표시(PERFECT, GOOD, MISS). */
	void ShowGrade(EPTBJudgementType Grade);

	/** 임의 단어 표시(대문자 글리프). */
	void ShowWord(const FString& Word, const FLinearColor& Color);

	/** 글리프 문자 -> 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	TMap<FString, TObjectPtr<UStaticMesh>> GlyphMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	TObjectPtr<UMaterialInterface> GlyphMaterial = nullptr;

	/** 글자 하나 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	FVector GlyphScale = FVector(0.5f, 0.5f, 0.5f);

	/** 글자 간 X 간격. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	float GlyphAdvance = 60.0f;

	/** 등급별 색(머티리얼 "Color" 파라미터). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	FLinearColor ColorPerfect = FLinearColor(1.0f, 0.85f, 0.2f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	FLinearColor ColorGood = FLinearColor(0.4f, 0.8f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	FLinearColor ColorMiss = FLinearColor(1.0f, 0.3f, 0.3f);

	/** 팝 애니: 스케일 인 / 유지 / 상승·페이드. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	float PopInSec = 0.12f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	float HoldSec = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	float RiseSec = 0.25f;

	/** 페이드 동안 위로 뜨는 높이. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	float RiseZ = 40.0f;

	/** 글자 슬롯 풀 크기. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|Text")
	int32 MaxSlots = 8;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<USceneComponent> Anim;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Slots;
	UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> SlotMIDs;

	void EnsureSlots();
	UStaticMesh* GlyphMeshFor(const FString& Ch) const;

	float AnimTime = -1.0f;
	float TotalAnim = 0.0f;
	int32 ActiveCount = 0;
};
