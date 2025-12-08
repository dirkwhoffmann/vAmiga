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

        case DMS_CANT_CREATE:
            set_msg("Failed to extract the DMS archive.");
            break;

        case EXT_FACTOR5:
            set_msg("The file is encoded in an outdated format that was"
                    " introduced by Factor 5 to distribute Turrican images."
                    " The format has no relevance today and is not supported"
                    " by the emulator.");
            break;

        case EXT_INCOMPATIBLE:
            set_msg("This file utilizes encoding features of the extended "
                    " ADF format that are not supported by the emulator yet.");
            break;

        case EXT_CORRUPTED:
            set_msg("The disk encoder failed to extract the disk due to "
                    " corrupted or inconsistend file data.");
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
