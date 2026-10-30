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

#include "DeviceTypes.h"
#include "Aliases.h"

namespace vamiga {


//
// Enumerations
//

/// Flags describing the state of a disk.
enum class DiskFlags
{
    PROTECTED  = 1,
    MODIFIED   = 2,
    BOOTABLE   = 4
};

struct DiskFlagsEnum : utl::Reflectable<DiskFlagsEnum, DiskFlags>
{
    static constexpr long minVal = 1;
    static constexpr long maxVal = long(DiskFlags::BOOTABLE);
    
    static const char *_key(DiskFlags value)
    {
        switch (value) {
                
            case DiskFlags::PROTECTED:    return "PROTECTED";
            case DiskFlags::MODIFIED:     return "MODIFIED";
            case DiskFlags::BOOTABLE:     return "BOOTABLE";
        }
        return "???";
    }
    static const char *help(DiskFlags value)
    {
        switch (value) {
                
            case DiskFlags::PROTECTED:    return "Write protected disk";
            case DiskFlags::MODIFIED:     return "Modified disk contents";
            case DiskFlags::BOOTABLE:     return "Bootable disk";
        }
        return "???";
    }
};


//
// Structures
//

/// Position of a drive head (cylinder, head, and offset into the track).
struct DriveHead
{
    retro::vault::CylNr cylinder;       ///< Cylinder the head is positioned over
    retro::vault::HeadNr head;          ///< Active head (0 or 1)
    isize offset;                       ///< Byte offset within the current track

    /// Returns the track number (2 * cylinder + head).
    isize track() const { return 2 * cylinder + head; }
};

}
