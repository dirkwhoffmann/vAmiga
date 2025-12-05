// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities/Reflectable.h"

namespace vamiga {

using namespace utl::abilities;

enum class DeviceFault : long
{
    OK,
    UNKNOWN,

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

struct DeviceFaultEnum : Reflectable<DeviceFaultEnum, DeviceFault>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DeviceFault::HDR_UNSUPPORTED);

    static const char *_key(DeviceFault value)
    {
        switch (value) {

            case DeviceFault::OK:                          return "OK";
            case DeviceFault::UNKNOWN:                     return "UNKNOWN";

            case DeviceFault::DSK_MISSING:                 return "DISK_MISSING";
            case DeviceFault::DSK_INCOMPATIBLE:            return "DISK_INCOMPATIBLE";
            case DeviceFault::DSK_INVALID_DIAMETER:        return "DISK_INVALID_DIAMETER";
            case DeviceFault::DSK_INVALID_DENSITY:         return "DISK_INVALID_DENSITY";
            case DeviceFault::DSK_INVALID_LAYOUT:          return "DISK_INVALID_LAYOUT";
            case DeviceFault::DSK_WRONG_SECTOR_COUNT:      return "DISK_WRONG_SECTOR_COUNT";
            case DeviceFault::DSK_INVALID_SECTOR_NUMBER:   return "DISK_INVALID_SECTOR_NUMBER";

            case DeviceFault::HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case DeviceFault::HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case DeviceFault::HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case DeviceFault::HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case DeviceFault::HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case DeviceFault::HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case DeviceFault::HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case DeviceFault::HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case DeviceFault::HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case DeviceFault::HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case DeviceFault::HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case DeviceFault::HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";
        }
        return "???";
    }

    static const char *help(DeviceFault value)
    {
        return "";
    }
};

class DeviceError : public utl::GenericException<DeviceFault>
{
public:

    DeviceError(DeviceFault fault, const std::string &s);
    DeviceError(DeviceFault fault, const char *s) : DeviceError(fault, std::string(s)) { };
    DeviceError(DeviceFault fault, const std::filesystem::path &p) : DeviceError(fault, p.string()) { };
    DeviceError(DeviceFault fault, std::integral auto v) : DeviceError(fault, std::to_string(v)) { };
    DeviceError(DeviceFault fault) : DeviceError(fault, "") { }

    DeviceFault fault() const { return _payload; }
};

}
