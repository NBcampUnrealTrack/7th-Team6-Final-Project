#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FFSSkFish.generated.h"

USTRUCT(BlueprintType)
struct FFFSSkFish : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> FishMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> SwimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> IdleMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnWeight =0;
};