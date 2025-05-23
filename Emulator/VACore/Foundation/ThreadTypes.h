// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

/// Execution state
enum class ExecState : long
{
    UNINIT,       ///< Not yet initialized
    OFF,          ///< Powered off
    PAUSED,       ///< Powered on, but currently paused
    RUNNING,      ///< Up and running
    HALTED        ///< Shut down
};

struct ExecStateEnum : Reflection<ExecStateEnum, ExecState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ExecState::HALTED);
    
    static const char *_key(ExecState value)
    {
        switch (value) {
                
            case ExecState::UNINIT:       return "UNINIT";
            case ExecState::OFF:          return "OFF";
            case ExecState::PAUSED:       return "PAUSED";
            case ExecState::RUNNING:      return "RUNNING";
            case ExecState::HALTED:       return "HALTED";
        }
        return "???";
    }
    
    static const char *help(ExecState value)
    {
        return "";
    }
};

}
