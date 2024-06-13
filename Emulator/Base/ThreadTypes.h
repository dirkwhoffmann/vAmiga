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
    STATE_OFF,
    STATE_PAUSED,
    STATE_RUNNING,
    STATE_SUSPENDED,
    STATE_HALTED
};
typedef EXEC_STATE ExecState;

#ifdef __cplusplus
struct ExecStateEnum : util::Reflection<ExecStateEnum, ExecState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = STATE_HALTED;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "STATE"; }
    static const char *key(ExecState value)
    {
        switch (value) {

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
