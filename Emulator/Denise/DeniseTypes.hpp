// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*
#pragma once

#include "Reflection.hpp"

namespace va {

#include "DeniseTypes.h"

//
// Reflection APIs
//

struct DeniseRevisionEnum : Reflection<DeniseRevisionEnum, DeniseRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < DENISE_COUNT;
    }

    static const char *prefix() { return "DENISE"; }
    static const char *key(DeniseRevision value)
    {
        switch (value) {
                
            case DENISE_OCS:          return "OCS";
            case DENISE_OCS_BRDRBLNK: return "OCS_BRDRBLNK";
            case DENISE_ECS:          return "ECS";
            case DENISE_COUNT:        return "???";
        }
        return "???";
    }
};

}
*/
