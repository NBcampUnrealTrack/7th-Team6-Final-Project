#include "PTBSRMiniGame.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "SRWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "PTBSRPlate.h"

APTBSRMiniGame::APTBSRMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
	ToppingDataTable = nullptr;

	PlateMoveSpeed = -100.0f; // 원래 BP_SR_Plate에서 쓰던 눈 판정용 속도로 복원
	SpawnerZHeight = 50.0f;
}

void APTBSRMiniGame::BeginPlay()
{
	Super::BeginPlay();
}

TMap<FKey, EPTBActionType> APTBSRMiniGame::GetActionMapping() const
{
	return {
		{ GameContext.UserSettings.RhythmKeys.ActionA, EPTBActionType::ActionA }
	};
}

void APTBSRMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	Super::InitializeMiniGame(Context);
	CurrentNoteIndex = 0;
}

FPTBRoundResult APTBSRMiniGame::FinishMiniGame(EPTBRoundEndReason Reason)
{
	//return Super::FinishMiniGame(Reason);

	FPTBRoundResult FinalResult = Super::FinishMiniGame(Reason);

	FinalResult.MiniGameId = FName("SR");
	FinalResult.Difficulty = GameContext.SessionRequest.Difficulty;
	FinalResult.PlayMode = GameContext.SessionRequest.PlayMode;

	int32 TotalSuccessSushi = SuccessSushiCount;

	// 초밥 1개당 1,000점 / 초밥 1개당 1,900원 계산 / 26개
	FinalResult.Score = TotalSuccessSushi * 1000;

	FPTBRewardSummary RewardSummary;
	RewardSummary.EarnedMoney = TotalSuccessSushi * 1900;

	if (FinalResult.Score >= 26000)       FinalResult.Grade = EPTBGradeType::S;
	else if (FinalResult.Score >= 20000)  FinalResult.Grade = EPTBGradeType::A;
	else if (FinalResult.Score >= 13000)  FinalResult.Grade = EPTBGradeType::B;
	else if (FinalResult.Score >= 5000)   FinalResult.Grade = EPTBGradeType::C;
	else                                  FinalResult.Grade = EPTBGradeType::Fail;

	RoundResult = FinalResult;

	if (OnMiniGameFinished.IsBound())
	{
		OnMiniGameFinished.Broadcast(FinalResult);
	}

	return FinalResult;
}

void APTBSRMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	PTB_WARNING(LogPTBMiniGames, TEXT("빌드 런타임 실행됨."));

	if (!ChartAsset || ChartAsset->NoteEvents.IsEmpty()) return;
	if (!ToppingDataTable) return;

	ToppingQueue.Reset();
	TArray<FName> RowNames = ToppingDataTable->GetRowNames();

	if (RowNames.IsEmpty()) return;

	int32 TotalNotes = ChartAsset->NoteEvents.Num();

	for (int32 i = 0; i < TotalNotes; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
		FName SelectedRowName = RowNames[RandomIndex];

		int32 ToppingTypeInt = FCString::Atoi(*SelectedRowName.ToString());

		if (ToppingTypeInt == 0 && !SelectedRowName.ToString().Equals(TEXT("0")))
		{
			ToppingTypeInt = 1;
		}

		ToppingQueue.Add(ToppingTypeInt);
	}

	UE_LOG(LogRhythm, Log, TEXT("[%s] 데이터 테이블 기반 총 %d개의 초밥 채보 토핑 무작위 셔플 완료!"), *GetNameSafe(this), ToppingQueue.Num());
}

void APTBSRMiniGame::RegisterActivePlate(int32 NoteId, AActor* PlateActor)
{
	if (PlateActor)
	{
		ActivePlatesMap.Add(NoteId, PlateActor);
	}
}

void APTBSRMiniGame::RegisterActiveTopping(int32 NoteId, AActor* ToppingActor)
{
	if (ToppingActor)
	{
		ActiveToppingsMap.Add(NoteId, ToppingActor);
	}
}

bool APTBSRMiniGame::NotifyToppingReachedConveyor(int32 NoteId)
{
	if (!PendingSuccessNoteIds.Contains(NoteId))
	{
		// 이 NoteId는 성공 대기 중이 아니었습니다 (Miss였거나 이미 처리됨) — 호출한 쪽에서 정리하면 됩니다.
		return false;
	}

	PendingSuccessNoteIds.Remove(NoteId);
	ResolveSuccessVisual(NoteId); // 지금(=닿는 순간) 완성 처리
	return true;
}

void APTBSRMiniGame::ResolveSuccessVisualIfStillPending(int32 NoteId)
{
	// 이미 NotifyToppingReachedConveyor로 처리됐다면 여기서는 아무것도 안 합니다.
	if (PendingSuccessNoteIds.Contains(NoteId))
	{
		PendingSuccessNoteIds.Remove(NoteId);
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] NoteId=%d 가 최대 대기 시간 동안 접시에 닿지 않아 강제로 완성 처리합니다."),
			*GetNameSafe(this), NoteId);
		ResolveSuccessVisual(NoteId);
	}
}

void APTBSRMiniGame::ResolveSuccessVisual(int32 NoteId)
{
	AActor* Topping = nullptr;
	if (AActor** Found = ActiveToppingsMap.Find(NoteId))
	{
		Topping = *Found;
	}

	if (!IsValid(Topping))
	{
		// 아직 스폰 전이거나(이론상 없어야 함), 이미 다른 경로로 정리된 경우
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] ResolveSuccessVisual: NoteId=%d 에 등록된 토핑을 찾지 못했습니다."),
			*GetNameSafe(this), NoteId);
		return;
	}

	// 토핑이 들고 있는 완성 스시 클래스를 리플렉션으로 읽음 (BP_SR_Topping::MySushiClass)
	TSubclassOf<AActor> SushiClass;
	if (const FClassProperty* ClassProp = FindFProperty<FClassProperty>(Topping->GetClass(), ToppingSushiClassPropertyName))
	{
		SushiClass = TSubclassOf<AActor>(Cast<UClass>(ClassProp->GetPropertyValue_InContainer(Topping)));
	}

	// ★ 완성 위치 결정 우선순위:
	//   1) 이 노트를 담당하며 화면에 다가오고 있던 "바로 그 접시"의 현재 위치 (가장 정확함)
	//   2) 레벨에 배치해둔 고정 위치(CompletionSpawnPoint) — 접시 등록이 안 됐을 때의 대비책
	//   3) 토핑의 현재 위치 — 최후의 수단, 판정이 물리적 낙하보다 먼저 끝나므로 부정확할 수 있음
	AActor* Plate = nullptr;
	if (AActor** FoundPlate = ActivePlatesMap.Find(NoteId))
	{
		Plate = *FoundPlate;
	}

	FVector CompletionLocation;
	FRotator CompletionRotation;
	if (IsValid(Plate))
	{
		CompletionLocation = Plate->GetActorLocation();
		CompletionRotation = Plate->GetActorRotation();
	}
	else if (CompletionSpawnPoint)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] ResolveSuccessVisual: NoteId=%d 에 등록된 접시를 찾지 못해 CompletionSpawnPoint를 대신 사용합니다."),
			*GetNameSafe(this), NoteId);
		CompletionLocation = CompletionSpawnPoint->GetActorLocation();
		CompletionRotation = CompletionSpawnPoint->GetActorRotation();
	}
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] ResolveSuccessVisual: NoteId=%d 에 등록된 접시도, CompletionSpawnPoint도 없어 토핑의 현재 위치를 대신 사용합니다 (부정확할 수 있음)."),
			*GetNameSafe(this), NoteId);
		CompletionLocation = Topping->GetActorLocation();
		CompletionRotation = Topping->GetActorRotation();
	}

	if (CompletionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, CompletionVFX, CompletionLocation, CompletionRotation,
			FVector(1.0f), /*bAutoDestroy=*/true, /*bAutoActivate=*/true,
			ENCPoolMethod::None, /*bPreCullCheck=*/true);
	}

	if (SushiClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AActor>(SushiClass, FTransform(CompletionRotation, CompletionLocation), SpawnParams);
		}
	}
	else
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[%s] ResolveSuccessVisual: NoteId=%d 토핑의 SushiClass가 유효하지 않습니다."),
			*GetNameSafe(this), NoteId);
	}

	// 판정이 끝난 토핑은 접시에 닿았는지와 무관하게 여기서 제거 (완성됨 = 사라짐)
	Topping->Destroy();
	ActiveToppingsMap.Remove(NoteId);

	// 담당하던 접시도 역할을 다했으니 제거 (완성 스시가 그 자리를 대신함)
	if (IsValid(Plate))
	{
		Plate->Destroy();
	}
	ActivePlatesMap.Remove(NoteId);
}

int32 APTBSRMiniGame::GetToppingTypeFromQueue(int32 NoteId) const
{
	if (ToppingQueue.IsValidIndex(NoteId))
	{
		return ToppingQueue[NoteId];
	}
	return 1;
}

FPTBToppingRow APTBSRMiniGame::GetToppingData(int32 ToppingType) const
{
	FPTBToppingRow DefaultRow;
	DefaultRow.SR_ToppingName = TEXT("기본 계란");
	DefaultRow.SR_ToppingMesh = nullptr;
	DefaultRow.SR_GravityScale = 1.0f;
	DefaultRow.SR_ToppingMaterial = nullptr;
	DefaultRow.SR_ToppingIcon = nullptr;

	if (!ToppingDataTable) return DefaultRow;

	FName RowName = *FString::FromInt(ToppingType);

	// 🚨 incorrect type 에러 차단을 위해 TEXT("") 적용
	FPTBToppingRow* FoundRow = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));

	if (FoundRow)
	{
		return *FoundRow;
	}

	return DefaultRow;
}

void APTBSRMiniGame::RefreshPreviewUI()
{
	if (!WBP_SR_Preview)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] RefreshPreviewUI: WBP_SR_Preview 위젯 변수가 nullptr입니다!"), *GetNameSafe(this));
		return;
	}
	/*
	if (!ToppingDataTable) return;

	int32 NextToppingType = ToppingQueue.IsValidIndex(CurrentNoteIndex) ? ToppingQueue[CurrentNoteIndex] : 1;
	FName RowName = *FString::FromInt(NextToppingType);

	// 🚨 [수정] 이 부분도 TEXT("")로 변경하여 무조건 안전하게 에러를 방어합니다!
	FPTBToppingRow* ToppingData = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));

	if (ToppingData)
	{
		if (!ToppingData->SR_ToppingIcon) return;

		UImage* TargetImage = Cast<UImage>(WBP_SR_Preview->GetWidgetFromName(TEXT("ImgNextTopping")));
		if (!TargetImage)
		{
			TargetImage = Cast<UImage>(WBP_SR_Preview->GetWidgetFromName(TEXT("Img_NextTopping")));
		}

		if (TargetImage)
		{
			TargetImage->SetBrushFromTexture(ToppingData->SR_ToppingIcon);
		}
	}*/
	USRWidget* MySushiUI = Cast<USRWidget>(WBP_SR_Preview);
	if (!MySushiUI || !ToppingDataTable) return;

	int32 NextToppingType = ToppingQueue.IsValidIndex(CurrentNoteIndex) ? ToppingQueue[CurrentNoteIndex] : 1;
	FName RowName = *FString::FromInt(NextToppingType);

	FPTBToppingRow* ToppingData = ToppingDataTable->FindRow<FPTBToppingRow>(RowName, TEXT(""));
	if (ToppingData && ToppingData->SR_ToppingIcon)
	{
		MySushiUI->UpdatePreviewImage(ToppingData->SR_ToppingIcon);
	}
}

void APTBSRMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
	// ★ 디자인 요구사항: 박자가 맞든 틀리든 "누를 때마다 토핑은 무조건 스폰"되어야 합니다.
	//   맞았을 때만 완성되고, 틀리면 그냥 떨어질 뿐입니다. 그래서 판정 결과와 무관하게
	//   여기서 먼저 토핑을 스폰하고, 임시 키(음수)로 등록해둡니다.
	int32 SpawnedTempKey = INDEX_NONE;
	if (Action == EPTBActionType::ActionA && !ToppingQueue.IsEmpty())
	{
		const int32 VisualIndex = ToppingQueue.IsValidIndex(CurrentNoteIndex) ? CurrentNoteIndex : 0;
		const int32 AssignedTopping = ToppingQueue[VisualIndex];

		SpawnedTempKey = NextTempToppingKey--;
		OnToppingDrop.Broadcast(AssignedTopping, SpawnedTempKey);
	}

	// Super:: 안에서 판정이 동기적으로 진행되며 HandleJudgementResult가 호출됩니다.
	// 그때 이 임시 키로 방금 스폰한 토핑을 찾아 실제 매칭 여부를 반영할 수 있도록 기억해둡니다.
	PendingInputToppingKey = SpawnedTempKey;
	Super::HandleRhythmInput(Action, TimeMs);
	PendingInputToppingKey = INDEX_NONE;
}



void APTBSRMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	if (Note.ActionType == EPTBActionType::ActionA)
	{
		int32 AssignedTopping = ToppingQueue.IsValidIndex(Note.NoteId) ? ToppingQueue[Note.NoteId] : 1;
		OnSushiPlateSpawn.Broadcast(AssignedTopping, Note.NoteId);
	}
}

void APTBSRMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
	NoteJudgementResults.Add(Result.NoteId, Result.JudgementType); // 결과 저장

	// ★ 토핑은 이미 HandleRhythmInput에서 (판정 결과와 무관하게) 스폰되어 임시 키로
	//   등록되어 있습니다. 이번 판정이 진짜 노트에 대한 것이었다면(Reason=Note), 그 토핑을
	//   임시 키에서 진짜 Result.NoteId로 다시 등록해서, 완성 로직이 정확히 이 토핑을 찾을 수 있게 합니다.
	//   헛입력(EmptyInput)이었다면 아무것도 안 하고 넘어갑니다 — 그 토핑은 임시 키를 단 채로
	//   그냥 계속 떨어지다가 접시에 닿으면(PTBSRPlate) 완성되지 않고 정리됩니다.
	if (Result.Reason == EPTBJudgementReason::Note && PendingInputToppingKey != INDEX_NONE)
	{
		if (AActor** FoundTopping = ActiveToppingsMap.Find(PendingInputToppingKey))
		{
			AActor* Topping = *FoundTopping;
			ActiveToppingsMap.Remove(PendingInputToppingKey);

			if (IsValid(Topping))
			{
				// 나중에 PTBSRPlate가 Overlap에서 이 토핑의 NoteId를 리플렉션으로 읽으므로,
				// 실제 액터의 프로퍼티 값도 진짜 NoteId로 갱신해줘야 합니다.
				if (FIntProperty* NoteIdProp = FindFProperty<FIntProperty>(Topping->GetClass(), TEXT("NoteId")))
				{
					NoteIdProp->SetPropertyValue_InContainer(Topping, Result.NoteId);
				}
				ActiveToppingsMap.Add(Result.NoteId, Topping);
			}
		}
	}

	// ★ 판정이 Miss가 아니면, 점수는 지금 이 순간 바로 확정합니다. 완성 비주얼(VFX/모델 스폰)은
	//   토핑이 실제로 접시(컨베이어)에 물리적으로 닿는 순간에 트리거됩니다
	//   (PTBSRPlate::HandleBoxBeginOverlap → NotifyToppingReachedConveyor). 그래야 "완전히
	//   떨어지고 나서 완성되는" 자연스러운 낙하 연출이 됩니다. 혹시 끝까지 아무 접시에도
	//   안 닿는 예외 상황을 대비해 MaxSuccessVisualWaitSeconds 뒤에는 강제로라도 완성 처리합니다.
	if (Result.JudgementType != EPTBJudgementType::Miss)
	{
		++SuccessSushiCount;
		OnSushiSuccessDelegate.Broadcast(Result.NoteId, Result.JudgementType);

		PendingSuccessNoteIds.Add(Result.NoteId);

		if (MaxSuccessVisualWaitSeconds > 0.0f)
		{
			FTimerDelegate FallbackResolve;
			FallbackResolve.BindUObject(this, &APTBSRMiniGame::ResolveSuccessVisualIfStillPending, Result.NoteId);

			FTimerHandle TempHandle;
			GetWorldTimerManager().SetTimer(TempHandle, FallbackResolve, MaxSuccessVisualWaitSeconds, false);
		}
	}
	else
	{
		OnSushiMissDelegate.Broadcast(Result.NoteId);
	}

	// ★ 성공 카운트 UI도 판정 시점에 바로 갱신 (Overlap 여부와 무관)
	if (USRWidget* MySushiUI = Cast<USRWidget>(WBP_SR_Preview))
	{
		MySushiUI->UpdateSuccessCount(SuccessSushiCount);
	}

	CurrentNoteIndex++;
	RefreshPreviewUI();
}