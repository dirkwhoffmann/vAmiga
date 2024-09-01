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

enum_long(VAERROR_CODE)
{
    VAERROR_OK,                   ///< No error
    VAERROR_UNKNOWN,              ///< Unclassified error condition

    // Emulator state
    VAERROR_LAUNCH,               ///< Launch error
    VAERROR_POWERED_OFF,          ///< The emulator is powered off
    VAERROR_POWERED_ON,           ///< The emulator is powered on
    VAERROR_DEBUG_OFF,
    VAERROR_RUNNING,              ///< The emulator is running

    // Configuration
    VAERROR_OPT_UNSUPPORTED,      ///< Unsupported configuration option
    VAERROR_OPT_INV_ARG,          ///< Invalid argument
    VAERROR_OPT_INV_ID,           ///< Invalid component id
    VAERROR_OPT_LOCKED,           ///< The option is temporarily locked

    // Property storage
    VAERROR_INVALID_KEY,          ///< Invalid property key
    VAERROR_SYNTAX,               ///< Syntax error

    // CPU
    VAERROR_CPU_UNSUPPORTED,      ///< Unsupported CPU model
    VAERROR_GUARD_NOT_FOUND,      ///< Guard is not set
    VAERROR_GUARD_ALREADY_SET,    ///< Guard is already set
    VAERROR_BP_NOT_FOUND,         ///< Breakpoint is not set (DEPRECATED)
    VAERROR_BP_ALREADY_SET,       ///< Breakpoint is already set (DEPRECATED)
    VAERROR_WP_NOT_FOUND,         ///< Watchpoint is not set (DEPRECATED)
    VAERROR_WP_ALREADY_SET,       ///< Watchpoint is already set (DEPRECATED)
    VAERROR_CP_NOT_FOUND,         ///< Catchpoint is not set (DEPRECATED)
    VAERROR_CP_ALREADY_SET,       ///< Catchpoint is already set (DEPRECATED)
    // VAERROR_CP_CANT_CATCH,

    // Memory
    VAERROR_OUT_OF_MEMORY,        ///< Out of memory

    // General
    VAERROR_DIR_NOT_FOUND,        ///< Directory does not exist
    VAERROR_DIR_ACCESS_DENIED,    ///< File access denied
    VAERROR_DIR_CANT_CREATE,      ///< Unable to create a directory
    VAERROR_DIR_NOT_EMPTY,        ///< Directory is not empty
    VAERROR_FILE_NOT_FOUND,       ///< File not found error
    VAERROR_FILE_EXISTS,          ///< File already exists
    VAERROR_FILE_IS_DIRECTORY,    ///< The file is a directory
    VAERROR_FILE_ACCESS_DENIED,   ///< File access denied
    VAERROR_FILE_TYPE_MISMATCH,   ///< File type mismatch
    VAERROR_FILE_TYPE_UNSUPPORTED,///< Unsupported file type
    VAERROR_FILE_CANT_READ,       ///< Can't read from file
    VAERROR_FILE_CANT_WRITE,      ///< Can't write to file
    VAERROR_FILE_CANT_CREATE,     ///< Can't create file

    // Ram
    VAERROR_CHIP_RAM_MISSING,
    VAERROR_CHIP_RAM_LIMIT,
    VAERROR_AROS_RAM_LIMIT,

    // Rom
    VAERROR_ROM_MISSING,
    VAERROR_AROS_NO_EXTROM,
    
    // Drives
    VAERROR_WT_BLOCKED,
    VAERROR_WT,
    
    // Floppy disks
    VAERROR_DISK_MISSING,
    VAERROR_DISK_INCOMPATIBLE,
    VAERROR_DISK_INVALID_DIAMETER,
    VAERROR_DISK_INVALID_DENSITY,
    VAERROR_DISK_INVALID_LAYOUT,
    VAERROR_DISK_WRONG_SECTOR_COUNT,
    VAERROR_DISK_INVALID_SECTOR_NUMBER,
    
    // Hard disks
    VAERROR_HDR_TOO_LARGE,
    VAERROR_HDR_UNSUPPORTED_CYL_COUNT,
    VAERROR_HDR_UNSUPPORTED_HEAD_COUNT,
    VAERROR_HDR_UNSUPPORTED_SEC_COUNT,
    VAERROR_HDR_UNSUPPORTED_BSIZE,
    VAERROR_HDR_UNKNOWN_GEOMETRY,
    VAERROR_HDR_UNMATCHED_GEOMETRY,
    VAERROR_HDR_UNPARTITIONED,
    VAERROR_HDR_CORRUPTED_PTABLE,
    VAERROR_HDR_CORRUPTED_FSH,
    VAERROR_HDR_CORRUPTED_LSEG,
    VAERROR_HDR_UNSUPPORTED,

    // Hard drive controller
    VAERROR_HDC_INIT,

    // Snapshots
    VAERROR_SNAP_TOO_OLD,         ///< Snapshot was created with an older version
    VAERROR_SNAP_TOO_NEW,         ///< Snapshot was created with a later version
    VAERROR_SNAP_IS_BETA,         ///< Snapshot was created with a beta release
    VAERROR_SNAP_CORRUPTED,       ///< Snapshot data is corrupted

    // Media files
    VAERROR_DMS_CANT_CREATE,
    VAERROR_EXT_FACTOR5,
    VAERROR_EXT_INCOMPATIBLE,
    VAERROR_EXT_CORRUPTED,
    
    // Encrypted Roms
    VAERROR_MISSING_ROM_KEY,
    VAERROR_INVALID_ROM_KEY,
    
    // Recorder
    VAERROR_REC_LAUNCH,

    // Debugger
    VAERROR_REG_READ_ONLY,
    VAERROR_REG_WRITE_ONLY,
    VAERROR_REG_UNUSED,
    VAERROR_ADDR_UNALIGNED,

    // OS Debugger
    VAERROR_OSDB,
    VAERROR_HUNK_BAD_COOKIE,
    VAERROR_HUNK_BAD_HEADER,
    VAERROR_HUNK_NO_SECTIONS,
    VAERROR_HUNK_UNSUPPORTED,
    VAERROR_HUNK_CORRUPTED,

    // Remote servers
    VAERROR_SOCK_CANT_CREATE,
    VAERROR_SOCK_CANT_CONNECT,
    VAERROR_SOCK_CANT_BIND,
    VAERROR_SOCK_CANT_LISTEN,
    VAERROR_SOCK_CANT_ACCEPT,
    VAERROR_SOCK_CANT_RECEIVE,
    VAERROR_SOCK_CANT_SEND,
    VAERROR_SOCK_DISCONNECTED,
    VAERROR_SERVER_PORT_IN_USE,
    VAERROR_SERVER_ON,
    VAERROR_SERVER_OFF,
    VAERROR_SERVER_RUNNING,
    VAERROR_SERVER_NOT_RUNNING,
    VAERROR_SERVER_NO_CLIENT,

    // GDB server
    VAERROR_GDB_NO_ACK,
    VAERROR_GDB_INVALID_FORMAT,
    VAERROR_GDB_INVALID_CHECKSUM,
    VAERROR_GDB_UNRECOGNIZED_CMD,
    VAERROR_GDB_UNSUPPORTED_CMD,

    // File system
    VAERROR_FS_UNKNOWN,
    VAERROR_FS_UNSUPPORTED,
    VAERROR_FS_UNFORMATTED,
    VAERROR_FS_WRONG_BSIZE,
    VAERROR_FS_WRONG_CAPACITY,
    VAERROR_FS_WRONG_DOS_TYPE,
    VAERROR_FS_HAS_CYCLES,
    VAERROR_FS_CORRUPTED,

    // File system (import errors)
    VAERROR_FS_OUT_OF_SPACE,
    
    // File system (export errors)
    VAERROR_FS_DIR_NOT_EMPTY,
    VAERROR_FS_CANNOT_CREATE_DIR,
    VAERROR_FS_CANNOT_CREATE_FILE,

    // File system (block errors)
    VAERROR_FS_INVALID_BLOCK_TYPE,
    VAERROR_FS_EXPECTED_VALUE,
    VAERROR_FS_EXPECTED_SMALLER_VALUE,
    VAERROR_FS_EXPECTED_DOS_REVISION,
    VAERROR_FS_EXPECTED_NO_REF,
    VAERROR_FS_EXPECTED_REF,
    VAERROR_FS_EXPECTED_SELFREF,
    VAERROR_FS_PTR_TO_UNKNOWN_BLOCK,
    VAERROR_FS_PTR_TO_EMPTY_BLOCK,
    VAERROR_FS_PTR_TO_BOOT_BLOCK,
    VAERROR_FS_PTR_TO_ROOT_BLOCK,
    VAERROR_FS_PTR_TO_BITMAP_BLOCK,
    VAERROR_FS_PTR_TO_BITMAP_EXT_BLOCK,
    VAERROR_FS_PTR_TO_USERDIR_BLOCK,
    VAERROR_FS_PTR_TO_FILEHEADER_BLOCK,
    VAERROR_FS_PTR_TO_FILELIST_BLOCK,
    VAERROR_FS_PTR_TO_DATA_BLOCK,
    VAERROR_FS_EXPECTED_DATABLOCK_NR,
    VAERROR_FS_INVALID_HASHTABLE_SIZE
};
typedef VAERROR_CODE ErrorCode;

#ifdef __cplusplus
struct ErrorCodeEnum : vamiga::util::Reflection<ErrorCodeEnum, ErrorCode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = VAERROR_FS_INVALID_HASHTABLE_SIZE;
    
    static const char *prefix() { return "ERROR"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case VAERROR_OK:                          return "OK";
            case VAERROR_UNKNOWN:                     return "UNKNOWN";
                
            case VAERROR_LAUNCH:                      return "LAUNCH";
            case VAERROR_POWERED_OFF:                 return "POWERED_OFF";
            case VAERROR_POWERED_ON:                  return "POWERED_ON";
            case VAERROR_DEBUG_OFF:                   return "DEBUG_OFF";
            case VAERROR_RUNNING:                     return "RUNNING";

            case VAERROR_OPT_UNSUPPORTED:             return "OPT_UNSUPPORTED";
            case VAERROR_OPT_INV_ARG:                 return "OPT_INV_ARG";
            case VAERROR_OPT_INV_ID:                  return "OPT_INV_ID";
            case VAERROR_OPT_LOCKED:                  return "OPT_LOCKED";
                
            case VAERROR_INVALID_KEY:                 return "INVALID_KEY";
            case VAERROR_SYNTAX:                      return "SYNTAX";
                
            case VAERROR_CPU_UNSUPPORTED:             return "CPU_UNSUPPORTED";
            case VAERROR_GUARD_NOT_FOUND:             return "GUARD_NOT_FOUND";
            case VAERROR_GUARD_ALREADY_SET:           return "GUARD_ALREADY_SET";
            case VAERROR_BP_NOT_FOUND:                return "BP_NOT_FOUND";
            case VAERROR_BP_ALREADY_SET:              return "BP_ALREADY_SET";
            case VAERROR_WP_NOT_FOUND:                return "WP_NOT_FOUND";
            case VAERROR_WP_ALREADY_SET:              return "WP_ALREADY_SET";
            case VAERROR_CP_NOT_FOUND:                return "CP_NOT_FOUND";
            case VAERROR_CP_ALREADY_SET:              return "CP_ALREADY_SET";
            // case VAERROR_CP_CANT_CATCH:               return "CP_CANT_CATCH";

            case VAERROR_OUT_OF_MEMORY:               return "OUT_OF_MEMORY";

            case VAERROR_DIR_NOT_FOUND:               return "DIR_NOT_FOUND";
            case VAERROR_DIR_ACCESS_DENIED:           return "DIR_ACCESS_DENIED";
            case VAERROR_DIR_CANT_CREATE:             return "DIR_CANT_CREATE";
            case VAERROR_DIR_NOT_EMPTY:               return "DIR_NOT_EMPTY";
            case VAERROR_FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case VAERROR_FILE_EXISTS:                 return "FILE_EXISTS";
            case VAERROR_FILE_IS_DIRECTORY:           return "FILE_IS_DIRECtORY";
            case VAERROR_FILE_ACCESS_DENIED:          return "FILE_ACCESS_DENIED";
            case VAERROR_FILE_TYPE_MISMATCH:          return "FILE_TYPE_MISMATCH";
            case VAERROR_FILE_TYPE_UNSUPPORTED:       return "FILE_TYPE_UNSUPPORTED";
            case VAERROR_FILE_CANT_READ:              return "FILE_CANT_READ";
            case VAERROR_FILE_CANT_WRITE:             return "FILE_CANT_WRITE";
            case VAERROR_FILE_CANT_CREATE:            return "FILE_CANT_CREATE";
                
            case VAERROR_CHIP_RAM_MISSING:            return "CHIP_RAM_MISSING";
            case VAERROR_CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case VAERROR_AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";

            case VAERROR_ROM_MISSING:                 return "ROM_MISSING";
            case VAERROR_AROS_NO_EXTROM:              return "AROS_NO_EXTROM";

            case VAERROR_WT_BLOCKED:                  return "WT_BLOCKED";
            case VAERROR_WT:                          return "WT";

            case VAERROR_DISK_MISSING:                return "DISK_MISSING";
            case VAERROR_DISK_INCOMPATIBLE:           return "DISK_INCOMPATIBLE";
            case VAERROR_DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case VAERROR_DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
            case VAERROR_DISK_INVALID_LAYOUT:         return "DISK_INVALID_LAYOUT";
            case VAERROR_DISK_WRONG_SECTOR_COUNT:     return "DISK_WRONG_SECTOR_COUNT";
            case VAERROR_DISK_INVALID_SECTOR_NUMBER:  return "DISK_INVALID_SECTOR_NUMBER";
                
            case VAERROR_HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case VAERROR_HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case VAERROR_HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case VAERROR_HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case VAERROR_HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case VAERROR_HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case VAERROR_HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case VAERROR_HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case VAERROR_HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case VAERROR_HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case VAERROR_HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case VAERROR_HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";
                
            case VAERROR_HDC_INIT:                    return "HDC_INIT";

            case VAERROR_SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case VAERROR_SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case VAERROR_SNAP_IS_BETA:                return "SNAP_IS_BETA";

            case VAERROR_DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case VAERROR_EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case VAERROR_EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case VAERROR_EXT_CORRUPTED:               return "EXT_CORRUPTED";
                
            case VAERROR_MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case VAERROR_INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
                
            case VAERROR_REC_LAUNCH:                  return "REC_LAUNCH";

            case VAERROR_REG_READ_ONLY:               return "REG_READ_ONLY";
            case VAERROR_REG_WRITE_ONLY:              return "REG_WRITE_ONLY";
            case VAERROR_REG_UNUSED:                  return "REG_UNUSED";
            case VAERROR_ADDR_UNALIGNED:              return "ADDR_UNALIGNED";

            case VAERROR_OSDB:                        return "OSDB";
            case VAERROR_HUNK_BAD_COOKIE:             return "HUNK_BAD_COOKIE";
            case VAERROR_HUNK_BAD_HEADER:             return "HUNK_BAD_HEADER";
            case VAERROR_HUNK_NO_SECTIONS:            return "HUNK_NO_SECTIONS";
            case VAERROR_HUNK_UNSUPPORTED:            return "HUNK_UNSUPPORTED";
            case VAERROR_HUNK_CORRUPTED:              return "HUNK_CORRUPTED";

            case VAERROR_SOCK_CANT_CREATE:            return "SOCK_CANT_CREATE";
            case VAERROR_SOCK_CANT_CONNECT:           return "SOCK_CANT_CONNECT";
            case VAERROR_SOCK_CANT_BIND:              return "SOCK_CANT_BIND";
            case VAERROR_SOCK_CANT_LISTEN:            return "SOCK_CANT_LISTEN";
            case VAERROR_SOCK_CANT_ACCEPT:            return "SOCK_CANT_ACCEPT";
            case VAERROR_SOCK_CANT_RECEIVE:           return "SOCK_CANT_RECEIVE";
            case VAERROR_SOCK_CANT_SEND:              return "SOCK_CANT_SEND";
            case VAERROR_SOCK_DISCONNECTED:           return "SOCK_DISCONNECTED";
            case VAERROR_SERVER_PORT_IN_USE:          return "SERVER_PORT_IN_USE";
            case VAERROR_SERVER_ON:                   return "SERVER_ON";
            case VAERROR_SERVER_OFF:                  return "SERVER_OFF";
            case VAERROR_SERVER_RUNNING:              return "SERVER_RUNNING";
            case VAERROR_SERVER_NOT_RUNNING:          return "SERVER_NOT_RUNNING";
            case VAERROR_SERVER_NO_CLIENT:            return "SERVER_NO_CLIENT";
                
            case VAERROR_GDB_NO_ACK:                  return "GDB_NO_ACK";
            case VAERROR_GDB_INVALID_FORMAT:          return "GDB_INVALID_FORMAT";
            case VAERROR_GDB_INVALID_CHECKSUM:        return "GDB_INVALID_CHECKSUM";
            case VAERROR_GDB_UNRECOGNIZED_CMD:        return "GDB_UNRECOGNIZED_CMD";
            case VAERROR_GDB_UNSUPPORTED_CMD:         return "GDB_UNSUPPORTED_CMD";

            case VAERROR_FS_UNKNOWN:                  return "FS_UNKNOWN";
            case VAERROR_FS_UNSUPPORTED:              return "FS_UNSUPPORTED";
            case VAERROR_FS_UNFORMATTED:              return "FS_UNFORMATTED";
            case VAERROR_FS_WRONG_BSIZE:              return "FS_WRONG_BSIZE";
            case VAERROR_FS_WRONG_CAPACITY:           return "FS_WRONG_CAPACITY";
            case VAERROR_FS_WRONG_DOS_TYPE:           return "FS_WRONG_DOS_TYPE";
            case VAERROR_FS_HAS_CYCLES:               return "FS_HAS_CYCLES";
            case VAERROR_FS_CORRUPTED:                return "FS_CORRUPTED";

            case VAERROR_FS_OUT_OF_SPACE:             return "FS_OUT_OF_SPACE";
                
            case VAERROR_FS_DIR_NOT_EMPTY:            return "FS_DIR_NOT_EMPTY";
            case VAERROR_FS_CANNOT_CREATE_DIR:        return "FS_CANNOT_CREATE_DIR";
            case VAERROR_FS_CANNOT_CREATE_FILE:       return "FS_CANNOT_CREATE_FILE";

            case VAERROR_FS_INVALID_BLOCK_TYPE:       return "FS_INVALID_BLOCK_TYPE";
            case VAERROR_FS_EXPECTED_VALUE:           return "FS_EXPECTED_VALUE";
            case VAERROR_FS_EXPECTED_SMALLER_VALUE:   return "FS_EXPECTED_SMALLER_VALUE";
            case VAERROR_FS_EXPECTED_DOS_REVISION:    return "FS_EXPECTED_DOS_REVISION";
            case VAERROR_FS_EXPECTED_NO_REF:          return "FS_EXPECTED_NO_REF";
            case VAERROR_FS_EXPECTED_REF:             return "FS_EXPECTED_REF";
            case VAERROR_FS_EXPECTED_SELFREF:         return "FS_EXPECTED_SELFREF";
            case VAERROR_FS_PTR_TO_UNKNOWN_BLOCK:     return "FS_PTR_TO_UNKNOWN_BLOCK";
            case VAERROR_FS_PTR_TO_EMPTY_BLOCK:       return "FS_PTR_TO_EMPTY_BLOCK";
            case VAERROR_FS_PTR_TO_BOOT_BLOCK:        return "FS_PTR_TO_BOOT_BLOCK";
            case VAERROR_FS_PTR_TO_ROOT_BLOCK:        return "FS_PTR_TO_ROOT_BLOCK";
            case VAERROR_FS_PTR_TO_BITMAP_BLOCK:      return "FS_PTR_TO_BITMAP_BLOCK";
            case VAERROR_FS_PTR_TO_BITMAP_EXT_BLOCK:  return "FS_PTR_TO_BITMAP_EXT_BLOCK";
            case VAERROR_FS_PTR_TO_USERDIR_BLOCK:     return "FS_PTR_TO_USERDIR_BLOCK";
            case VAERROR_FS_PTR_TO_FILEHEADER_BLOCK:  return "FS_PTR_TO_FILEHEADER_BLOCK";
            case VAERROR_FS_PTR_TO_FILELIST_BLOCK:    return "FS_PTR_TO_FILELIST_BLOCK";
            case VAERROR_FS_PTR_TO_DATA_BLOCK:        return "FS_PTR_TO_DATA_BLOCK";
            case VAERROR_FS_EXPECTED_DATABLOCK_NR:    return "FS_EXPECTED_DATABLOCK_NR";
            case VAERROR_FS_INVALID_HASHTABLE_SIZE:   return "FS_INVALID_HASHTABLE_SIZE";
        }
        return "???";
    }
};
#endif
