#include "MiniGames/PC/PTBPCMiniGame.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "MiniGames/PC/PTBPCMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"

APTBPCMiniGame::APTBPCMiniGame()
{
    static ConstructorHelpers::FClassFinder<APCTileImageActor> ImageActorClass(
        TEXT("/Game/PTB/MiniGames/PC/Blueprints/BP_PCTileImageActor"));
    if (ImageActorClass.Succeeded())
        TileImageClass = ImageActorClass.Class;
}

void APTBPCMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();
    ActiveTiles.Empty();
    PCSpawnedActors.Reset();
    PracticePointIndexByNoteId.Reset();
    TileTextures.Empty();

    FString TilePrefix;
    FString TextureFolderPath;

    int32 TileCount = ChartAsset ? ChartAsset->NoteEvents.Num() : 0;
    EPTBDifficulty CurrentDifficulty = GameContext.SessionRequest.Difficulty;

    switch (CurrentDifficulty)
    {
    case EPTBDifficulty::Easy:
        TilePrefix = TEXT("EasyTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_EasyTiles");
        break;
    case EPTBDifficulty::Insane:
        TilePrefix = TEXT("InsaneTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_InsaneTiles");
        break;
    case EPTBDifficulty::Standard:
        TilePrefix = TEXT("StandardTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_StandardTiles");
        break;
    }

    for (int32 i = 1; i <= TileCount; i++)
    {
        FString AssetName = FString::Printf(TEXT("%s_%d_"), *TilePrefix, i);
        FString Path = FString::Printf(TEXT("%s/%s.%s"), *TextureFolderPath, *AssetName, *AssetName);

        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Path);
        TileTextures.Add(Texture);

        if (!Texture)
        {
            UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *Path);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("로드된 텍스처 수: %d, 총 노트 수: %d"), TileTextures.Num(), TileCount);
   
    RailCharacter = Cast<APCRailCharacter>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCRailCharacter::StaticClass()));

    TileSpawner = Cast<APCTileSpawner>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCTileSpawner::StaticClass()));

    if (RailCharacter)
    {
        RailCharacter->SelectRailByDifficulty(CurrentDifficulty);
    }
}


void APTBPCMiniGame::HandleActionAInput()
{
    HandleRhythmInput(EPTBActionType::ActionA);
}

void APTBPCMiniGame::HandleActionAInputReleased()
{
    HandleRhythmInputReleased(EPTBActionType::ActionA);
}

void APTBPCMiniGame::PreloadAudioAssets()
{
    Super::PreloadAudioAssets();
}

void APTBPCMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    if (!TileSpawner) return;

    APCTileActor* Tile = TileSpawner->SpawnTile();
    if (Tile)
    {
        ActiveTiles.Add(Note.NoteId, Tile);
        RegisterSpawnedRoundActor(Tile);
        PCSpawnedActors.Add(Tile);
    }
}

void APTBPCMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);

}

void APTBPCMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);
    if (!RailCharacter) return;

    FTimerHandle MoveDelayHandle;
    GetWorldTimerManager().SetTimer(
        MoveDelayHandle,
        RailCharacter,
        &APCRailCharacter::MoveOneStep,
        0.02f,
        false
    );
}

void APTBPCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);

    if (JudgementPopupWidget && !UsesCommonJudgementDisplay())   // 공통 판정 표시를 쓰면 전용 팝업은 끔
    {
        JudgementPopupWidget->ShowJudgement(Result.JudgementType);
    }

    if (RailCharacter)
    {
        RailCharacter->PlayHandMontage(Result.JudgementType == EPTBJudgementType::Miss
            ? RailCharacter->HandFailMontage
            : RailCharacter->HandSuccessMontage);
    }

    if (Result.JudgementType == EPTBJudgementType::Miss) return;

    if (!RailCharacter || !RailCharacter->PCRailPath) return;

    const int32 PointIndex = ResolvePointIndex(Result.NoteId);

    if (!TileTextures.IsValidIndex(PointIndex - 1)) return;

    FVector SpawnLocation = RailCharacter->PCRailPath->GetPointLocation(PointIndex);
    SpawnLocation.Z += -75.f;

    APCTileImageActor* ImageActor = GetWorld()->SpawnActor<APCTileImageActor>(
        TileImageClass, SpawnLocation, FRotator(0.f, 0.f, 0.f));

    if (ImageActor)
    {
        ImageActor->SetTexture(TileTextures[PointIndex - 1]);
        RegisterSpawnedRoundActor(ImageActor);
        PCSpawnedActors.Add(ImageActor);
    }

}

int32 APTBPCMiniGame::ResolvePointIndex(int32 NoteId) const
{
    if (const int32* Found = PracticePointIndexByNoteId.Find(NoteId))
    {
        return *Found;
    }
    return NoteId;
}

void APTBPCMiniGame::ClearPCSpawnedActors()
{
    for (AActor* Actor : PCSpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    PCSpawnedActors.Reset();
    ActiveTiles.Empty();
    SpawnedRoundActors.RemoveAll([](const TObjectPtr<AActor>& Actor) { return !IsValid(Actor); });
}

void APTBPCMiniGame::HandleRoundChartChanged()
{
    Super::HandleRoundChartChanged();

    // 연습에서 완성한 타일/이동한 거리가 본게임에 이어지지 않도록 처음 상태로
    ClearPCSpawnedActors();
    if (RailCharacter)
    {
        // 노트마다 0.02초 뒤 한 칸 이동 예약 → 남은 예약이 초기화 후에 실행되지 않도록 취소
        GetWorldTimerManager().ClearAllTimersForObject(RailCharacter);
    }

    // 연습 채보는 NoteId가 100000번대라 칸 번호로 못 씀 → 노트 순서대로 1, 2, 3... 칸 사용
    PracticePointIndexByNoteId.Reset();
    const bool bPracticeChart = TutorialPracticeChart && ChartAsset.Get() == TutorialPracticeChart.Get();
    if (bPracticeChart && ChartAsset)
    {
        for (int32 Index = 0; Index < ChartAsset->NoteEvents.Num(); ++Index)
        {
            PracticePointIndexByNoteId.Add(ChartAsset->NoteEvents[Index].NoteId, Index + 1);
        }
    }

    if (RailCharacter)
    {
        RailCharacter->SelectRailByDifficulty(GameContext.SessionRequest.Difficulty);
    }

    UE_LOG(LogTemp, Log, TEXT("[PC] Round chart changed. Practice=%d Notes=%d"),
        bPracticeChart ? 1 : 0, ChartAsset ? ChartAsset->NoteEvents.Num() : 0);
}

