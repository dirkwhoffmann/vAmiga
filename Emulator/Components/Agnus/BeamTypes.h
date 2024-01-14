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

enum_long(FRAME_TYPE)
{
    FRAME_PAL_LF,       // PAL long frame
    FRAME_PAL_SF,       // PAL short frame
    FRAME_NTSC_LF_LL,   // NTSC long frame starting with a long line
    FRAME_NTSC_LF_SL,   // NTSC long frame starting with a short line
    FRAME_NTSC_SF_LL,   // NTSC short frame starting with a long line
    FRAME_NTSC_SF_SL    // NTSC short frame starting with a short line
};
typedef FRAME_TYPE FrameType;

#ifdef __cplusplus
struct FrameTypeEnum : util::Reflection<FrameTypeEnum, FrameType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FRAME_NTSC_SF_SL;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FRAME"; }
    static const char *key(FrameType value)
    {
        switch (value) {

            case FRAME_PAL_LF:      return "PAL_LF";
            case FRAME_PAL_SF:      return "PAL_SF";
            case FRAME_NTSC_LF_LL:  return "NTSC_LF_LL";
            case FRAME_NTSC_LF_SL:  return "NTSC_LF_SL";
            case FRAME_NTSC_SF_LL:  return "NTSC_SF_LL";
            case FRAME_NTSC_SF_SL:  return "NTSC_SF_SL";
        }
        return "???";
    }
};
#endif
