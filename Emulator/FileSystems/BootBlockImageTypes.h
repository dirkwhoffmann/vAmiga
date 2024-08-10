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

enum_long(BB_TYPE)
{
    BB_STANDARD,
    BB_VIRUS,
    BB_CUSTOM
};
typedef BB_TYPE BootBlockType;

#ifdef __cplusplus
struct BootBlockTypeEnum : vamiga::util::Reflection<BootBlockTypeEnum, BootBlockType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = BB_CUSTOM;

    static const char *prefix() { return "BB"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case BB_STANDARD:  return "STANDARD";
            case BB_VIRUS:     return "VIRUS";
            case BB_CUSTOM:    return "CUSTOM";
        }
        return "???";
    }
};
#endif

enum_long(BB_ID)
{
    BB_NONE,
    BB_AMIGADOS_13,
    BB_AMIGADOS_20,
    BB_SCA,
    BB_BYTE_BANDIT
};
typedef BB_ID BootBlockId;

#ifdef __cplusplus
struct BootBlockIdEnum : vamiga::util::Reflection<BootBlockIdEnum, BootBlockId>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = BB_BYTE_BANDIT;
    
    static const char *prefix() { return "BB"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case BB_NONE:         return "NONE";
            case BB_AMIGADOS_13:  return "AMIGADOS_13";
            case BB_AMIGADOS_20:  return "AMIGADOS_20";
            case BB_SCA:          return "SCA";
            case BB_BYTE_BANDIT:  return "BYTE_BANDIT";
        }
        return "???";
    }
};
#endif
