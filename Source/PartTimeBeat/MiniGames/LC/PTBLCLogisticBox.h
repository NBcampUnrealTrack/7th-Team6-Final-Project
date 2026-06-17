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
	APTBLCLogisticBox();
	
	virtual void Tick(float DeltaTime) override;
	
	int32 GetNoteId() const;
	
	float GetNoteTimeMs() const;
	
	EPTBLCColorState GetContentColorState() const;
	
	EPTBLCColorState GetBoxColorState() const;
	
	UStaticMeshComponent* GetStaticMeshComponent() const;
	
	UMaterialInstance* GetRedMaterialInstance() const;
	
	UMaterialInstance* GetBlueMaterialInstance() const;
	
	UMaterialInstance* GetYellowMaterialInstance() const;

	void InitializeFromNote(const FPTBNoteEvent& Note);
	
	void PackageWithActionType(EPTBActionType ActionType);
	
	bool GetIsPackaged() const;
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Mesh")
	void StopMovingAndEnablePhysics();
	
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Movement")
	void SetMovementRotation(FRotator NewRotation);
	
	void ChangeMeshToBox();
	
	void ApplyPackagedMaterial();
	
	void StartPackagingSpin();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> BoxMeshAsset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> TriangleMeshAsset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> RedBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> BlueBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> YellowBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	TObjectPtr<UMaterialInterface> PackagedBoxMaterial;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor RedColor = FLinearColor::Red;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor YellowColor = FLinearColor::Yellow;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FLinearColor BlueColor = FLinearColor::Blue;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FName BoxColorParameterName = TEXT("BoxColor");
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Material")
	FName MarkColorParameterName = TEXT("MarkColor");
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	int32 NoteId = 0;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	float NoteTimeMs = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	EPTBLCColorState ContentColorState = EPTBLCColorState::None;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PTB|LC|State")
	EPTBLCColorState BoxColorState = EPTBLCColorState::None;
private:
	
	UPROPERTY(EditAnywhere, Category = "PTB|LC|Param")
	float MovingSpeed = 300.f;
	
	bool bIsMoving = true;
	
	bool bIsPackaged = false;
	
	bool bIsPackagingSpinActive = false;
	
	float PackagingSpinElapsedSeconds = 0.0f;
	
	FRotator PackagingSpinStartRotation = FRotator::ZeroRotator;
	
	FVector MovementDirection = FVector::RightVector;
};
