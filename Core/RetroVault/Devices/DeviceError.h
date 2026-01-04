// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace retro::vault::device {

struct DeviceError : public Error {

    static constexpr long DEV_OK                        = 0;
    static constexpr long DEV_UNKNOWN                   = 1;

    // Device I/O
    static constexpr long DEV_SEEK_ERR                  = 10;
    static constexpr long DEV_READ_ERR                  = 11;
    static constexpr long DEV_WRITE_ERR                 = 12;

    // Floppy disks
    static constexpr long DSK_MISSING                   = 20;
    static constexpr long DSK_INCOMPATIBLE              = 21;
    static constexpr long DSK_INVALID_DIAMETER          = 22;
    static constexpr long DSK_INVALID_DENSITY           = 23;
    static constexpr long DSK_INVALID_LAYOUT            = 24;
    static constexpr long DSK_WRONG_SECTOR_COUNT        = 25;
    static constexpr long DSK_INVALID_SECTOR_NUMBER     = 26;

    // Hard disks
    static constexpr long HDR_TOO_LARGE                 = 30;
    static constexpr long HDR_UNSUPPORTED_CYL_COUNT     = 31;
    static constexpr long HDR_UNSUPPORTED_HEAD_COUNT    = 32;
    static constexpr long HDR_UNSUPPORTED_SEC_COUNT     = 33;
    static constexpr long HDR_UNSUPPORTED_BSIZE         = 34;
    static constexpr long HDR_UNKNOWN_GEOMETRY          = 35;
    static constexpr long HDR_UNMATCHED_GEOMETRY        = 36;
    static constexpr long HDR_UNPARTITIONED             = 37;
    static constexpr long HDR_CORRUPTED_PTABLE          = 38;
    static constexpr long HDR_CORRUPTED_FSH             = 39;
    static constexpr long HDR_CORRUPTED_LSEG            = 40;
    static constexpr long HDR_UNSUPPORTED               = 41;

    const char *errstr() const noexcept override {

        switch (payload) {

            case DEV_UNKNOWN:                 return "UNKNOWN";

            case DEV_SEEK_ERR:                return "DEV_SEEK_ERR";
            case DEV_READ_ERR:                return "DEV_READ_ERR";
            case DEV_WRITE_ERR:               return "DEV_WRITE_ERR";

            case DSK_MISSING:                 return "DSK_MISSING";
            case DSK_INCOMPATIBLE:            return "DSK_INCOMPATIBLE";
            case DSK_INVALID_DIAMETER:        return "DSK_INVALID_DIAMETER";
            case DSK_INVALID_DENSITY:         return "DSK_INVALID_DENSITY";
            case DSK_INVALID_LAYOUT:          return "DSK_INVALID_LAYOUT";
            case DSK_WRONG_SECTOR_COUNT:      return "DSK_WRONG_SECTOR_COUNT";
            case DSK_INVALID_SECTOR_NUMBER:   return "DSK_INVALID_SECTOR_NUMBER";

            case HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";

            default:
                return "???";
        }
    }

public:

    explicit DeviceError(long fault, const std::string &s = "");
    explicit DeviceError(long fault, const char *s) : DeviceError(fault, std::string(s)) { };
    explicit DeviceError(long fault, const std::filesystem::path &p) : DeviceError(fault, p.string()) { };
    explicit DeviceError(long fault, std::integral auto v) : DeviceError(fault, std::to_string(v)) { };
};

}
