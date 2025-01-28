// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

enum class VAError : long
{
    OK,                   ///< No error
    UNKNOWN,              ///< Unclassified error condition
    
    // Emulator state
    LAUNCH,               ///< Launch error
    POWERED_OFF,          ///< The emulator is powered off
    POWERED_ON,           ///< The emulator is powered on
    DEBUG_OFF,
    RUNNING,              ///< The emulator is running
    
    // Configuration
    OPT_UNSUPPORTED,      ///< Unsupported configuration option
    OPT_INV_ARG,          ///< Invalid argument
    OPT_INV_ID,           ///< Invalid component id
    OPT_LOCKED,           ///< The option is temporarily locked
    
    // Property storage
    INVALID_KEY,          ///< Invalid property key
    SYNTAX,               ///< Syntax error
    
    // CPU
    CPU_UNSUPPORTED,      ///< Unsupported CPU model
    GUARD_NOT_FOUND,      ///< Guard is not set
    GUARD_ALREADY_SET,    ///< Guard is already set
    BP_NOT_FOUND,         ///< Breakpoint is not set (DEPRECATED)
    BP_ALREADY_SET,       ///< Breakpoint is already set (DEPRECATED)
    WP_NOT_FOUND,         ///< Watchpoint is not set (DEPRECATED)
    WP_ALREADY_SET,       ///< Watchpoint is already set (DEPRECATED)
    CP_NOT_FOUND,         ///< Catchpoint is not set (DEPRECATED)
    CP_ALREADY_SET,       ///< Catchpoint is already set (DEPRECATED)
    
    // Memory
    OUT_OF_MEMORY,        ///< Out of memory
    
    // General
    DIR_NOT_FOUND,        ///< Directory does not exist
    DIR_ACCESS_DENIED,    ///< File access denied
    DIR_CANT_CREATE,      ///< Unable to create a directory
    DIR_NOT_EMPTY,        ///< Directory is not empty
    FILE_NOT_FOUND,       ///< File not found error
    FILE_EXISTS,          ///< File already exists
    FILE_IS_DIRECTORY,    ///< The file is a directory
    FILE_ACCESS_DENIED,   ///< File access denied
    FILE_TYPE_MISMATCH,   ///< File type mismatch
    FILE_TYPE_UNSUPPORTED,///< Unsupported file type
    FILE_CANT_READ,       ///< Can't read from file
    FILE_CANT_WRITE,      ///< Can't write to file
    FILE_CANT_CREATE,     ///< Can't create file
    
    // Ram
    CHIP_RAM_MISSING,
    CHIP_RAM_LIMIT,
    AROS_RAM_LIMIT,
    
    // Rom
    ROM_MISSING,
    AROS_NO_EXTROM,
    
    // Drives
    WT_BLOCKED,
    WT,
    
    // Floppy disks
    DISK_MISSING,
    DISK_INCOMPATIBLE,
    DISK_INVALID_DIAMETER,
    DISK_INVALID_DENSITY,
    DISK_INVALID_LAYOUT,
    DISK_WRONG_SECTOR_COUNT,
    DISK_INVALID_SECTOR_NUMBER,
    
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
    HDR_UNSUPPORTED,
    
    // Hard drive controller
    HDC_INIT,
    
    // Snapshots
    SNAP_TOO_OLD,         ///< Snapshot was created with an older version
    SNAP_TOO_NEW,         ///< Snapshot was created with a later version
    SNAP_IS_BETA,         ///< Snapshot was created with a beta release
    SNAP_CORRUPTED,       ///< Snapshot data is corrupted
    
    // Media files
    DMS_CANT_CREATE,
    EXT_FACTOR5,
    EXT_INCOMPATIBLE,
    EXT_CORRUPTED,
    
    // Encrypted Roms
    MISSING_ROM_KEY,
    INVALID_ROM_KEY,
    
    // Recorder
    REC_LAUNCH,
    
    // Debugger
    REG_READ_ONLY,
    REG_WRITE_ONLY,
    REG_UNUSED,
    ADDR_UNALIGNED,
    
    // OS Debugger
    OSDB,
    HUNK_BAD_COOKIE,
    HUNK_BAD_HEADER,
    HUNK_NO_SECTIONS,
    HUNK_UNSUPPORTED,
    HUNK_CORRUPTED,
    
    // Remote servers
    SOCK_CANT_CREATE,
    SOCK_CANT_CONNECT,
    SOCK_CANT_BIND,
    SOCK_CANT_LISTEN,
    SOCK_CANT_ACCEPT,
    SOCK_CANT_RECEIVE,
    SOCK_CANT_SEND,
    SOCK_DISCONNECTED,
    SERVER_PORT_IN_USE,
    SERVER_ON,
    SERVER_OFF,
    SERVER_RUNNING,
    SERVER_NOT_RUNNING,
    SERVER_NO_CLIENT,
    
    // GDB server
    GDB_NO_ACK,
    GDB_INVALID_FORMAT,
    GDB_INVALID_CHECKSUM,
    GDB_UNRECOGNIZED_CMD,
    GDB_UNSUPPORTED_CMD,
    
    // File system
    FS_UNKNOWN,
    FS_UNSUPPORTED,
    FS_UNFORMATTED,
    FS_WRONG_BSIZE,
    FS_WRONG_CAPACITY,
    FS_WRONG_DOS_TYPE,
    FS_HAS_CYCLES,
    FS_CORRUPTED,
    
    // File system (import errors)
    FS_OUT_OF_SPACE,
    
    // File system (export errors)
    FS_DIR_NOT_EMPTY,
    FS_CANNOT_CREATE_DIR,
    FS_CANNOT_CREATE_FILE,
    
    // File system (block errors)
    FS_INVALID_BLOCK_TYPE,
    FS_EXPECTED_VALUE,
    FS_EXPECTED_SMALLER_VALUE,
    FS_EXPECTED_DOS_REVISION,
    FS_EXPECTED_NO_REF,
    FS_EXPECTED_REF,
    FS_EXPECTED_SELFREF,
    FS_PTR_TO_UNKNOWN_BLOCK,
    FS_PTR_TO_EMPTY_BLOCK,
    FS_PTR_TO_BOOT_BLOCK,
    FS_PTR_TO_ROOT_BLOCK,
    FS_PTR_TO_BITMAP_BLOCK,
    FS_PTR_TO_BITMAP_EXT_BLOCK,
    FS_PTR_TO_USERDIR_BLOCK,
    FS_PTR_TO_FILEHEADER_BLOCK,
    FS_PTR_TO_FILELIST_BLOCK,
    FS_PTR_TO_DATA_BLOCK,
    FS_EXPECTED_DATABLOCK_NR,
    FS_INVALID_HASHTABLE_SIZE
};

struct VAErrorEnum : util::Reflection<VAErrorEnum, VAError>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(VAError::FS_INVALID_HASHTABLE_SIZE);
    
    static const char *_key(VAError value)
    {
        switch (value) {
                
            case VAError::OK:                          return "OK";
            case VAError::UNKNOWN:                     return "UNKNOWN";
                
            case VAError::LAUNCH:                      return "LAUNCH";
            case VAError::POWERED_OFF:                 return "POWERED_OFF";
            case VAError::POWERED_ON:                  return "POWERED_ON";
            case VAError::DEBUG_OFF:                   return "DEBUG_OFF";
            case VAError::RUNNING:                     return "RUNNING";
                
            case VAError::OPT_UNSUPPORTED:             return "OPT_UNSUPPORTED";
            case VAError::OPT_INV_ARG:                 return "OPT_INV_ARG";
            case VAError::OPT_INV_ID:                  return "OPT_INV_ID";
            case VAError::OPT_LOCKED:                  return "OPT_LOCKED";
                
            case VAError::INVALID_KEY:                 return "INVALID_KEY";
            case VAError::SYNTAX:                      return "SYNTAX";
                
            case VAError::CPU_UNSUPPORTED:             return "CPU_UNSUPPORTED";
            case VAError::GUARD_NOT_FOUND:             return "GUARD_NOT_FOUND";
            case VAError::GUARD_ALREADY_SET:           return "GUARD_ALREADY_SET";
            case VAError::BP_NOT_FOUND:                return "BP_NOT_FOUND";
            case VAError::BP_ALREADY_SET:              return "BP_ALREADY_SET";
            case VAError::WP_NOT_FOUND:                return "WP_NOT_FOUND";
            case VAError::WP_ALREADY_SET:              return "WP_ALREADY_SET";
            case VAError::CP_NOT_FOUND:                return "CP_NOT_FOUND";
            case VAError::CP_ALREADY_SET:              return "CP_ALREADY_SET";
                
            case VAError::OUT_OF_MEMORY:               return "OUT_OF_MEMORY";
                
            case VAError::DIR_NOT_FOUND:               return "DIR_NOT_FOUND";
            case VAError::DIR_ACCESS_DENIED:           return "DIR_ACCESS_DENIED";
            case VAError::DIR_CANT_CREATE:             return "DIR_CANT_CREATE";
            case VAError::DIR_NOT_EMPTY:               return "DIR_NOT_EMPTY";
            case VAError::FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case VAError::FILE_EXISTS:                 return "FILE_EXISTS";
            case VAError::FILE_IS_DIRECTORY:           return "FILE_IS_DIRECtORY";
            case VAError::FILE_ACCESS_DENIED:          return "FILE_ACCESS_DENIED";
            case VAError::FILE_TYPE_MISMATCH:          return "FILE_TYPE_MISMATCH";
            case VAError::FILE_TYPE_UNSUPPORTED:       return "FILE_TYPE_UNSUPPORTED";
            case VAError::FILE_CANT_READ:              return "FILE_CANT_READ";
            case VAError::FILE_CANT_WRITE:             return "FILE_CANT_WRITE";
            case VAError::FILE_CANT_CREATE:            return "FILE_CANT_CREATE";
                
            case VAError::CHIP_RAM_MISSING:            return "CHIP_RAM_MISSING";
            case VAError::CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case VAError::AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";
                
            case VAError::ROM_MISSING:                 return "ROM_MISSING";
            case VAError::AROS_NO_EXTROM:              return "AROS_NO_EXTROM";
                
            case VAError::WT_BLOCKED:                  return "WT_BLOCKED";
            case VAError::WT:                          return "WT";
                
            case VAError::DISK_MISSING:                return "DISK_MISSING";
            case VAError::DISK_INCOMPATIBLE:           return "DISK_INCOMPATIBLE";
            case VAError::DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case VAError::DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
            case VAError::DISK_INVALID_LAYOUT:         return "DISK_INVALID_LAYOUT";
            case VAError::DISK_WRONG_SECTOR_COUNT:     return "DISK_WRONG_SECTOR_COUNT";
            case VAError::DISK_INVALID_SECTOR_NUMBER:  return "DISK_INVALID_SECTOR_NUMBER";
                
            case VAError::HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case VAError::HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case VAError::HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case VAError::HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case VAError::HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case VAError::HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case VAError::HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case VAError::HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case VAError::HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case VAError::HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case VAError::HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case VAError::HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";
                
            case VAError::HDC_INIT:                    return "HDC_INIT";
                
            case VAError::SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case VAError::SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case VAError::SNAP_IS_BETA:                return "SNAP_IS_BETA";
            case VAError::SNAP_CORRUPTED:              return "SNAP_CORRUPTED";
                
            case VAError::DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case VAError::EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case VAError::EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case VAError::EXT_CORRUPTED:               return "EXT_CORRUPTED";
                
            case VAError::MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case VAError::INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
                
            case VAError::REC_LAUNCH:                  return "REC_LAUNCH";
                
            case VAError::REG_READ_ONLY:               return "REG_READ_ONLY";
            case VAError::REG_WRITE_ONLY:              return "REG_WRITE_ONLY";
            case VAError::REG_UNUSED:                  return "REG_UNUSED";
            case VAError::ADDR_UNALIGNED:              return "ADDR_UNALIGNED";
                
            case VAError::OSDB:                        return "OSDB";
            case VAError::HUNK_BAD_COOKIE:             return "HUNK_BAD_COOKIE";
            case VAError::HUNK_BAD_HEADER:             return "HUNK_BAD_HEADER";
            case VAError::HUNK_NO_SECTIONS:            return "HUNK_NO_SECTIONS";
            case VAError::HUNK_UNSUPPORTED:            return "HUNK_UNSUPPORTED";
            case VAError::HUNK_CORRUPTED:              return "HUNK_CORRUPTED";
                
            case VAError::SOCK_CANT_CREATE:            return "SOCK_CANT_CREATE";
            case VAError::SOCK_CANT_CONNECT:           return "SOCK_CANT_CONNECT";
            case VAError::SOCK_CANT_BIND:              return "SOCK_CANT_BIND";
            case VAError::SOCK_CANT_LISTEN:            return "SOCK_CANT_LISTEN";
            case VAError::SOCK_CANT_ACCEPT:            return "SOCK_CANT_ACCEPT";
            case VAError::SOCK_CANT_RECEIVE:           return "SOCK_CANT_RECEIVE";
            case VAError::SOCK_CANT_SEND:              return "SOCK_CANT_SEND";
            case VAError::SOCK_DISCONNECTED:           return "SOCK_DISCONNECTED";
            case VAError::SERVER_PORT_IN_USE:          return "SERVER_PORT_IN_USE";
            case VAError::SERVER_ON:                   return "SERVER_ON";
            case VAError::SERVER_OFF:                  return "SERVER_OFF";
            case VAError::SERVER_RUNNING:              return "SERVER_RUNNING";
            case VAError::SERVER_NOT_RUNNING:          return "SERVER_NOT_RUNNING";
            case VAError::SERVER_NO_CLIENT:            return "SERVER_NO_CLIENT";
                
            case VAError::GDB_NO_ACK:                  return "GDB_NO_ACK";
            case VAError::GDB_INVALID_FORMAT:          return "GDB_INVALID_FORMAT";
            case VAError::GDB_INVALID_CHECKSUM:        return "GDB_INVALID_CHECKSUM";
            case VAError::GDB_UNRECOGNIZED_CMD:        return "GDB_UNRECOGNIZED_CMD";
            case VAError::GDB_UNSUPPORTED_CMD:         return "GDB_UNSUPPORTED_CMD";
                
            case VAError::FS_UNKNOWN:                  return "FS_UNKNOWN";
            case VAError::FS_UNSUPPORTED:              return "FS_UNSUPPORTED";
            case VAError::FS_UNFORMATTED:              return "FS_UNFORMATTED";
            case VAError::FS_WRONG_BSIZE:              return "FS_WRONG_BSIZE";
            case VAError::FS_WRONG_CAPACITY:           return "FS_WRONG_CAPACITY";
            case VAError::FS_WRONG_DOS_TYPE:           return "FS_WRONG_DOS_TYPE";
            case VAError::FS_HAS_CYCLES:               return "FS_HAS_CYCLES";
            case VAError::FS_CORRUPTED:                return "FS_CORRUPTED";
                
            case VAError::FS_OUT_OF_SPACE:             return "FS_OUT_OF_SPACE";
                
            case VAError::FS_DIR_NOT_EMPTY:            return "FS_DIR_NOT_EMPTY";
            case VAError::FS_CANNOT_CREATE_DIR:        return "FS_CANNOT_CREATE_DIR";
            case VAError::FS_CANNOT_CREATE_FILE:       return "FS_CANNOT_CREATE_FILE";
                
            case VAError::FS_INVALID_BLOCK_TYPE:       return "FS_INVALID_BLOCK_TYPE";
            case VAError::FS_EXPECTED_VALUE:           return "FS_EXPECTED_VALUE";
            case VAError::FS_EXPECTED_SMALLER_VALUE:   return "FS_EXPECTED_SMALLER_VALUE";
            case VAError::FS_EXPECTED_DOS_REVISION:    return "FS_EXPECTED_DOS_REVISION";
            case VAError::FS_EXPECTED_NO_REF:          return "FS_EXPECTED_NO_REF";
            case VAError::FS_EXPECTED_REF:             return "FS_EXPECTED_REF";
            case VAError::FS_EXPECTED_SELFREF:         return "FS_EXPECTED_SELFREF";
            case VAError::FS_PTR_TO_UNKNOWN_BLOCK:     return "FS_PTR_TO_UNKNOWN_BLOCK";
            case VAError::FS_PTR_TO_EMPTY_BLOCK:       return "FS_PTR_TO_EMPTY_BLOCK";
            case VAError::FS_PTR_TO_BOOT_BLOCK:        return "FS_PTR_TO_BOOT_BLOCK";
            case VAError::FS_PTR_TO_ROOT_BLOCK:        return "FS_PTR_TO_ROOT_BLOCK";
            case VAError::FS_PTR_TO_BITMAP_BLOCK:      return "FS_PTR_TO_BITMAP_BLOCK";
            case VAError::FS_PTR_TO_BITMAP_EXT_BLOCK:  return "FS_PTR_TO_BITMAP_EXT_BLOCK";
            case VAError::FS_PTR_TO_USERDIR_BLOCK:     return "FS_PTR_TO_USERDIR_BLOCK";
            case VAError::FS_PTR_TO_FILEHEADER_BLOCK:  return "FS_PTR_TO_FILEHEADER_BLOCK";
            case VAError::FS_PTR_TO_FILELIST_BLOCK:    return "FS_PTR_TO_FILELIST_BLOCK";
            case VAError::FS_PTR_TO_DATA_BLOCK:        return "FS_PTR_TO_DATA_BLOCK";
            case VAError::FS_EXPECTED_DATABLOCK_NR:    return "FS_EXPECTED_DATABLOCK_NR";
            case VAError::FS_INVALID_HASHTABLE_SIZE:   return "FS_INVALID_HASHTABLE_SIZE";
        }
        return "???";
    }
    
    static const char *help(VAError value)
    {
        return "";
    }
};

}
