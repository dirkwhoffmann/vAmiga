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

namespace vamiga {

Error::Error(ErrorCode code, const string &s)
{
    data = code;
    
    switch (code) {
            
        case VAERROR_OK:
            fatalError;
            break;
            
        case VAERROR_POWERED_OFF:
            description = "The emulator is powered off.";
            break;

        case VAERROR_POWERED_ON:
            description = "The emulator is powered on.";
            break;

        case VAERROR_DEBUG_OFF:
            description = "Debug mode is switched off.";
            break;

        case VAERROR_RUNNING:
            description = "The emulator is running.";
            break;

        case VAERROR_OPT_UNSUPPORTED:
            description = s == "" ? "This option is not supported yet." : s;
            break;
            
        case VAERROR_OPT_INV_ARG:
            description = "Invalid argument. Expected: " + s;
            break;

        case VAERROR_OPT_INV_ID:
            description = "Invalid component ID. Expected: " + s;
            break;

        case VAERROR_OPT_LOCKED:
            description = "This option is locked because the Amiga is powered on.";
            break;

        case VAERROR_INVALID_KEY:
            description = "Invalid key: " + s + ".";
            break;
            
        case VAERROR_SYNTAX:
            description = "Syntax error in line " + s + ".";
            break;

        case VAERROR_CPU_UNSUPPORTED:
            description = "CPU revision is not supported yet.";
            break;

        case VAERROR_GUARD_NOT_FOUND:
            description = "Entry " + s + " not found.";
            break;

        case VAERROR_GUARD_ALREADY_SET:
            description = "Target " + s + " is already observed.";
            break;

        case VAERROR_BP_NOT_FOUND:
            description = "Breakpoint " + s + " not found.";
            break;
            
        case VAERROR_BP_ALREADY_SET:
            description = "A breakpoint at " + s + " is already set.";
            break;
            
        case VAERROR_WP_NOT_FOUND:
            description = "Watchpoint " + s + " not found.";
            break;

        case VAERROR_WP_ALREADY_SET:
            description = "A watchpoint at " + s + " is already set.";
            break;

        case VAERROR_CP_NOT_FOUND:
            description = "Catchpoint " + s + " not found.";
            break;

        case VAERROR_CP_ALREADY_SET:
            description = "This catchpoint is already set.";
            break;

        /*
        case VAERROR_CP_CANT_CATCH:
            description = "Exception vector " + s + " cannot be catched.";
            break;
        */

        case VAERROR_DIR_NOT_FOUND:
            description = "Folder \"" + s + "\" not found.";
            break;

        case VAERROR_DIR_ACCESS_DENIED:
            description = "Unable to access folder \"" + s + "\". Permission denied.";
            break;

        case VAERROR_FILE_NOT_FOUND:
            description = "File \"" + s + "\" not found.";
            break;

        case VAERROR_FILE_EXISTS:
            description = "File \"" + s + "\" already exists.";
            break;

        case VAERROR_FILE_IS_DIRECTORY:
            if (s.empty()) {
                description = "The selected file is a directory.";
            } else {
                description = "File \"" + s + "\" is a directory.";
            }
            break;

        case VAERROR_FILE_ACCESS_DENIED:
            description = "Unable to access file \"" + s + "\". Permission denied.";
            break;

        case VAERROR_FILE_TYPE_MISMATCH:
            description = "The file content and the file type do not match.";
            break;
            
        case VAERROR_FILE_CANT_READ:
            description = "Failed to read from file \"" + s + "\".";
            break;
            
        case VAERROR_FILE_CANT_WRITE:
            description = "Failed to write to file \"" + s + "\".";
            break;
            
        case VAERROR_FILE_CANT_CREATE:
            description = "Failed to create file \"" + s + "\".";
            break;
            
        case VAERROR_OUT_OF_MEMORY:
            description = "Out of memory.";
            break;
            
        case VAERROR_CHIP_RAM_MISSING:
            description = "No Chip RAM installed.";
            break;
            
        case VAERROR_CHIP_RAM_LIMIT:
            description = "The selected Agnus revision is not able to address";
            description += " the selected amount of Chip RAM.";
            break;

        case VAERROR_AROS_RAM_LIMIT:
            description = "The Aros Kickstart requires at least 1 MB of memory.";
            break;

        case VAERROR_ROM_MISSING:
            description = "No Rom installed.";
            break;
            
        case VAERROR_AROS_NO_EXTROM:
            description = "No Extension Rom installed.";
            break;

        case VAERROR_WT_BLOCKED:
            description = "The storage file for the selected hard drive is";
            description += " being used by another emulator instance. It cannot ";
            description += " be shared among multiple emulator instances.";
            break;

        case VAERROR_WT:
            description = "Write through: " + s;
            break;

        case VAERROR_DISK_MISSING:
            description = "No disk in drive.";
            break;
            
        case VAERROR_DISK_INCOMPATIBLE:
            description = "This disk is not compatible with the selected drive.";
            break;
            
        case VAERROR_DISK_INVALID_DIAMETER:
            description = "Invalid disk diameter.";
            break;

        case VAERROR_DISK_INVALID_DENSITY:
            description = "Invalid disk density.";
            break;

        case VAERROR_DISK_INVALID_LAYOUT:
            description = "The disk density and disk diameter do not match.";
            break;

        case VAERROR_DISK_WRONG_SECTOR_COUNT:
            description = "Unable to decode the MFM bit stream (wrong sector count).";
            break;
            
        case VAERROR_DISK_INVALID_SECTOR_NUMBER:
            description = "Unable to decode the MFM bit stream (invalid sector number).";
            break;

        case VAERROR_HDR_TOO_LARGE:
            description = "vAmiga supports hard drives with a maximum capacity of ";
            description += "504 MB.";
            break;

        case VAERROR_HDR_UNSUPPORTED_CYL_COUNT:
            description = "The geometry of this drive is not supported. ";
            description += "vAmiga supports hard drives with ";
            description += "at least " + std::to_string(HDR_C_MIN) + " and ";
            description += "at most " + std::to_string(HDR_C_MAX) + " cylinders. ";
            description += "This drive has " + s + " cylinders.";
            break;
            
        case VAERROR_HDR_UNSUPPORTED_HEAD_COUNT:
            description = "The geometry of this drive is not supported. ";
            description += "vAmiga supports hard drives with ";
            description += "at least " + std::to_string(HDR_H_MIN) + " and ";
            description += "at most " + std::to_string(HDR_H_MAX) + " heads. ";
            description += "The drive has " + s + " heads.";
            break;

        case VAERROR_HDR_UNSUPPORTED_SEC_COUNT:
            description = "The geometry of this drive is not supported. ";
            description += "vAmiga only supports hard drives with ";
            description += "at least " + std::to_string(HDR_S_MIN) + " and ";
            description += "at most " + std::to_string(HDR_S_MAX) + " sectors. ";
            description += "The drive stores " + s + " sectors per track.";
            break;

        case VAERROR_HDR_UNSUPPORTED_BSIZE:
            description = "The geometry of this drive is not supported. ";
            description += "vAmiga only supports hard drives with a ";
            description += "block size of 512 bytes. ";
            description += "The drive stores " + s + " bytes per block.";
            break;

        case VAERROR_HDR_UNKNOWN_GEOMETRY:
            description = "vAmiga failed to derive to geometry of this drive.";
            break;

        case VAERROR_HDR_UNMATCHED_GEOMETRY:
            description = "The drive geometry doesn't match the hard drive capacity.";
            break;

        case VAERROR_HDR_UNPARTITIONED:
            description = "The hard drive has no partitions.";
            break;

        case VAERROR_HDR_CORRUPTED_PTABLE:
            description = "Invalid partition table.";
            break;

        case VAERROR_HDR_CORRUPTED_FSH:
            description = "Invalid file system header block.";
            break;

        case VAERROR_HDR_UNSUPPORTED:
            description = "The hard drive is encoded in an unknown or unsupported format.";
            break;

        case VAERROR_HDC_INIT:
            description = "Failed to initialize hard drive: " + s;
            break;

        case VAERROR_SNAP_TOO_OLD:
            description = "The snapshot was created with an older version of vAmiga";
            description += " and is incompatible with this release.";
            break;

        case VAERROR_SNAP_TOO_NEW:
            description = "The snapshot was created with a newer version of vAmiga";
            description += " and is incompatible with this release.";
            break;

        case VAERROR_SNAP_IS_BETA:
            description = "The snapshot was created with a beta version of vAmiga";
            description += " and is incompatible with this release.";
            break;

        case VAERROR_SNAP_CORRUPTED:
            description = "The snapshot data is corrupted and has put the";
            description += " emulator into an inconsistent state.";
            break;

        case VAERROR_DMS_CANT_CREATE:
            description = "Failed to extract the DMS archive.";
            break;

        case VAERROR_EXT_FACTOR5:
            description = "The file is encoded in an outdated format that was";
            description += " introduced by Factor 5 to distribute Turrican images.";
            description += " The format has no relevance today and is not supported";
            description += " by the emulator.";
            break;

        case VAERROR_EXT_INCOMPATIBLE:
            description = "This file utilizes encoding features of the extended ";
            description += " ADF format that are not supported by the emulator yet.";
            break;

        case VAERROR_EXT_CORRUPTED:
            description = "The disk encoder failed to extract the disk due to ";
            description += " corrupted or inconsistend file data.";
            break;
            
        case VAERROR_MISSING_ROM_KEY:
            description = "No \"rom.key\" file found.";
            break;

        case VAERROR_INVALID_ROM_KEY:
            description = "Invalid Rom key.";
            break;

        case VAERROR_REC_LAUNCH:
            description = s;
            break;

        case VAERROR_REG_READ_ONLY:
            description = s + " is a read-only register";
            break;

        case VAERROR_REG_WRITE_ONLY:
            description = s + " is a write-only register";
            break;

        case VAERROR_REG_UNUSED:
            description = "Register " + s + " is unused";
            break;

        case VAERROR_ADDR_UNALIGNED:
            description = "Address not aligned";
            break;

        case VAERROR_OSDB:
            description = "OS Debugger: " + s;
            break;
            
        case VAERROR_HUNK_BAD_COOKIE:
            description = "Invalid magic cookie.";
            break;

        case VAERROR_HUNK_BAD_HEADER:
            description = "Bad header.";
            break;

        case VAERROR_HUNK_NO_SECTIONS:
            description = "No hunks found.";
            break;

        case VAERROR_HUNK_UNSUPPORTED:
            description = "Unsupported hunk: " + s;
            break;

        case VAERROR_HUNK_CORRUPTED:
            description = "Corrupted hunk structure.";
            break;

        case VAERROR_FS_UNSUPPORTED:
            description = "Unsupported file system.";
            break;

        case VAERROR_FS_UNFORMATTED:
            description = "Unformatted device.";
            break;

        case VAERROR_FS_WRONG_BSIZE:
            description = "Invalid block size.";
            break;

        case VAERROR_FS_WRONG_DOS_TYPE:
            description = "Wrong DOS type.";
            break;

        case VAERROR_FS_WRONG_CAPACITY:
            description = "Wrong file system capacity.";
            break;

        case VAERROR_FS_HAS_CYCLES:
            description = "Cyclic reference chain detected.";
            break;

        case VAERROR_FS_CORRUPTED:
            description = "Corrupted file system.";
            break;

        case VAERROR_FS_DIR_NOT_EMPTY:
            description = "Directory is not empty.";
            break;

        case VAERROR_FS_CANNOT_CREATE_DIR:
            description = "Unable to create directory.";
            break;

        case VAERROR_FS_CANNOT_CREATE_FILE:
            description = "Unable to create file.";
            break;

        default:
            description =
            "Error code " + std::to_string(data) +
            " (" + ErrorCodeEnum::key((ErrorCode)data) + ").";
            break;
    }
}

const char *
Error::what() const throw()
{
    return description.c_str();
}

}
