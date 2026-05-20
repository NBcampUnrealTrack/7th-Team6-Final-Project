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

public:
    //키 바인딩 관련 
    // 키 입력 전달 — MiniGame 등에서 호출
    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleInput(FKey Key);

    // 키 바인딩은 C++ 전용 (TFunction은 UPROPERTY 불가)
    void BindAction(FKey Key, TFunction<void()> Fn);

    // ── 행동 함수들 ──────────────────────────────────────────
    // BP 서브클래스에서 오버라이드 가능

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void MoveLeft();
    virtual void MoveLeft_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void MoveRight();
    virtual void MoveRight_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void MoveUp();   
    virtual void MoveUp_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void MoveDown();
    virtual void MoveDown_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void Interact();
    virtual void Interact_Implementation();

    // 선택적 행동 — 필요한 캐릭터만 _Implementation 오버라이드
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void HitNoteA();
    virtual void HitNoteA_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void HitNoteW();
    virtual void HitNoteW_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void HitNoteS();
    virtual void HitNoteS_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void HitNoteD();
    virtual void HitNoteD_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
    void HitNoteEnter();
    virtual void HitNoteEnter_Implementation();

protected:
    // 에디터에서 편집 가능한 스탯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 100.f;

private:
    TMap<FKey, TFunction<void()>> Actions;
};