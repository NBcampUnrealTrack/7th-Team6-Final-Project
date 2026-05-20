#pragma once

#include "Debug/PTBLogChannels.h"

#ifndef PTB_ENABLE_TEAM_LOG
#define PTB_ENABLE_TEAM_LOG (!UE_BUILD_SHIPPING)
#endif

#if PTB_ENABLE_TEAM_LOG
#define PTB_RECORD(Category, Format, ...) \
	do \
	{ \
		UE_LOG(Category, Log, Format, ##__VA_ARGS__); \
	} while (false)

#define PTB_ERROR(Category, Format, ...) \
	do \
	{ \
		UE_LOG(Category, Error, Format, ##__VA_ARGS__); \
	} while (false)

#define PTB_WARNING(Category, Format, ...) \
	do \
	{ \
		UE_LOG(Category, Warning, Format, ##__VA_ARGS__); \
	} while (false)

#define PTB_VERBOSE(Category, Format, ...) \
	do \
	{ \
		UE_LOG(Category, Verbose, Format, ##__VA_ARGS__); \
	} while (false)
#else
#define PTB_RECORD(Category, Format, ...) do { } while (false)
#define PTB_ERROR(Category, Format, ...) do { } while (false)
#define PTB_WARNING(Category, Format, ...) do { } while (false)
#define PTB_VERBOSE(Category, Format, ...) do { } while (false)
#endif
