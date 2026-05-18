#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PTBRhythmCharacterBase.generated.h"

class UAnimInstance;
class UAnimMontage;
class UAkComponent;
class UAkAudioEvent;

/**
 * 모든 미니게임 캐릭터의 공통 베이스 클래스
 * 캐릭터 식별자 관리
 * 애니메이션 재생
 * Wwise SFX 연동
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API APTBRhythmCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	APTBRhythmCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|Character")
	virtual void PlaySuccessAnim();

	UFUNCTION(BlueprintCallable, Category = "PTB|Character")
	virtual void PlayFailAnim();

	UFUNCTION(BlueprintCallable, Category = "PTB|Character")
	virtual void PlayIdleAnim();

	UFUNCTION(BlueprintCallable, Category = "PTB|Character")
	virtual void PlayCustomAnim(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, Category = "PTB|Character")
	virtual void PostCharacterSFX(FName EventKey);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Character")
	FName CharacterId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Character")
	TObjectPtr<UAnimInstance> AnimInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TObjectPtr<UAkComponent> AkComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TObjectPtr<UAkAudioEvent> SuccessAkEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TObjectPtr<UAkAudioEvent> FailAkEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Animation")
	TObjectPtr<UAnimMontage> SuccessMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Animation")
	TObjectPtr<UAnimMontage> FailMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Animation")
	TObjectPtr<UAnimMontage> IdleMontage = nullptr;
};