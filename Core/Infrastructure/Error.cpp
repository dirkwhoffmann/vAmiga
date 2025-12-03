// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Error.h"
#include "Constants.h"
#include "Macros.h"
#include "DeviceTypes.h"

namespace vamiga {

AppError::AppError(Fault code, const string &s) : utl::exception(code)
{
    // string _msg;
    
    switch (code)
    {

        case Fault::OK:
            fatalError;
            break;
            
        case Fault::POWERED_OFF:
            set_msg("The emulator is powered off.");
            break;

        case Fault::POWERED_ON:
            set_msg("The emulator is powered on.");
            break;

        case Fault::DEBUG_OFF:
            set_msg("Debug mode is switched off.");
            break;

        case Fault::RUNNING:
            set_msg("The emulator is running.");
            break;

        case Fault::OPT_UNSUPPORTED:
            set_msg(s == "" ? "This option is not supported yet." : s);
            break;
            
        case Fault::OPT_INV_ARG:
            set_msg("Invalid argument. Expected: " + s);
            break;

        case Fault::OPT_INV_ID:
            set_msg("Invalid component ID. Expected: " + s);
            break;

        case Fault::OPT_LOCKED:
            set_msg("This option is locked because the Amiga is powered on.");
            break;

        case Fault::INVALID_KEY:
            set_msg("Invalid key: " + s + ".");
            break;
            
        case Fault::SYNTAX:
            set_msg("Syntax error" + (s.empty() ? "" : " in line " + s + "."));
            break;

        case Fault::CPU_UNSUPPORTED:
            set_msg("CPU revision is not supported yet.");
            break;

        case Fault::GUARD_NOT_FOUND:
            set_msg("Entry " + s + " not found.");
            break;

        case Fault::GUARD_ALREADY_SET:
            set_msg("Target " + s + " is already observed.");
            break;

        case Fault::BP_NOT_FOUND:
            set_msg("Breakpoint " + s + " not found.");
            break;
            
        case Fault::BP_ALREADY_SET:
            set_msg("A breakpoint at " + s + " is already set.");
            break;
            
        case Fault::WP_NOT_FOUND:
            set_msg("Watchpoint " + s + " not found.");
            break;

        case Fault::WP_ALREADY_SET:
            set_msg("A watchpoint at " + s + " is already set.");
            break;

        case Fault::CP_NOT_FOUND:
            set_msg("Catchpoint " + s + " not found.");
            break;

        case Fault::CP_ALREADY_SET:
            set_msg("This catchpoint is already set.");
            break;

        case Fault::DIR_NOT_FOUND:
            set_msg("Folder \"" + s + "\" not found.");
            break;

        case Fault::DIR_ACCESS_DENIED:
            set_msg("Unable to access folder \"" + s + "\". Permission denied.");
            break;

        case Fault::FILE_NOT_FOUND:
            set_msg("File \"" + s + "\" not found.");
            break;

        case Fault::FILE_EXISTS:
            set_msg("File \"" + s + "\" already exists.");
            break;

        case Fault::FILE_IS_DIRECTORY:
            if (s.empty()) {
                set_msg("The selected file is a directory.");
            } else {
                set_msg("File \"" + s + "\" is a directory.");
            }
            break;

        case Fault::FILE_ACCESS_DENIED:
            set_msg("Unable to access file \"" + s + "\". Permission denied.");
            break;

        case Fault::FILE_TYPE_MISMATCH:
            set_msg("The file content and the file type do not match.");
            break;

        case Fault::FILE_CANT_READ:
            set_msg("Failed to read from file \"" + s + "\".");
            break;
            
        case Fault::FILE_CANT_WRITE:
            set_msg("Failed to write to file \"" + s + "\".");
            break;
            
        case Fault::FILE_CANT_CREATE:
            set_msg("Failed to create file \"" + s + "\".");
            break;
            
        case Fault::OUT_OF_MEMORY:
            set_msg("Out of memory.");
            break;
            
        case Fault::CHIP_RAM_MISSING:
            set_msg("No Chip RAM installed.");
            break;
            
        case Fault::CHIP_RAM_LIMIT:
            set_msg(string("The selected Agnus revision is not able to address") +
                    " the selected amount of Chip RAM.");
            break;

        case Fault::AROS_RAM_LIMIT:
            set_msg("The Aros Kickstart requires at least 1 MB of memory.");
            break;

        case Fault::ROM_MISSING:
            set_msg("No Rom installed.");
            break;
            
        case Fault::AROS_NO_EXTROM:
            set_msg("No Extension Rom installed.");
            break;

        case Fault::DISK_MISSING:
            set_msg("No disk in drive.");
            break;
            
        case Fault::DISK_INCOMPATIBLE:
            set_msg("This disk is not compatible with the selected drive.");
            break;
            
        case Fault::DISK_INVALID_DIAMETER:
            set_msg("Invalid disk diameter.");
            break;

        case Fault::DISK_INVALID_DENSITY:
            set_msg("Invalid disk density.");
            break;

        case Fault::DISK_INVALID_LAYOUT:
            set_msg("The disk density and disk diameter do not match.");
            break;

        case Fault::DISK_WRONG_SECTOR_COUNT:
            set_msg("Unable to decode the MFM bit stream (wrong sector count).");
            break;
            
        case Fault::DISK_INVALID_SECTOR_NUMBER:
            set_msg("Unable to decode the MFM bit stream (invalid sector number).");
            break;

        case Fault::HDR_TOO_LARGE:
            set_msg(string("vAmiga supports hard drives with a maximum capacity of ") +
                    "504 MB.");
            break;

        case Fault::HDR_UNSUPPORTED_CYL_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_C_MIN) + " and " +
                    "at most " + std::to_string(HDR_C_MAX) + " cylinders. " +
                    "This drive has " + s + " cylinders.");
            break;

        case Fault::HDR_UNSUPPORTED_HEAD_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga supports hard drives with " +
                    "at least " + std::to_string(HDR_H_MIN) + " and " +
                    "at most " + std::to_string(HDR_H_MAX) + " heads. " +
                    "The drive has " + s + " heads.");
            break;

        case Fault::HDR_UNSUPPORTED_SEC_COUNT:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with " +
                    "at least " + std::to_string(HDR_S_MIN) + " and " +
                    "at most " + std::to_string(HDR_S_MAX) + " sectors. " +
                    "The drive stores " + s + " sectors per track.");
            break;

        case Fault::HDR_UNSUPPORTED_BSIZE:
            set_msg(string("The geometry of this drive is not supported. ") +
                    "vAmiga only supports hard drives with a " +
                    "block size of 512 bytes. " +
                    "The drive stores " + s + " bytes per block.");
            break;

        case Fault::HDR_UNKNOWN_GEOMETRY:
            set_msg("vAmiga failed to derive to geometry of this drive.");
            break;

        case Fault::HDR_UNMATCHED_GEOMETRY:
            set_msg("The drive geometry doesn't match the hard drive capacity.");
            break;

        case Fault::HDR_UNPARTITIONED:
            set_msg("The hard drive has no partitions.");
            break;

        case Fault::HDR_CORRUPTED_PTABLE:
            set_msg("Invalid partition table.");
            break;

        case Fault::HDR_CORRUPTED_FSH:
            set_msg("Invalid file system header block.");
            break;

        case Fault::HDR_UNSUPPORTED:
            set_msg("The hard drive is encoded in an unknown or unsupported format.");
            break;

        case Fault::HDC_INIT:
            set_msg("Failed to initialize hard drive: " + s);
            break;

        case Fault::SNAP_TOO_OLD:
            set_msg("The snapshot was created with an older version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case Fault::SNAP_TOO_NEW:
            set_msg("The snapshot was created with a newer version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case Fault::SNAP_IS_BETA:
            set_msg("The snapshot was created with a beta version of vAmiga"
                    " and is incompatible with this release.");
            break;

        case Fault::SNAP_CORRUPTED:
            set_msg("The snapshot data is corrupted and has put the"
                    " emulator into an inconsistent state.");
            break;

        case Fault::DMS_CANT_CREATE:
            set_msg("Failed to extract the DMS archive.");
            break;

        case Fault::EXT_FACTOR5:
            set_msg("The file is encoded in an outdated format that was"
                    " introduced by Factor 5 to distribute Turrican images."
                    " The format has no relevance today and is not supported"
                    " by the emulator.");
            break;

        case Fault::EXT_INCOMPATIBLE:
            set_msg("This file utilizes encoding features of the extended "
                    " ADF format that are not supported by the emulator yet.");
            break;

        case Fault::EXT_CORRUPTED:
            set_msg("The disk encoder failed to extract the disk due to "
                    " corrupted or inconsistend file data.");
            break;
 
        case Fault::ZLIB_ERROR:
            set_msg(s);
            break;
            
        case Fault::MISSING_ROM_KEY:
            set_msg("No \"rom.key\" file found.");
            break;

        case Fault::INVALID_ROM_KEY:
            set_msg("Invalid Rom key.");
            break;

        case Fault::REC_LAUNCH:
            set_msg(s);
            break;

        case Fault::REG_READ_ONLY:
            set_msg(s + " is a read-only register.");
            break;

        case Fault::REG_WRITE_ONLY:
            set_msg(s + " is a write-only register.");
            break;

        case Fault::REG_UNUSED:
            set_msg("Register " + s + " is unused.");
            break;

        case Fault::ADDR_UNALIGNED:
            set_msg("Address not aligned");
            break;

        case Fault::OSDB:
            set_msg("OS Debugger: " + s);
            break;
            
        case Fault::HUNK_BAD_COOKIE:
            set_msg("Invalid magic cookie.");
            break;

        case Fault::HUNK_BAD_HEADER:
            set_msg("Bad header.");
            break;

        case Fault::HUNK_NO_SECTIONS:
            set_msg("No hunks found.");
            break;

        case Fault::HUNK_UNSUPPORTED:
            set_msg("Unsupported hunk: " + s);
            break;

        case Fault::HUNK_CORRUPTED:
            set_msg("Corrupted hunk structure.");
            break;

            /*
        case Fault::FS_UNINITIALIZED:
            set_msg("No file system present.");
            break;

        case Fault::FS_INVALID_PATH:
            set_msg("Invalid path: " + s);
            break;

        case Fault::FS_INVALID_REGEX:
            set_msg("Invalid search pattern: " + s);
            break;

        case Fault::FS_NOT_A_DIRECTORY:
            set_msg(s.empty() ? "Not a directory." : s + " is not a directory.");
            break;

        case Fault::FS_NOT_A_FILE:
            set_msg(s.empty() ? "Not a file." : s + " is not a file.");
            break;

        case Fault::FS_NOT_A_FILE_OR_DIRECTORY:
            set_msg(s.empty() ? "Not a file or directory." : s + " is not a file or directory.");
            break;

        case Fault::FS_NOT_FOUND:
            set_msg(s.empty() ? "Not found." : s + " not found.");
            break;

        case Fault::FS_EXISTS:
            set_msg(s.empty() ? "Item already exists." : s + " already exists.");
            break;

        case Fault::FS_CANNOT_OPEN:
            set_msg("Cannot open file" + (s.empty() ? "" : " " + s) + ".");
            break;

        case Fault::FS_UNFORMATTED:
            set_msg("Unformatted device.");
            break;

        case Fault::FS_UNSUPPORTED:
            set_msg("Unsupported file system.");
            break;

        case Fault::FS_READ_ONLY:
            set_msg("Read-only file system.");
            break;

        case Fault::FS_WRONG_BSIZE:
            set_msg("Invalid block size.");
            break;

        case Fault::FS_WRONG_DOS_TYPE:
            set_msg("Wrong DOS type.");
            break;

        case Fault::FS_WRONG_CAPACITY:
            set_msg("Wrong file system capacity.");
            break;

        case Fault::FS_HAS_CYCLES:
            set_msg("Cyclic reference chain detected.");
            break;

        case Fault::FS_CORRUPTED:
            set_msg("Corrupted file system.");
            break;

        case Fault::FS_OUT_OF_SPACE:
            set_msg("File system capacity exceeded.");
            break;
                        
        case Fault::FS_DIR_NOT_EMPTY:
            set_msg("Directory is not empty.");
            break;

        case Fault::FS_CANNOT_CREATE_DIR:
            set_msg("Unable to create directory.");
            break;

        case Fault::FS_CANNOT_CREATE_FILE:
            set_msg("Unable to create file.");
            break;
             */
            
        default:
            set_msg(string("Error code ") + std::to_string((i64)fault()) +
            " (" + FaultEnum::key(fault()) + ").");
            break;
    }
}

}
