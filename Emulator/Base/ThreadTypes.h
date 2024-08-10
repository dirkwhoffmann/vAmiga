// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

//
// Enumerations
//

/// Execution state
enum_long(EXEC_STATE)
{
    STATE_UNINIT,       ///< Not yet initialized
    STATE_OFF,          ///< Powered off
    STATE_PAUSED,       ///< Powered on, but currently paused
    STATE_RUNNING,      ///< Up and running
    STATE_SUSPENDED,    ///< Shortly paused for an internal state change
    STATE_HALTED        ///< Shut down
};
typedef EXEC_STATE ExecState;

#ifdef __cplusplus
struct ExecStateEnum : vamiga::util::Reflection<ExecStateEnum, ExecState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = STATE_HALTED;

    static const char *prefix() { return "STATE"; }
    static const char *_key(long value)
    {
        switch (value) {

            case STATE_UNINIT:       return "UNINIT";
            case STATE_OFF:          return "OFF";
            case STATE_PAUSED:       return "PAUSED";
            case STATE_RUNNING:      return "RUNNING";
            case STATE_SUSPENDED:    return "SUSPENDED";
            case STATE_HALTED:       return "HALTED";
        }
        return "???";
    }
};
#endif
