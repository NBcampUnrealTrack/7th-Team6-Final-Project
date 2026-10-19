#include "UI/PTBSceneWidgetBase.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/DataTable.h"
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
	}
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[PTBScene] FinishScene: OwnerMiniGame 없음. 위젯만 제거합니다."));
		RemoveFromParent();
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
