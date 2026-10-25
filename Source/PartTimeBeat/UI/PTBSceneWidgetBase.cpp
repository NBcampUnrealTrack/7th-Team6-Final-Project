#include "UI/PTBSceneWidgetBase.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/DataTable.h"
#include "Components/TextBlock.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "UI/PTBKeyTextLibrary.h"

namespace PTBSceneWidgetInternal
{
	template <typename RowType>
	bool CollectRows(const UDataTable* Table, TArray<RowType>& OutRows)
	{
		OutRows.Reset();
		if (!Table || Table->GetRowStruct() != RowType::StaticStruct())
		{
			return false;
		}

		TArray<RowType*> RowPtrs;
		Table->GetAllRows<RowType>(TEXT("PTBSceneWidget"), RowPtrs);
		for (const RowType* Row : RowPtrs)
		{
			if (Row)
			{
				OutRows.Add(*Row);
			}
		}
		return true;
	}
}

void UPTBSceneWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Enter/Space 등 키 입력을 받기 위해 포커스 가능하게
	SetIsFocusable(true);
}

void UPTBSceneWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsTypewriterRunning())
	{
		return;
	}

	TypewriterElapsed += InDeltaTime;
	const int32 TargetChars = FMath::Min(
		TypewriterFullText.Len(),
		FMath::FloorToInt(TypewriterElapsed * FMath::Max(1.0f, TypewriterCharsPerSecond)));

	if (TargetChars != TypewriterShownChars)
	{
		TypewriterShownChars = TargetChars;
		if (UTextBlock* Target = TypewriterTarget.Get())
		{
			Target->SetText(FText::FromString(TypewriterFullText.Left(TypewriterShownChars)));
		}

		if (TypewriterShownChars >= TypewriterFullText.Len())
		{
			OnTypewriterFinished();
		}
	}
}

void UPTBSceneWidgetBase::StartTypewriter(UTextBlock* Target, const FText& FullText)
{
	TypewriterTarget = Target;
	TypewriterFullText = FullText.ToString();
	TypewriterElapsed = 0.0f;
	TypewriterShownChars = 0;

	if (!Target)
	{
		return;
	}

	if (TypewriterCharsPerSecond <= 0.0f || TypewriterFullText.IsEmpty())
	{
		TypewriterShownChars = TypewriterFullText.Len();
		Target->SetText(FullText);
		OnTypewriterFinished();
		return;
	}

	Target->SetText(FText::GetEmpty());
}

bool UPTBSceneWidgetBase::IsTypewriterRunning() const
{
	return TypewriterTarget.IsValid() && TypewriterShownChars < TypewriterFullText.Len();
}

void UPTBSceneWidgetBase::CompleteTypewriter()
{
	if (!IsTypewriterRunning())
	{
		return;
	}

	TypewriterShownChars = TypewriterFullText.Len();
	if (UTextBlock* Target = TypewriterTarget.Get())
	{
		Target->SetText(FText::FromString(TypewriterFullText));
	}
	OnTypewriterFinished();
}

void UPTBSceneWidgetBase::SetupScene(APTBBaseMiniGame* InOwnerMiniGame, EPTBMiniGameScenePhase InPhase, UDataTable* InTable)
{
	OwnerMiniGame = InOwnerMiniGame;
	ScenePhase = InPhase;
	SceneTable = InTable;
	bSceneFinished = false;

	NativeOnSceneSetup();
	if (!bSceneFinished)
	{
		OnSceneSetup();
	}
}

void UPTBSceneWidgetBase::HandleSceneAction(EPTBActionType Action)
{
	if (bSceneFinished)
	{
		return;
	}

	NativeOnSceneAction(Action);
	if (!bSceneFinished)
	{
		OnSceneAction(Action);
	}
}

void UPTBSceneWidgetBase::FinishScene(bool bSkipped)
{
	if (bSceneFinished)
	{
		return;
	}
	bSceneFinished = true;

	if (APTBBaseMiniGame* MiniGame = OwnerMiniGame.Get())
	{
		MiniGame->NotifySceneFinished(bSkipped);
		OnSceneFinished.Broadcast(this, bSkipped);
	}
	else
	{
		// 미니게임 밖(허브 오프닝 등): 위젯을 띄운 쪽이 OnSceneFinished로 다음 단계 진행
		if (!OnSceneFinished.IsBound())
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[PTBScene] FinishScene: OwnerMiniGame/구독자 없음. 위젯만 제거합니다."));
		}
		RemoveFromParent();
		OnSceneFinished.Broadcast(this, bSkipped);
	}
}

bool UPTBSceneWidgetBase::HandleConfirmInput()
{
	if (bSceneFinished)
	{
		return false;
	}

	return NativeOnConfirm();
}

void UPTBSceneWidgetBase::HandlePracticeFinished(bool bPassed, int32 HitCount, int32 RequiredCount, int32 TotalCount)
{
	if (bSceneFinished)
	{
		return;
	}

	NativeOnPracticeFinished(bPassed, HitCount, RequiredCount, TotalCount);
	if (!bSceneFinished)
	{
		OnPracticeFinished(bPassed, HitCount, RequiredCount, TotalCount);
	}
}

bool UPTBSceneWidgetBase::GetDialogueLines(TArray<FPTBDialogueLine>& OutLines) const
{
	return PTBSceneWidgetInternal::CollectRows(SceneTable.Get(), OutLines);
}

bool UPTBSceneWidgetBase::GetTutorialSteps(TArray<FPTBTutorialStep>& OutSteps) const
{
	return PTBSceneWidgetInternal::CollectRows(SceneTable.Get(), OutSteps);
}

FText UPTBSceneWidgetBase::FormatSceneText(const FText& SourceText) const
{
	return UPTBKeyTextLibrary::FormatKeyTokens(this, SourceText);
}
