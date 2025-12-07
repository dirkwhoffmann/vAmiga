// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types/Error.h"
#include "utl/abilities/Reflectable.h"

namespace vamiga {

using namespace utl::abilities;

namespace fault {

enum : long
{
    DEV_UNKNOWN,

    // Floppy disks
    DSK_MISSING,
    DSK_INCOMPATIBLE,
    DSK_INVALID_DIAMETER,
    DSK_INVALID_DENSITY,
    DSK_INVALID_LAYOUT,
    DSK_WRONG_SECTOR_COUNT,
    DSK_INVALID_SECTOR_NUMBER,

    // Hard disks
    HDR_TOO_LARGE,
    HDR_UNSUPPORTED_CYL_COUNT,
    HDR_UNSUPPORTED_HEAD_COUNT,
    HDR_UNSUPPORTED_SEC_COUNT,
    HDR_UNSUPPORTED_BSIZE,
    HDR_UNKNOWN_GEOMETRY,
    HDR_UNMATCHED_GEOMETRY,
    HDR_UNPARTITIONED,
    HDR_CORRUPTED_PTABLE,
    HDR_CORRUPTED_FSH,
    HDR_CORRUPTED_LSEG,
    HDR_UNSUPPORTED
};

struct DeviceFaultEnum : Reflectable<DeviceFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDR_UNSUPPORTED;

    static const char *_key(long value)
    {
        switch (value) {

            case DEV_UNKNOWN:                 return "UNKNOWN";

            case DSK_MISSING:                 return "DISK_MISSING";
            case DSK_INCOMPATIBLE:            return "DISK_INCOMPATIBLE";
            case DSK_INVALID_DIAMETER:        return "DISK_INVALID_DIAMETER";
            case DSK_INVALID_DENSITY:         return "DISK_INVALID_DENSITY";
            case DSK_INVALID_LAYOUT:          return "DISK_INVALID_LAYOUT";
            case DSK_WRONG_SECTOR_COUNT:      return "DISK_WRONG_SECTOR_COUNT";
            case DSK_INVALID_SECTOR_NUMBER:   return "DISK_INVALID_SECTOR_NUMBER";

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
        }
        return "???";
    }

    static const char *help(long value)
    {
        return "";
    }
};

}

class DeviceError : public utl::Error
{
public:

    DeviceError(long fault, const std::string &s = "");
    DeviceError(long fault, const char *s) : DeviceError(fault, std::string(s)) { };
    DeviceError(long fault, const std::filesystem::path &p) : DeviceError(fault, p.string()) { };
    DeviceError(long fault, std::integral auto v) : DeviceError(fault, std::to_string(v)) { };
};

}
