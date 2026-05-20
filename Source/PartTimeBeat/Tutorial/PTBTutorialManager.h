// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Core/PTBStructEnums.h"
#include "PTBTutorialManager.generated.h"

USTRUCT(BlueprintType)
struct FPTBTutorialStepRow : public FTableRowBase
{
	GENERATED_BODY()
	// 미니게임 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName GameId; 
	
	//UI안내 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText InstructionText; 

	//시범영상(선택사항)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FSoftObjectPath DemoVideoPath; 

	// 엑션 선택 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	EPTBActionType RequiredInput; 

	//이 스텝 스킵가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bAllowSkip = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepChanged, int32, StepIndex, FText, InstructionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialCompleted, FName, GameId);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBTutorialManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTBTutorialManager();
	
	/** 진입여부 필요?*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool ShouldShowTutorial(FName GameId, const FString& ProfileId);
	
	/**튜토리얼 시작*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorial(FName GameId,const FString& ProfileId);
	
	/**진행중인 스텝*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvanceStep();
	
	/**튜토리얼 스킵*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SkipTutorial();
	
	// 델리게이트용함수 두개
	
	/**튜토리얼 스텝이 넘어갈때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTutorialStepChanged OnTutorialStepChanged;
    
	/**튜토리얼 완료시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTutorialCompleted OnTutorialCompleted;

protected:
	/**튜토리얼 완료*/
	UFUNCTION(BlueprintCallable,Category="Tutorial")
	void CompleteTutorial(FName GameId,const FString& ProfileId);

private:
	/**스킵가능 여부*/
	UPROPERTY() 
	bool bCanSkip;
	
	/** 튜토리얼 진행 중 */
	UPROPERTY()
	bool bIsTutorialActive;
	
	/**현재 튜토리얼 스텝*/
	UPROPERTY()
	int32 CurrentStepIndex;
	
	/**게임별 튜토리얼 스텝*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial",meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDataTable> TutorialDataTable;
	
	/**튜토리얼 스텝을 넣을 배열*/
	UPROPERTY()
	TArray<FPTBTutorialStepRow> CurrentSteps;
	
	/** 프로필 아이디*/
	UPROPERTY()
	FString CurrentProfileId;
};