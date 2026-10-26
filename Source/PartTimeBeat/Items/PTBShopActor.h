#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PTBHubInteractable.h"
#include "PTBShopActor.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UWidgetComponent;
class UPTBShopWidget;

/**
 * 허브에 배치하는 상점 (헤어샵, 판정 스킨샵, 탈것 가게 등).
 * 범위(InteractVolume) 안에서 Enter → 이 상점의 판매 카테고리만 보여주는 상점 팝업이 열림
 *
 * 사용법
 *  1) 이 클래스를 부모로 BP 생성 (예: BP_Shop_Hair) → 건물/NPC 메시를 자식으로 추가
 *  2) ShopName, SoldCategories 지정 (예: "헤어샵", [Hair])
 *  3) PromptWidget의 Widget Class에 "Enter: 상점" 안내 위젯 지정 (선택)
 *  4) 레벨에 배치하고 InteractVolume 크기 조절
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API APTBShopActor : public AActor, public IPTBHubInteractable
{
	GENERATED_BODY()

public:
	APTBShopActor();

	virtual bool OnHubInteract_Implementation(APlayerController* Player) override;

	/** 상점 이름 (팝업 제목) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	FText ShopName;

	/** 판매 카테고리 (예: Hair). 비어 있으면 전체 카테고리. 탭 순서 = 배열 순서 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	TArray<FName> SoldCategories;

	/** 이 상점만 다른 WBP를 쓰고 싶을 때 (비우면 PTB 아이템 설정의 ShopWidgetClass) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	TSubclassOf<UPTBShopWidget> ShopWidgetClassOverride;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 플레이어가 범위에 들어오거나 나갈 때 (BP: 하이라이트, 효과음 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Shop")
	void OnPlayerFocusChanged(bool bFocused);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	TObjectPtr<USceneComponent> Root;

	/** 상호작용 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	TObjectPtr<UBoxComponent> InteractVolume;

	/** "Enter: 상점" 안내 (범위 안에서만 표시) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Shop")
	TObjectPtr<UWidgetComponent> PromptWidget;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsLocalPlayerPawn(const AActor* Actor) const;
	void SetFocused(bool bFocused);

	bool bFocused = false;
};
