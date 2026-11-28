// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

/// Ramsey chip revision.
enum class RamseyRevision : long
{
    NONE,
    REV04,
    REV07
};

struct RamseyRevisionEnum : utl::Reflectable<RamseyRevisionEnum, RamseyRevision>
{
    static constexpr auto minVal = (long)RamseyRevision::NONE;
    static constexpr auto maxVal = (long)RamseyRevision::REV07;

    static const char *_key(RamseyRevision value)
    {
        switch (value) {

            case RamseyRevision::NONE:   return "NONE";
            case RamseyRevision::REV04:  return "REV04";
            case RamseyRevision::REV07:  return "REV07";
        }
        return "???";
    }
    static const char *help(RamseyRevision value)
    {
        switch (value) {

            case RamseyRevision::NONE:   return "No Ramsey";
            case RamseyRevision::REV04:  return "Ramsey-04";
            case RamseyRevision::REV07:  return "Ramsey-07";
        }
        return "???";
    }
};


//
// Structures
//

/// Configuration options of the Ramsey memory controller.
typedef struct
{
    RamseyRevision revision;                ///< Chip revision
}
RamseyConfig;

}
