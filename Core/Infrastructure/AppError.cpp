// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AppError.h"
#include "Constants.h"
#include "Macros.h"
#include "DeviceTypes.h"

namespace vamiga {

AppError::AppError(long code, const string &s) : Error(code)
{
    // string _msg;
    
    switch (code)
    {

        case OK:
            fatalError;
            break;
            
        case POWERED_OFF:
            set_msg("The emulator is powered off.");
            break;

        case POWERED_ON:
            set_msg("The emulator is powered on.");
            break;

        case DEBUG_OFF:
            set_msg("Debug mode is switched off.");
            break;

        case RUNNING:
            set_msg("The emulator is running.");
            break;

        case OPT_UNSUPPORTED:
            set_msg(s == "" ? "This option is not supported yet." : s);
            break;
            
        case OPT_INV_ARG:
            set_msg("Invalid argument. Expected: " + s);
            break;

        case OPT_INV_ID:
            set_msg("Invalid component ID. Expected: " + s);
            break;

        case OPT_LOCKED:
            set_msg("This option is locked because the Amiga is powered on.");
            break;

        case INVALID_KEY:
            set_msg("Invalid key: " + s + ".");
            break;
            
        case SYNTAX:
            set_msg("Syntax error" + (s.empty() ? "" : " in line " + s + "."));
            break;

        case CPU_UNSUPPORTED:
            set_msg("CPU revision is not supported yet.");
            break;

        case GUARD_NOT_FOUND:
            set_msg("Entry " + s + " not found.");
            break;

        case GUARD_ALREADY_SET:
            set_msg("Target " + s + " is already observed.");
            break;

        case BP_NOT_FOUND:
            set_msg("Breakpoint " + s + " not found.");
            break;
            
        case BP_ALREADY_SET:
            set_msg("A breakpoint at " + s + " is already set.");
            break;
            
        case WP_NOT_FOUND:
            set_msg("Watchpoint " + s + " not found.");
            break;

        case WP_ALREADY_SET:
            set_msg("A watchpoint at " + s + " is already set.");
            break;

        case CP_NOT_FOUND:
            set_msg("Catchpoint " + s + " not found.");
            break;

        case CP_ALREADY_SET:
            set_msg("This catchpoint is already set.");
            break;

        case OUT_OF_MEMORY:
            set_msg("Out of memory.");
            break;
            
        case CHIP_RAM_MISSING:
            set_msg("No Chip RAM installed.");
            break;
            
        case CHIP_RAM_LIMIT:
            set_msg(string("The selected Agnus revision is not able to address") +
                    " the selected amount of Chip RAM.");
            break;

        case AROS_RAM_LIMIT:
            set_msg("The Aros Kickstart requires at least 1 MB of memory.");
            break;

        case ROM_MISSING:
            set_msg("No Rom installed.");
            break;
            
        case AROS_NO_EXTROM:
            set_msg("No Extension Rom installed.");
            break;

        case DISK_MISSING:
            set_msg("No disk in drive.");
            break;
            
        case DISK_INCOMPATIBLE:
            set_msg("This disk is not compatible with the selected drive.");
            break;
            
        case DISK_INVALID_DIAMETER:
            set_msg("Invalid disk diameter.");
            break;

        case DISK_INVALID_DENSITY:
            set_msg("Invalid disk density.");
            break;

        case DISK_INVALID_LAYOUT:
            set_msg("The disk density and disk diameter do not match.");
            break;

        case DISK_WRONG_SECTOR_COUNT:
            set_msg("Unable to decode the MFM bit stream (wrong sector count).");
            break;
            
        case DISK_INVALID_SECTOR_NUMBER:
            set_msg("Unable to decode the MFM bit stream (invalid sector number).");
            break;

        case HDR_TOO_LARGE:
            set_msg(string("vAmiga supports hard drives with a maximum capacity of ") +
                    "504 MB.");
            break;

        case HDR_UNSUPPORTED_CYL_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_C_MIN) + " and " +
                    "at most " + std::to_string(HDR_C_MAX) + " cylinders. " +
                    "This drive has " + s + " cylinders.");
            break;

        case HDR_UNSUPPORTED_HEAD_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_H_MIN) + " and " +
                    "at most " + std::to_string(HDR_H_MAX) + " heads. " +
                    "The drive has " + s + " heads.");
            break;

        case HDR_UNSUPPORTED_SEC_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with " +
                    "at least " + std::to_string(HDR_S_MIN) + " and " +
                    "at most " + std::to_string(HDR_S_MAX) + " sectors. " +
                    "The drive stores " + s + " sectors per track.");
            break;

        case HDR_UNSUPPORTED_BSIZE:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with a " +
                    "block size of 512 bytes. " +
                    "The drive stores " + s + " bytes per block.");
            break;

        case HDR_UNKNOWN_GEOMETRY:
            set_msg("vAmiga failed to derive to geometry of this drive.");
            break;

        case HDR_UNMATCHED_GEOMETRY:
            set_msg("The drive geometry doesn't match the hard drive capacity.");
            break;

        case HDR_UNPARTITIONED:
            set_msg("The hard drive has no partitions.");
            break;

        case HDR_CORRUPTED_PTABLE:
            set_msg("Invalid partition table.");
            break;

        case HDR_CORRUPTED_FSH:
            set_msg("Invalid file system header block.");
            break;

        case HDR_UNSUPPORTED:
            set_msg("The hard drive is encoded in an unknown or unsupported format.");
            break;

        case HDC_INIT:
            set_msg("Failed to initialize hard drive: " + s);
            break;

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
 
        case ZLIB_ERROR:
            set_msg(s);
            break;
            
        case MISSING_ROM_KEY:
            set_msg("No \"rom.key\" file found.");
            break;

        case INVALID_ROM_KEY:
            set_msg("Invalid Rom key.");
            break;

        case REG_READ_ONLY:
            set_msg(s + " is a read-only register.");
            break;

        case REG_WRITE_ONLY:
            set_msg(s + " is a write-only register.");
            break;

        case REG_UNUSED:
            set_msg("Register " + s + " is unused.");
            break;

        case ADDR_UNALIGNED:
            set_msg("Address not aligned");
            break;

        case OSDB:
            set_msg("OS Debugger: " + s);
            break;
            
        case HUNK_BAD_COOKIE:
            set_msg("Invalid magic cookie.");
            break;

        case HUNK_BAD_HEADER:
            set_msg("Bad header.");
            break;

        case HUNK_NO_SECTIONS:
            set_msg("No hunks found.");
            break;

        case HUNK_UNSUPPORTED:
            set_msg("Unsupported hunk: " + s);
            break;

        case HUNK_CORRUPTED:
            set_msg("Corrupted hunk structure.");
            break;

        default:
            set_msg(string("CoreError ") + std::to_string(code) + " (" + errstr() + ").");
            break;
    }
}

}
