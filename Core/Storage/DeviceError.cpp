// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DeviceTypes.h"
#include "DeviceError.h"

namespace vamiga {

DeviceError::DeviceError(DeviceFault code, const string &s) : utl::exception(code)
{
    switch (code)
    {
        case DeviceFault::DSK_MISSING:
            set_msg("No disk in drive.");
            break;

        case DeviceFault::DSK_INCOMPATIBLE:
            set_msg("This disk is not compatible with the selected drive.");
            break;

        case DeviceFault::DSK_INVALID_DIAMETER:
            set_msg("Invalid disk diameter.");
            break;

        case DeviceFault::DSK_INVALID_DENSITY:
            set_msg("Invalid disk density.");
            break;

        case DeviceFault::DSK_INVALID_LAYOUT:
            set_msg("The disk density and disk diameter do not match.");
            break;

        case DeviceFault::DSK_WRONG_SECTOR_COUNT:
            set_msg("Unable to decode the MFM bit stream (wrong sector count).");
            break;

        case DeviceFault::DSK_INVALID_SECTOR_NUMBER:
            set_msg("Unable to decode the MFM bit stream (invalid sector number).");
            break;

        case DeviceFault::HDR_TOO_LARGE:
            set_msg(string("vAmiga supports hard drives with a maximum capacity of ") +
                    "504 MB.");
            break;

        case DeviceFault::HDR_UNSUPPORTED_CYL_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_C_MIN) + " and " +
                    "at most " + std::to_string(HDR_C_MAX) + " cylinders. " +
                    "This drive has " + s + " cylinders.");
            break;

        case DeviceFault::HDR_UNSUPPORTED_HEAD_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_H_MIN) + " and " +
                    "at most " + std::to_string(HDR_H_MAX) + " heads. " +
                    "The drive has " + s + " heads.");
            break;

        case DeviceFault::HDR_UNSUPPORTED_SEC_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with " +
                    "at least " + std::to_string(HDR_S_MIN) + " and " +
                    "at most " + std::to_string(HDR_S_MAX) + " sectors. " +
                    "The drive stores " + s + " sectors per track.");
            break;

        case DeviceFault::HDR_UNSUPPORTED_BSIZE:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with a " +
                    "block size of 512 bytes. " +
                    "The drive stores " + s + " bytes per block.");
            break;

        case DeviceFault::HDR_UNKNOWN_GEOMETRY:
            set_msg("vAmiga failed to derive to geometry of this drive.");
            break;

        case DeviceFault::HDR_UNMATCHED_GEOMETRY:
            set_msg("The drive geometry doesn't match the hard drive capacity.");
            break;

        case DeviceFault::HDR_UNPARTITIONED:
            set_msg("The hard drive has no partitions.");
            break;

        case DeviceFault::HDR_CORRUPTED_PTABLE:
            set_msg("Invalid partition table.");
            break;

        case DeviceFault::HDR_CORRUPTED_FSH:
            set_msg("Invalid file system header block.");
            break;

        case DeviceFault::HDR_UNSUPPORTED:
            set_msg("The hard drive is encoded in an unknown or unsupported format.");
            break;

        default:
            set_msg(string("Error code ") + std::to_string((i64)fault()) +
            " (" + DeviceFaultEnum::key(fault()) + ").");
            break;
    }
}

}
