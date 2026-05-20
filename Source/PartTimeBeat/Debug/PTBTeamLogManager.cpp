#include "PTBTeamLogManager.h"

#include "Debug/PTBTeamLog.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	const FName ChannelCore(TEXT("Core"));
	const FName ChannelFlow(TEXT("Flow"));
	const FName ChannelProfile(TEXT("Profile"));
	const FName ChannelRhythm(TEXT("Rhythm"));
	const FName ChannelAudio(TEXT("Audio"));
	const FName ChannelCommon(TEXT("Common"));
	const FName ChannelMiniGames(TEXT("MiniGames"));
	const FName ChannelTutorial(TEXT("Tutorial"));
	const FName ChannelProgression(TEXT("Progression"));
	const FName ChannelCharacters(TEXT("Characters"));
	const FName ChannelMultiplayer(TEXT("Multiplayer"));
	const FName ChannelUI(TEXT("UI"));
	const FName ChannelDebug(TEXT("Debug"));
	const FName ChannelInterfaces(TEXT("Interfaces"));
	const FName ChannelWwise(TEXT("Wwise"));
}

void UPTBTeamLogManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SessionId = FGuid::NewGuid();
	bWriteToFile = !UE_BUILD_SHIPPING;

	EnabledChannels = {
		ChannelCore,
		ChannelFlow,
		ChannelProfile,
		ChannelRhythm,
		ChannelAudio,
		ChannelCommon,
		ChannelMiniGames,
		ChannelTutorial,
		ChannelProgression,
		ChannelCharacters,
		ChannelMultiplayer,
		ChannelUI,
		ChannelDebug,
		ChannelInterfaces,
		ChannelWwise
	};

	PTB_RECORD(LogPTBDebug, TEXT("[PTBLog] 팀 로그 세션 시작. SessionId=%s"), *SessionId.ToString());
}

void UPTBTeamLogManager::Deinitialize()
{
	FlushSessionLog();
	PTB_RECORD(LogPTBDebug, TEXT("[PTBLog] 팀 로그 세션 종료. SessionId=%s"), *SessionId.ToString());

	Super::Deinitialize();
}

void UPTBTeamLogManager::LogRecord(FName ChannelName, const FString& Message, const FPTBLogContext& Context)
{
	AddEntry(ChannelName, EPTBTeamLogPurpose::Record, Message, Context);
}

void UPTBTeamLogManager::LogError(FName ChannelName, const FString& Message, const FPTBLogContext& Context)
{
	AddEntry(ChannelName, EPTBTeamLogPurpose::Error, Message, Context);
}

void UPTBTeamLogManager::LogRhythmJudge(const FPTBJudgementResult& Result, const FPTBLogContext& Context)
{
	const FString Message = FString::Printf(
		TEXT("리듬 판정 NoteId=%d Action=%d 판정=%s 오차Ms=%.2f 점수변화=%d 콤보끊김=%d"),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		*GetJudgementName(Result.JudgementType),
		Result.DeltaMs,
		Result.ScoreDelta,
		Result.bBreaksCombo);

	AddEntry(ChannelRhythm, Result.JudgementType == EPTBJudgementType::Miss ? EPTBTeamLogPurpose::Error : EPTBTeamLogPurpose::Record, Message, Context);
}

void UPTBTeamLogManager::LogWWiseEvent(FName EventName, int32 AkResultCode, const FPTBLogContext& Context)
{
	const EPTBTeamLogPurpose Purpose = (AkResultCode == 0 || AkResultCode == 1) ? EPTBTeamLogPurpose::Record : EPTBTeamLogPurpose::Error;
	const FString Message = FString::Printf(
		TEXT("Wwise 이벤트=%s 결과코드=%d"),
		*EventName.ToString(),
		AkResultCode);

	AddEntry(ChannelWwise, Purpose, Message, Context);
}

void UPTBTeamLogManager::LogFlowTransition(EGameFlowState From, EGameFlowState To, const FPTBLogContext& Context)
{
	const FString Message = FString::Printf(
		TEXT("화면 흐름 전환 %s -> %s"),
		*GetFlowStateName(From),
		*GetFlowStateName(To));

	AddEntry(ChannelFlow, EPTBTeamLogPurpose::Record, Message, Context);
}

void UPTBTeamLogManager::LogMultiResultMismatch(const FString& PlayerId, const FPTBRoundResult& Local,
	const FPTBRoundResult& Server, const FPTBLogContext& Context)
{
	const FString Message = FString::Printf(
		TEXT("멀티 결과 불일치 Player=%s 로컬점수=%d 서버점수=%d 로컬콤보=%d 서버콤보=%d 로컬정확도=%.2f 서버정확도=%.2f"),
		*PlayerId,
		Local.Score,
		Server.Score,
		Local.MaxCombo,
		Server.MaxCombo,
		Local.AccuracyRate,
		Server.AccuracyRate);

	AddEntry(ChannelMultiplayer, EPTBTeamLogPurpose::Error, Message, Context);
}

void UPTBTeamLogManager::FlushSessionLog()
{
	if (!bWriteToFile || LogBuffer.Num() == 0)
	{
		return;
	}

	TArray<FString> Lines;
	Lines.Reserve(LogBuffer.Num());

	for (const FPTBLogEntry& Entry : LogBuffer)
	{
		const TCHAR* PurposeText = Entry.Purpose == EPTBTeamLogPurpose::Error ? TEXT("Error") : TEXT("Record");
		Lines.Add(FString::Printf(
			TEXT("[%s][%s][%s] %s%s"),
			*Entry.Timestamp.ToString(TEXT("%Y-%m-%d %H:%M:%S.%s")),
			*Entry.ChannelName.ToString(),
			PurposeText,
			*Entry.Message,
			*BuildContextSuffix(Entry.Context)));
	}

	const FString LogDir = FPaths::ProjectSavedDir() / TEXT("Logs");
	IFileManager::Get().MakeDirectory(*LogDir, true);

	const FString FileName = FString::Printf(TEXT("PTBTeamLog_%s.log"), *SessionId.ToString(EGuidFormats::Digits));
	const FString FilePath = LogDir / FileName;

	FFileHelper::SaveStringArrayToFile(Lines, *FilePath);
}

void UPTBTeamLogManager::SetChannelEnabled(FName ChannelName, bool bEnabled)
{
	if (bEnabled)
	{
		EnabledChannels.Add(ChannelName);
		return;
	}

	EnabledChannels.Remove(ChannelName);
}

void UPTBTeamLogManager::SetWriteToFile(bool bEnabled)
{
	bWriteToFile = bEnabled;
}

bool UPTBTeamLogManager::IsChannelEnabled(FName ChannelName) const
{
	return EnabledChannels.Contains(ChannelName);
}

void UPTBTeamLogManager::AddEntry(FName ChannelName, EPTBTeamLogPurpose Purpose, const FString& Message, const FPTBLogContext& Context)
{
	if (!PTB_ENABLE_TEAM_LOG || !IsChannelEnabled(ChannelName))
	{
		return;
	}

	FPTBLogEntry Entry;
	Entry.Timestamp = FDateTime::Now();
	Entry.ChannelName = ChannelName;
	Entry.Purpose = Purpose;
	Entry.Context = Context;
	Entry.Message = Message;
	LogBuffer.Add(Entry);

	EmitToOutputLog(ChannelName, Purpose, Message + BuildContextSuffix(Context));
}

void UPTBTeamLogManager::EmitToOutputLog(FName ChannelName, EPTBTeamLogPurpose Purpose, const FString& Message) const
{
#define PTB_EMIT(Category) \
	do \
	{ \
		if (Purpose == EPTBTeamLogPurpose::Error) \
		{ \
			PTB_ERROR(Category, TEXT("%s"), *Message); \
		} \
		else \
		{ \
			PTB_RECORD(Category, TEXT("%s"), *Message); \
		} \
	} while (false)

	if (ChannelName == ChannelCore) { PTB_EMIT(LogPTBCore); }
	else if (ChannelName == ChannelFlow) { PTB_EMIT(LogPTBFlow); }
	else if (ChannelName == ChannelProfile) { PTB_EMIT(LogPTBProfile); }
	else if (ChannelName == ChannelRhythm) { PTB_EMIT(LogPTBRhythm); }
	else if (ChannelName == ChannelAudio) { PTB_EMIT(LogPTBAudio); }
	else if (ChannelName == ChannelCommon) { PTB_EMIT(LogPTBCommon); }
	else if (ChannelName == ChannelMiniGames) { PTB_EMIT(LogPTBMiniGames); }
	else if (ChannelName == ChannelTutorial) { PTB_EMIT(LogPTBTutorial); }
	else if (ChannelName == ChannelProgression) { PTB_EMIT(LogPTBProgression); }
	else if (ChannelName == ChannelCharacters) { PTB_EMIT(LogPTBCharacters); }
	else if (ChannelName == ChannelMultiplayer) { PTB_EMIT(LogPTBMultiplayer); }
	else if (ChannelName == ChannelUI) { PTB_EMIT(LogPTBUI); }
	else if (ChannelName == ChannelInterfaces) { PTB_EMIT(LogPTBInterfaces); }
	else if (ChannelName == ChannelWwise) { PTB_EMIT(LogPTBWwise); }
	else { PTB_EMIT(LogPTBDebug); }

#undef PTB_EMIT
}

FString UPTBTeamLogManager::BuildContextSuffix(const FPTBLogContext& Context)
{
	TArray<FString> Parts;

	if (Context.SessionId.IsValid())
	{
		Parts.Add(FString::Printf(TEXT("세션=%s"), *Context.SessionId.ToString()));
	}
	if (!Context.PlayerId.IsEmpty())
	{
		Parts.Add(FString::Printf(TEXT("플레이어=%s"), *Context.PlayerId));
	}
	if (!Context.MiniGameId.IsNone())
	{
		Parts.Add(FString::Printf(TEXT("미니게임=%s"), *Context.MiniGameId.ToString()));
	}
	if (!FMath::IsNearlyZero(Context.BeatTime))
	{
		Parts.Add(FString::Printf(TEXT("박자=%.3f"), Context.BeatTime));
	}

	return Parts.Num() > 0 ? FString::Printf(TEXT(" {%s}"), *FString::Join(Parts, TEXT(", "))) : FString();
}

FString UPTBTeamLogManager::GetFlowStateName(EGameFlowState State)
{
	const UEnum* Enum = StaticEnum<EGameFlowState>();
	return Enum ? Enum->GetNameStringByValue(static_cast<int64>(State)) : FString::FromInt(static_cast<int32>(State));
}

FString UPTBTeamLogManager::GetJudgementName(EPTBJudgementType JudgementType)
{
	const UEnum* Enum = StaticEnum<EPTBJudgementType>();
	return Enum ? Enum->GetNameStringByValue(static_cast<int64>(JudgementType)) : FString::FromInt(static_cast<int32>(JudgementType));
}
