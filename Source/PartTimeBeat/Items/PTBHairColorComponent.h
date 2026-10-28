#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTBHairColorComponent.generated.h"

class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

/**
 * 장착한 헤어 아이템 색을 캐릭터 머리 머티리얼에 실시간 적용.
 *
 * 사용법: 허브 캐릭터 BP(PTB_Girl / PTB_Boy를 쓰는 캐릭터)에 컴포넌트로 추가하기만 하면 됨
 *  - BeginPlay 다음 프레임에 적용 (BP BeginPlay에서 성별에 따라 메시를 바꾸는 경우도 반영)
 *  - 헤어 장착이 바뀌거나 프로필이 바뀌면 자동으로 다시 적용
 *  - 메시를 나중에 바꾸면 BP에서 ReapplyHairColor 호출
 *
 * 머리 머티리얼(Hair_UV)에 필요한 파라미터
 *  - HairColor        (Vector) : 아이템 Color
 *  - HairTintStrength (Scalar) : 0 = 원본 텍스처 그대로, 1 = 아이템 색 적용
 * 기본 아이템(bIsDefault, 예: Hair_Yellow)은 TintStrength 0 → 원래 머리색 그대로
 */
UCLASS(ClassGroup = (PTB), meta = (BlueprintSpawnableComponent))
class PARTTIMEBEAT_API UPTBHairColorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBHairColorComponent();

	/** 현재 장착 헤어로 다시 적용 (메시 교체 후 호출) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hair")
	void ReapplyHairColor();

	/** 장착과 무관하게 특정 아이템 색 미리보기 (상점 미리보기용). ItemId=None이면 장착 헤어로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hair")
	void PreviewHairItem(FName ItemId);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 색을 바꿀 메시. 비우면 오너의 첫 번째 SkeletalMeshComponent (캐릭터면 GetMesh()) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hair")
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	/**
	 * 머리 머티리얼 슬롯 이름. 비우면 슬롯 이름이나 머티리얼 이름에 HairKeyword가 들어간 슬롯을 자동으로 찾음.
	 * PTB_Girl과 PTB_Boy의 슬롯 이름이 다르면 비워 두고 자동 찾기 사용
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hair")
	FName HairSlotName = NAME_None;

	/** 자동 찾기에 쓸 단어 (대소문자 무시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hair")
	FString HairKeyword = TEXT("Hair");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hair")
	FName ColorParamName = TEXT("HairColor");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hair")
	FName TintStrengthParamName = TEXT("HairTintStrength");

private:
	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	USkeletalMeshComponent* ResolveMesh() const;
	int32 FindHairSlotIndex(USkeletalMeshComponent* Mesh) const;
	UMaterialInstanceDynamic* GetOrCreateHairMID();
	void ApplyItem(FName ItemId);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HairMID;

	/** MID를 만든 메시/슬롯 (메시가 바뀌면 다시 생성) */
	TWeakObjectPtr<USkeletalMeshComponent> MIDOwnerMesh;
	TWeakObjectPtr<UObject> MIDSourceMeshAsset;
	int32 MIDSlotIndex = INDEX_NONE;

	/** 미리보기 중인 아이템 (None이면 장착 헤어) */
	FName PreviewItemId = NAME_None;
};
