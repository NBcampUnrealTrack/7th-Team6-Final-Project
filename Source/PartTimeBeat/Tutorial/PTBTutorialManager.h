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
	
	//순서
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 StepIndex =0; 

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
	UFUNCTION()
	bool ShouldShowTutorial(FName GameId, const FString& ProfileId) const;
	
	UFUNCTION()
	void StartTutorial(FName GameId);
	
	UFUNCTION()
	void AdvanceStep();
	
	UFUNCTION()
	void SkipTutorial();
	
	UFUNCTION()
	void CompleteTutorial(FName GameId,const FString& ProfileId);
	
	// 델리게이트용함수 두개
	
	/**튜토리얼 스텝이 넘어갈때 호출되는 함수 */
	UPROPERTY()
	FOnTutorialStepChanged OnTutorialStepChanged;
    
	/**튜토리얼 완료시 호출되는 함수 */
	UPROPERTY()
	FOnTutorialCompleted OnTutorialCompleted;
	
protected:
	UPROPERTY()
	FName CurrentTutorialGameId;
	
	UPROPERTY() 
	bool bCanSkip;
	
	UPROPERTY()
	bool bIsTutorialActive;
	
	UPROPERTY()
	int32 TutorialClearCount;
	
	UPROPERTY()
	int32 CurrentStepIndex;
	
	UPROPERTY()
	TObjectPtr<UDataTable> TutorialDataTable;
};