// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class BootBlockType
{
    STANDARD,
    VIRUS,
    CUSTOM
};

struct BootBlockTypeEnum : Reflection<BootBlockTypeEnum, BootBlockType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BootBlockType::CUSTOM);
    
    static const char *_key(BootBlockType value)
    {
        switch (value) {
                
            case BootBlockType::STANDARD:  return "STANDARD";
            case BootBlockType::VIRUS:     return "VIRUS";
            case BootBlockType::CUSTOM:    return "CUSTOM";
        }
        return "???";
    }
    static const char *help(BootBlockType value)
    {
        return "";
    }
};

enum class BootBlockId
{
    NONE,
    AMIGADOS_13,
    AMIGADOS_20,
    SCA,
    BYTE_BANDIT
};

struct BootBlockIdEnum : Reflection<BootBlockIdEnum, BootBlockId>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BootBlockId::BYTE_BANDIT);
    
    static const char *_key(BootBlockId value)
    {
        switch (value) {
                
            case BootBlockId::NONE:         return "NONE";
            case BootBlockId::AMIGADOS_13:  return "AMIGADOS_13";
            case BootBlockId::AMIGADOS_20:  return "AMIGADOS_20";
            case BootBlockId::SCA:          return "SCA";
            case BootBlockId::BYTE_BANDIT:  return "BYTE_BANDIT";
        }
        return "???";
    }
    static const char *help(BootBlockId value)
    {
        return "";
    }
};

}
