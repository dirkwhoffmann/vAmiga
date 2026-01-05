// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MediaError.h"

namespace vamiga {

MediaError::MediaError(long code, const string &s) : Error(code)
{
    switch (code) {

        case SNAP_TOO_OLD:
            set_msg("The snapshot was created with an older version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case SNAP_TOO_NEW:
            set_msg("The snapshot was created with a newer version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case SNAP_IS_BETA:
            set_msg("The snapshot was created with a beta version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case SNAP_CORRUPTED:
            set_msg("The snapshot data is corrupted and has put the"
                    " emulator into an inconsistent state.");
            break;

        case MISSING_ROM_KEY:
            set_msg("No \"rom.key\" file found.");
            break;

        case INVALID_ROM_KEY:
            set_msg("Invalid Rom key.");
            break;

        default:
            set_msg(string("MediaError ") + std::to_string(code) + " (" + errstr() + ").");
            break;
    }
}

}
