#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/PTBStructEnums.h"
#include "PTBProfileSubsystem.generated.h"

class UPTBSaveGame;


/** 활성 프로필이 변경되었을 때. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveProfileChanged, const FPTBProfileData& /*NewProfile*/);

/** 프로필 목록(생성/삭제)이 변경되었을 때. */
DECLARE_MULTICAST_DELEGATE(FOnProfileListChanged);

UCLASS()
class PARTTIMEBEAT_API UPTBProfileSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** 신규 프로필 생성 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    FGuid CreateProfile(const FString& Nickname, EPTBGender Gender, int32 SlotIndex, FDateTime Birthday);

    /** ID로 프로필 조회 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    FPTBProfileData GetProfile(const FGuid& ProfileId, bool& bOutFound) const;

    /** 모든 프로필 목록 반환 (UI 슬롯 표시용) */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    TArray<FPTBProfileData> GetAllProfiles() const;

    /** 현재 저장된 프로필 개수 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    int32 GetProfileCount() const;

    /** 슬롯 인덱스로 프로필 조회 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    FPTBProfileData GetProfileBySlot(int32 SlotIndex, bool& bOutFound) const;

    /** UI 슬롯 칸 개수 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    int32 GetMaxProfileSlotCount() const { return MaxProfiles; }

    /** 슬롯 인덱스로 프로필 삭제 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool DeleteProfileBySlot(int32 SlotIndex);

    /** 활성화된 프로필 슬롯 설정 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Active")
    bool SetActiveProfile(const FGuid& ProfileId);

    /** 현재 활성화된 프로필 데이터 반환 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Active")
    FPTBProfileData GetActiveProfile(bool& bOutHasActive) const;

    /** 활성화 한 프로필이 있는지 여부. */
    UFUNCTION(BlueprintCallable, Category = "Profile|Active")
    bool HasActiveProfile() const;


    UFUNCTION(BlueprintCallable, Category = "Profile|Validation")
    EPTBNicknameValidationResult ValidateNickname(const FString& Nickname) const;

    /** 해당 미니게임을 처음 플레이하는지 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Validation")
    bool IsFirstTimeProfile(const FGuid& ProfileId, FName MiniGameId) const;

    /** 라운드 결과 프로필에 반영 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    void ApplyRoundResultToActive(const FPTBProfileProgressUpdate& Update);

    /** 튜토리얼 완료 마크 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    void MarkTutorialComplete(FName MiniGameId);

    /** 스토리 시청 완료 마크 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    void MarkStoryViewed(FName StoryId);

    /** 코스튬 해금 */
    //UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    //void UnlockCostume(FName CostumeId);

    /** 미니게임 최고 점수 (난이도 무관 전체 최고) */
    UFUNCTION(BlueprintCallable, Category = "Profile|Query")
    int32 GetBestScore(FName MiniGameId) const;

    /** 특정 난이도의 미니게임 최고 점수. 기록 없으면 0 반환. */
    UFUNCTION(BlueprintCallable, Category = "Profile|Query")
    int32 GetBestScoreForDifficulty(FName MiniGameId, EPTBDifficulty Difficulty) const;

    /** 미니게임 획득 별 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Query")
    int32 GetEarnedStars(FName MiniGameId) const;

    /** 활성화 된 프로필의 누적 알바비 */
    UFUNCTION(BlueprintCallable, Category = "Profile|Query")
    int32 GetTotalEarnedMoney() const;


    /** SaveGame에 동기화하여 디스크에 저장 */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    void RequestSave();

    FOnActiveProfileChanged OnActiveProfileChanged;
    FOnProfileListChanged   OnProfileListChanged;

protected:
    // 내부 헬퍼

    /** GUID 기반 삭제 — C++ 내부 전용. Blueprint에서는 DeleteProfileBySlot을 사용할 것. */
    bool DeleteProfile(const FGuid& ProfileId);

    // SaveGame에서 프로필 목록을 메모리로 로드
    void LoadProfilesFromSave();

    // 활성화 된 프로필 복원
    void ClearActiveProfile();

    // SaveGame 인스턴스 불러오기 또는 만들기
    UPTBSaveGame* GetOrCreateSaveGame() const;

    // 닉네임 정리 (앞뒤 공백 제거 등)
    FString NormalizeNickname(const FString& InNickname) const;

    // 활성화 된 프로필 데이터에 대한 수정 가능 포인터 없으면 nullptr
    FPTBProfileData* FindActiveProfileMutable();

    // ID로 프로필 수정 가능 포인터 검색
    FPTBProfileData* FindProfileMutable(const FGuid& ProfileId);

    // ── 금칙어 필터 ──────────────────────────────────────────────

    /**
     * Content/Data/ForbiddenWords.txt 에서 금칙어 목록을 로드
     * Initialize() 에서 1회 호출
     */
    void LoadForbiddenWords();

    /**
     * 닉네임에 금칙어가 포함되어 있는지 검사
     * 영숫자만 남긴 소문자 문자열로 정규화 후 부분 문자열 매칭
     */
    bool ContainsForbiddenWord(const FString& Nickname) const;

    /**
     * 혼합 정규화: 한글 음절 → 자모 분해, 영숫자 → 소문자 유지, 특수문자 제거
     * 영어 금칙어 및 "시×발" 같은 특수문자 삽입 우회를 차단
     */
    static FString NormalizeForFilter(const FString& Input);

    /**
     * 한글 전용 정규화: 한글 자모만 남기고 영숫자 포함 나머지 모두 제거
     * "ㅅxㅂ"처럼 한글 사이에 영문자를 끼워 넣는 우회를 차단
     */
    static FString NormalizeForFilterKoreanOnly(const FString& Input);

    /**
     * 난이도별 점수 키 생성 유틸리티.
     * 반환 형식: "MiniGameId_DifficultyName" (예: "TG_Standard")
     */
    UFUNCTION(BlueprintPure, Category = "Profile|Query")
    static FName MakeDifficultyScoreKey(FName MiniGameId, EPTBDifficulty Difficulty);

private:
    /** 메모리 상의 모든 프로필 */
    UPROPERTY()
    TArray<FPTBProfileData> AllProfiles;

    /** 정규화된 금칙어 목록 (LoadForbiddenWords에서 채워짐) */
    TArray<FString> ForbiddenWords;

    /** 활성화 된 프로필 ID */
    UPROPERTY()
    FGuid ActiveProfileId;

    /** 최대 슬롯 수 */
    UPROPERTY()
    int32 MaxProfiles = 3;

    static constexpr int32 NicknameMinLength = 1;
    static constexpr int32 NicknameMaxLength = 12;
};
