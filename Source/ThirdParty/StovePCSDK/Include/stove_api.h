/*
 * file: stove_api.h
 * Umbrella header for the StovePCSDK Public/C surface.
 * Including this single header exposes the complete public C API of every
 * module at once, so integrators do not have to include each module header
 * individually. For each module it pulls in:
 *   - <module>_misc.h     : enums, method codes, result codes, STOVE_API
 *   - <module>_types.h    : interface (IStove*) and struct type definitions
 *   - <module>_api.h       : action functions (Stove_*)
 *   - <module>_flat_api.h  : flat-C accessors for the IStove* interfaces
 *
 * Modules: Base, GameSupport, GamingServices, IAP, Log, Ownership, PCBang, View.
 * Base is included first because every other module's types depend on it
 * (base_misc.h defines STOVE_API; base_types.h is included by all <module>_types.h).
 *
 * Start here: the core SDK lifecycle entry points
 * (Stove_Initialize / Stove_Uninitialize / Stove_RunCallback /
 *  Stove_RestartAppIfNecessary / Stove_GetVersion) are declared in base_api.h,
 * which is included first below. This umbrella adds no declarations of its own;
 * it only aggregates the per-module headers.
 */

#pragma once

/* Base - foundation: SDK lifecycle entry points + core API (defines STOVE_API; required by all other modules) */
#include "base_misc.h"
#include "base_types.h"
#include "base_api.h"
#include "base_flat_api.h"

/* GameSupport */
#include "game_support_misc.h"
#include "game_support_types.h"
#include "game_support_api.h"
#include "game_support_flat_api.h"

/* IAP */
#include "iap_misc.h"
#include "iap_types.h"
#include "iap_api.h"
#include "iap_flat_api.h"

/* Log */
#include "log_misc.h"
#include "log_types.h"
#include "log_api.h"
#include "log_flat_api.h"

/* Ownership */
#include "ownership_misc.h"
#include "ownership_types.h"
#include "ownership_api.h"
#include "ownership_flat_api.h"

/* PCBang */
#include "pcbang_misc.h"
#include "pcbang_types.h"
#include "pcbang_api.h"
#include "pcbang_flat_api.h"

/* View */
#include "view_misc.h"
#include "view_types.h"
#include "view_api.h"
#include "view_flat_api.h"
