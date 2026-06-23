#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "PTBLCLogisticBox.generated.h"

/** LC 박스 색 상태 */
UENUM(BlueprintType)
enum class EPTBLCColorState : uint8
{
	Red = 0 UMETA(DisplayName = "Red"),
	Yellow = 1 UMETA(DisplayName = "Yellow"),
	Blue = 2 UMETA(DisplayName = "Blue"),
	None = 3 UMETA(DisplayName = "None"),
};

/**
 * LC 미니게임에서 사용되는 물류 박스 Actor입니다.
 * 박스는 언패키징된 상태(색상별 삼각형 메시)와 패키징된 상태(색상별 박스 메시)로 구분됩니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBLCLogisticBox : public AActor
{
	GENERATED_BODY()

public:
	/** 기본값 초기화 */
	APTBLCLogisticBox();

	/** 프레임별 이동 및 포장 회전 연출 처리 */
	virtual void Tick(float DeltaTime) override;

	/** 연결된 채보 노트 ID */
	int32 GetNoteId() const;

	/** 연결된 채보 노트 시간(ms) */
	float GetNoteTimeMs() const;

	/** 내용물 색 상태 */
	EPTBLCColorState GetContentColorState() const;

	/** 외부 박스 색 상태 */
	EPTBLCColorState GetBoxColorState() const;

	/** 비주얼 메시 컴포넌트 */
	UStaticMeshComponent* GetStaticMeshComponent() const;

	/** Red 내용물 머터리얼 */
	UMaterialInstance* GetRedMaterialInstance() const;

	/** Blue 내용물 머터리얼 */
	UMaterialInstance* GetBlueMaterialInstance() const;

	/** Yellow 내용물 머터리얼 */
	UMaterialInstance* GetYellowMaterialInstance() const;

	/** 채보 노트 기준으로 내용물 상태 초기화 */
	void InitializeFromNote(const FPTBNoteEvent& Note);

	/** 입력 액션 기준으로 박스 포장 처리 */
	void PackageWithActionType(EPTBActionType ActionType);

	/** 포장 완료 여부 */
	bool GetIsPackaged() const;

	/** 현재 박스 이동 속도 */
	float GetMovingSpeed() const;

	/** Beat 기준 둠칫 스케일 적용. 미포장 내용물 또는 올바른 박스에만 반영됩니다. */
	void SetBeatPulseScale(float NewScale);

	/** 이동 정지 후 물리 활성화 */
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Mesh")
	void StopMovingAndEnablePhysics();

	/** Actor 회전과 이동 방향을 함께 갱신 */
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Movement")
	void SetMovementRotation(FRotator NewRotation);

	/** 박스 메시로 전환 */
	void ChangeMeshToBox();

	/** 포장 박스 Dynamic Material 적용 */
	void ApplyPackagedMaterial();

	/** 포장 시 회전 연출 시작 */
	void StartPackagingSpin();

protected:
	/** 게임 시작 처리 */
	virtual void BeginPlay() override;

	/** 박스 비주얼 메시 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;

	/** 포장 후 박스 메시 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> BoxMeshAsset;

	/** 포장 전 내용물 메시 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> TriangleMeshAsset;

	/** Red 내용물 머터리얼 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> RedBoxMaterialInstance;

	/** Blue 내용물 머터리얼 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> BlueBoxMaterialInstance;

	/** Yellow 내용물 머터리얼 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> YellowBoxMaterialInstance;

	/** 포장 박스 단일 머터리얼 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	TObjectPtr<UMaterialInterface> PackagedBoxMaterial;

	/** Red 색상 파라미터 값 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor RedColor = FLinearColor::Red;

	/** Yellow 색상 파라미터 값 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor YellowColor = FLinearColor::Yellow;

	/** Blue 색상 파라미터 값 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor BlueColor = FLinearColor::Blue;

	/** 외부 박스 색 머터리얼 파라미터 이름 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FName BoxColorParameterName = TEXT("BoxColor");

	/** 내용물 표시 색 머터리얼 파라미터 이름 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FName MarkColorParameterName = TEXT("MarkColor");

	/** 연결된 채보 노트 ID */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	int32 NoteId = 0;

	/** 연결된 채보 노트 시간(ms) */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	float NoteTimeMs = 0.0f;

	/** 내용물 색 상태 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	EPTBLCColorState ContentColorState = EPTBLCColorState::None;

	/** 외부 박스 색 상태 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	EPTBLCColorState BoxColorState = EPTBLCColorState::None;

private:
	/** Beat 둠칫 효과 적용 가능 여부 */
	bool ShouldApplyBeatPulse() const;

	/** 이동 속도 */
	UPROPERTY(EditAnywhere, Category = "PTB|LC|Param")
	float MovingSpeed = 300.f;

	/** 이동 중 여부 */
	bool bIsMoving = true;

	/** 포장 완료 여부 */
	bool bIsPackaged = false;

	/** 포장 회전 연출 중 여부 */
	bool bIsPackagingSpinActive = false;

	/** 포장 회전 연출 경과 시간 */
	float PackagingSpinElapsedSeconds = 0.0f;

	/** 포장 회전 시작 회전값 */
	FRotator PackagingSpinStartRotation = FRotator::ZeroRotator;

	/** Actor 회전과 분리된 이동 방향 */
	FVector MovementDirection = FVector::RightVector;

	/** Beat 둠칫 효과 기준 메시 스케일 */
	FVector BaseMeshRelativeScale = FVector::OneVector;
};
