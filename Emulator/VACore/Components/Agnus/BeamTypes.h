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

enum class FrameType
{
    PAL_LF,       // PAL long frame
    PAL_SF,       // PAL short frame
    NTSC_LF_LL,   // NTSC long frame starting with a long line
    NTSC_LF_SL,   // NTSC long frame starting with a short line
    NTSC_SF_LL,   // NTSC short frame starting with a long line
    NTSC_SF_SL    // NTSC short frame starting with a short line
};

struct FrameTypeEnum : Reflection<FrameTypeEnum, FrameType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FrameType::NTSC_SF_SL);
    
    static const char *_key(FrameType value)
    {
        switch (value) {
                
            case FrameType::PAL_LF:      return "PAL_LF";
            case FrameType::PAL_SF:      return "PAL_SF";
            case FrameType::NTSC_LF_LL:  return "NTSC_LF_LL";
            case FrameType::NTSC_LF_SL:  return "NTSC_LF_SL";
            case FrameType::NTSC_SF_LL:  return "NTSC_SF_LL";
            case FrameType::NTSC_SF_SL:  return "NTSC_SF_SL";
        }
        return "???";
    }
    static const char *help(FrameType value)
    {
        switch (value) {
                
            case FrameType::PAL_LF:      return "PAL long frame";
            case FrameType::PAL_SF:      return "PAL short frame";
            case FrameType::NTSC_LF_LL:  return "NTSC long frame, long line";
            case FrameType::NTSC_LF_SL:  return "NTSC long frame, short line";
            case FrameType::NTSC_SF_LL:  return "NTSC short frame, long line";
            case FrameType::NTSC_SF_SL:  return "NTSC short frame, short line";
        }
        return "???";
    }
};

}
