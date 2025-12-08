// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types.h"

namespace vamiga {

using namespace utl;

struct MediaError : public Error {

    static constexpr long OK                =   0; ///< No error
    static constexpr long UNKNOWN           =   1; ///< Unclassified

    // Snapshots
    static constexpr long SNAP_TOO_OLD      = 10; ///< Snapshot was created with an older version
    static constexpr long SNAP_TOO_NEW      = 11; ///< Snapshot was created with a later version
    static constexpr long SNAP_IS_BETA      = 12; ///< Snapshot was created with a beta release
    static constexpr long SNAP_CORRUPTED    = 13; ///< Snapshot data is corrupted

    // Disk images
    static constexpr long DMS_CANT_CREATE   = 20;
    static constexpr long EXT_FACTOR5       = 21;
    static constexpr long EXT_INCOMPATIBLE  = 22;
    static constexpr long EXT_CORRUPTED     = 23;

    // Encrypted Roms
    static constexpr long MISSING_ROM_KEY   = 30;
    static constexpr long INVALID_ROM_KEY   = 31;

    const char *errstr() const noexcept override {

        switch (payload) {

            case OK:                          return "OK";
            case UNKNOWN:                     return "UNKNOWN";

            case SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case SNAP_IS_BETA:                return "SNAP_IS_BETA";
            case SNAP_CORRUPTED:              return "SNAP_CORRUPTED";

            case DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case EXT_CORRUPTED:               return "EXT_CORRUPTED";

            case MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
        }
        return "???";
    }

    explicit MediaError(long fault, const string &s);
    explicit MediaError(long fault, const char *s) : CoreError(fault, string(s)) { };
    explicit MediaError(long fault, const fs::path &p) : CoreError(fault, p.string()) { };
    explicit MediaError(long fault, std::integral auto v) : CoreError(fault, std::to_string(v)) { };
    explicit MediaError(long fault) : CoreError(fault, "") { }
};

}
