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

void UPTBSceneWidgetBase::SetupScene(APTBBaseMiniGame* InOwnerMiniGame, EPTBMiniGameScenePhase InPhase, UDataTable* InTable)
{
	OwnerMiniGame = InOwnerMiniGame;
	ScenePhase = InPhase;
	SceneTable = InTable;
	bSceneFinished = false;

	OnSceneSetup();
}

void UPTBSceneWidgetBase::HandleSceneAction(EPTBActionType Action)
{
	if (bSceneFinished)
	{
		return;
	}

	OnSceneAction(Action);
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
