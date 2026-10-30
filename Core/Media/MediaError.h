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

#include "utl/common.h"

namespace vamiga {

using namespace utl;

/// Error thrown if a media file is invalid or not supported.
struct MediaError : public Error {

    static constexpr long OK                =   0; ///< No error
    static constexpr long UNKNOWN           =   1; ///< Unclassified

    // Snapshots
    static constexpr long SNAP_TOO_OLD      = 10; ///< Snapshot was created with an older version
    static constexpr long SNAP_TOO_NEW      = 11; ///< Snapshot was created with a later version
    static constexpr long SNAP_IS_BETA      = 12; ///< Snapshot was created with a beta release
    static constexpr long SNAP_CORRUPTED    = 13; ///< Snapshot data is corrupted

    // Encrypted Roms
    static constexpr long MISSING_ROM_KEY   = 30;  ///< No "rom.key" file was found (encrypted ROMs)
    static constexpr long INVALID_ROM_KEY   = 31;  ///< The ROM key is invalid (encrypted ROMs)

    const char *errstr() const noexcept override {

        switch (payload) {

            case OK:                          return "OK";
            case UNKNOWN:                     return "UNKNOWN";

            case SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case SNAP_IS_BETA:                return "SNAP_IS_BETA";
            case SNAP_CORRUPTED:              return "SNAP_CORRUPTED";

            case MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
        }
        return "???";
    }

    /// Creates an error with an error code and a descriptive string
    explicit MediaError(long fault, const string &s);
    /// Creates an error with an error code and a descriptive string
    explicit MediaError(long fault, const char *s) : MediaError(fault, string(s)) { };
    /// Creates an error with an error code and a file path
    explicit MediaError(long fault, const fs::path &p) : MediaError(fault, p.string()) { };
    /// Creates an error with an error code and a number
    explicit MediaError(long fault, std::integral auto v) : MediaError(fault, std::to_string(v)) { };
    /// Creates an error with an error code
    explicit MediaError(long fault) : MediaError(fault, "") { }
};

}
