// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(EXEC_STATE)
{
    EXEC_OFF,
    EXEC_PAUSED,
    EXEC_RUNNING,
    EXEC_SUSPENDED,
    EXEC_HALTED
};
typedef EXEC_STATE ExecutionState;

#ifdef __cplusplus
struct ExecutionStateEnum : util::Reflection<ExecutionStateEnum, ExecutionState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = EXEC_HALTED;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "EXEC"; }
    static const char *key(ExecutionState value)
    {
        switch (value) {

            case EXEC_OFF:          return "OFF";
            case EXEC_PAUSED:       return "PAUSED";
            case EXEC_RUNNING:      return "RUNNING";
            case EXEC_SUSPENDED:    return "SUSPENDED";
            case EXEC_HALTED:       return "HALTED";
        }
        return "???";
    }
};
#endif

enum_long(SYNC_MODE)
{
    SYNC_PERIODIC,
    SYNC_PULSED,
};
typedef SYNC_MODE SyncMode;

#ifdef __cplusplus
struct SyncModeEnum : util::Reflection<SyncModeEnum, SyncMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SYNC_PULSED;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "SYNC"; }
    static const char *key(SyncMode value)
    {
        switch (value) {

            case SYNC_PERIODIC:   return "PERIODIC";
            case SYNC_PULSED:     return "PULSED";
        }
        return "???";
    }
};

#endif
