#include "UI/PTBGameSelectPopupWidget.h"

#include "Components/TextBlock.h"
#include "Core/PTBStructEnums.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Progression/PTBProgressLibrary.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "UObject/UnrealType.h"

void UPTBGameSelectPopupWidget::NativeConstruct()
{
	Super::NativeConstruct(); // BP Construct가 먼저 실행된 뒤 C++이 점수를 채움
	LastShownMiniGameId = NAME_None;
	RefreshBestScoreTexts();
}

void UPTBGameSelectPopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 팝업을 연 뒤에 미니게임 ID가 들어오는 경우(또는 같은 팝업을 다른 NPC가 재사용) 대비
	const FName CurrentId = ResolveMiniGameId();
	if (CurrentId != LastShownMiniGameId)
	{
		RefreshBestScoreTexts();
	}
}

void UPTBGameSelectPopupWidget::SetBestScoreMiniGameId(FName InMiniGameId)
{
	ExplicitMiniGameId = InMiniGameId;
	RefreshBestScoreTexts();
}

void UPTBGameSelectPopupWidget::RefreshBestScoreTexts()
{
	const FName Id = ResolveMiniGameId();
	LastShownMiniGameId = Id;

	if (Id.IsNone())
	{
		if (!bWarnedNoMiniGameId)
		{
			bWarnedNoMiniGameId = true;
			UE_LOG(LogTemp, Warning, TEXT("[GameSelectPopup] %s: 미니게임 ID를 못 찾아서 최고 점수를 표시하지 않음. Construct에서 Set Best Score Mini Game Id를 호출해 주세요."), *GetClass()->GetName());
		}
		SetBestScoreText(BestScore_Easy, Id, EPTBDifficulty::Easy);
		SetBestScoreText(BestScore_Standard, Id, EPTBDifficulty::Standard);
		SetBestScoreText(BestScore_Insane, Id, EPTBDifficulty::Insane);
		return;
	}

	SetBestScoreText(BestScore_Easy, Id, EPTBDifficulty::Easy);
	SetBestScoreText(BestScore_Standard, Id, EPTBDifficulty::Standard);
	SetBestScoreText(BestScore_Insane, Id, EPTBDifficulty::Insane);
}

void UPTBGameSelectPopupWidget::SetBestScoreText(UTextBlock* TextBlock, FName InMiniGameId, EPTBDifficulty Difficulty) const
{
	if (!TextBlock)
	{
		return;
	}

	const int32 Score = InMiniGameId.IsNone() ? 0 : UPTBProgressLibrary::GetBestScore(this, InMiniGameId, Difficulty);
	if (Score <= 0)
	{
		TextBlock->SetText(NoRecordText);
		return;
	}

	FNumberFormattingOptions Grouped;
	Grouped.SetUseGrouping(true);

	FFormatNamedArguments Args;
	Args.Add(TEXT("Score"), FText::AsNumber(Score, &Grouped));
	Args.Add(TEXT("Percent"), UPTBScoreCalculator::ScoreToPercentText(Score));
	TextBlock->SetText(FText::Format(BestScoreFormat, Args));
}

FName UPTBGameSelectPopupWidget::ResolveMiniGameId() const
{
	return !ExplicitMiniGameId.IsNone() ? ExplicitMiniGameId : FindMiniGameIdFromBlueprintVariables();
}

FName UPTBGameSelectPopupWidget::FindMiniGameIdFromBlueprintVariables() const
{
	// BP에서 만든 변수만 검사 (C++ 클래스 쪽 변수는 제외)
	for (TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const FProperty* Property = *It;
		if (!Property->GetOwnerClass() || !Cast<UBlueprintGeneratedClass>(Property->GetOwnerClass()))
		{
			continue;
		}

		// 1) 이름에 MiniGame이 들어간 Name 변수 (예: MiniGameId, TargetMiniGameId)
		if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
		{
			if (Property->GetName().Contains(TEXT("MiniGame")))
			{
				const FName Value = NameProperty->GetPropertyValue_InContainer(this);
				if (!Value.IsNone())
				{
					return Value;
				}
			}
			continue;
		}

		// 2) MiniGameId 필드를 가진 구조체 변수 (예: FPTBGameSessionRequest, FPTBMiniGameDefinition 등)
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			const FNameProperty* InnerId = CastField<FNameProperty>(StructProperty->Struct->FindPropertyByName(TEXT("MiniGameId")));
			if (InnerId)
			{
				const void* StructData = StructProperty->ContainerPtrToValuePtr<void>(this);
				const FName Value = InnerId->GetPropertyValue_InContainer(StructData);
				if (!Value.IsNone())
				{
					return Value;
				}
			}
		}
	}

	return NAME_None;
}
