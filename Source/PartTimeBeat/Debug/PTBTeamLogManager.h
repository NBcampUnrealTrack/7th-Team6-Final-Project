#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTBTeamLogManager.generated.h"

/** 로그가 단순 기록인지, 문제 확인용 에러인지 구분 */
UENUM(BlueprintType)
enum class EPTBTeamLogPurpose : uint8
{
	/** 일반적인 흐름 확인, 상태 기록, 테스트 기록용 로그 */
	Record,

	/** 실패, 불일치, 비정상 상태처럼 확인이 필요한 로그 */
	Error
};

/** 한 번 찍힌 팀 로그를 세션 버퍼와 파일 출력에 남기기 위한 단일 로그 항목 */
USTRUCT(BlueprintType)
struct FPTBLogEntry
{
	GENERATED_BODY()

	/** 로그가 기록된 로컬 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Log")
	FDateTime Timestamp;

	/** 로그가 속한 영역 이름, 예: Flow, Rhythm, Audio, Multiplayer. */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Log")
	FName ChannelName;

	/** 로그의 목적, 단순 기록인지 에러 확인용인지 구분 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Log")
	EPTBTeamLogPurpose Purpose = EPTBTeamLogPurpose::Record;

	/** 세션, 플레이어, 미니게임, 박자 같은 부가 추적 정보 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Log")
	FPTBLogContext Context;
	
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Log")
	FString Message;
};

/**
 * 프로젝트 내부에서 사용하는 팀 전용 로그 매니저
 *
 * 일반 C++ 로그는 PTB_RECORD/PTB_ERROR 같은 매크로를 우선 사용하고,
 * 이 매니저는 세션 단위 버퍼링, Blueprint 호출, 파일 저장이 필요한 로그에 사용
 * AI 도움을 받아 작성됨
 */
UCLASS()
class PARTTIMEBEAT_API UPTBTeamLogManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** GameInstance 서브시스템 초기화 시 로그 세션 ID와 기본 활성 채널을 준비 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** GameInstance 서브시스템 종료 시 남아 있는 세션 로그를 파일로 flush */
	virtual void Deinitialize() override;

	/** 지정한 채널에 일반 기록용 로그를 남김 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogRecord(FName ChannelName, const FString& Message, const FPTBLogContext& Context);

	/** 지정한 채널에 에러 확인용 로그를 남김 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogError(FName ChannelName, const FString& Message, const FPTBLogContext& Context);

	/** 리듬 판정 결과를 정해진 형식으로 로그에 남김, Miss 판정은 에러 목적 로그로 기록하여 구분 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogRhythmJudge(const FPTBJudgementResult& Result, const FPTBLogContext& Context);

	/** Wwise 이벤트 호출 결과를 로그에 남김, 실패 코드로 판단되면 에러 목적 로그로 기록하여 구분 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogWWiseEvent(FName EventName, int32 AkResultCode, const FPTBLogContext& Context);

	/** 게임 흐름 상태 전환을 로그에 남김 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogFlowTransition(EGameFlowState From, EGameFlowState To, const FPTBLogContext& Context);

	/** 멀티플레이 결과 검증에서 로컬 결과와 서버 결과가 다를 때 에러 목적 로그로 남김 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void LogMultiResultMismatch(const FString& PlayerId, const FPTBRoundResult& Local, const FPTBRoundResult& Server, const FPTBLogContext& Context);

	/** 현재 메모리 버퍼에 쌓인 세션 로그를 Saved/Logs 폴더의 파일로 저장 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void FlushSessionLog();

	/** 특정 로그 채널의 기록 여부를 켜거나 끔 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void SetChannelEnabled(FName ChannelName, bool bEnabled);

	/** 세션 로그 파일 저장 여부를 설정, Output Log 출력 여부에는 영향을 주지 않음 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Log")
	void SetWriteToFile(bool bEnabled);

	/** 특정 팀 로그 채널이 현재 활성화되어 있는지 반환 */
	UFUNCTION(BlueprintPure, Category = "PTB|Log")
	bool IsChannelEnabled(FName ChannelName) const;

	/** 현재 세션 동안 메모리에 쌓인 로그 버퍼를 반환 */
	UFUNCTION(BlueprintPure, Category = "PTB|Log")
	TArray<FPTBLogEntry> GetLogBuffer() const { return LogBuffer; }

private:
	/** 로그 항목을 버퍼에 추가하고, 대응되는 UE Output Log 카테고리로도 출력 */
	void AddEntry(FName ChannelName, EPTBTeamLogPurpose Purpose, const FString& Message, const FPTBLogContext& Context);

	/** 채널 이름을 프로젝트 로그 카테고리에 매핑해서 UE Output Log에 출력 */
	void EmitToOutputLog(FName ChannelName, EPTBTeamLogPurpose Purpose, const FString& Message) const;

	/** FPTBLogContext의 유효한 필드만 모아 로그 메시지 뒤에 붙일 문자열을 만듦 */
	static FString BuildContextSuffix(const FPTBLogContext& Context);

	/** EGameFlowState 값을 사람이 읽기 쉬운 enum 이름 문자열로 변환 */
	static FString GetFlowStateName(EGameFlowState State);

	/** EPTBJudgementType 값을 사람이 읽기 쉬운 enum 이름 문자열로 변환 */
	static FString GetJudgementName(EPTBJudgementType JudgementType);

	/** 현재 GameInstance 생명주기 동안 유지되는 로그 세션 ID */
	UPROPERTY()
	FGuid SessionId;

	/** 현재 기록을 허용하는 로그 채널 이름 목록 */
	UPROPERTY()
	TSet<FName> EnabledChannels;

	/** 현재 세션 동안 메모리에 쌓아 둔 로그 항목 목록 */
	UPROPERTY()
	TArray<FPTBLogEntry> LogBuffer;

	/** FlushSessionLog 호출 시 파일로 저장할지 여부 */
	UPROPERTY()
	bool bWriteToFile = false;
};
