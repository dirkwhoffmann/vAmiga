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

namespace vamiga {

//
// Enumerations
//

enum class RecState : long
{
    WAIT = 0,
    PREPARE = 1,
    RECORD = 2,
    FINALIZE = 3,
    ABORT = 4
};
// typedef REC_STATE RecorderState;

/*
struct RecStateEnum : util::Reflection<RecStateEnum, RecState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = RecState::ABORT;
    
    static const char *prefix() { return "REC"; }
    static const char *_key(RecState value)
    {
        switch (value) {
                
            case RecState::WAIT:      return "WAIT";
            case RecState::PREPARE:   return "PREPARE";
            case RecState::RECORD:    return "RECORD";
            case RecState::FINALIZE:  return "FINALIZE";
            case RecState::ABORT:     return "ABORT";
        }
        return "???";
    }
    static const char *help(long value)
    {
        return "";
    }
};
*/

//
// Structures
//

typedef struct
{
    // Video paramters
    isize frameRate;
    isize bitRate;
    isize sampleRate;
}
RecorderConfig;

/*
typedef struct
{
    RecState state;
}
RecorderInfo;
*/

}
