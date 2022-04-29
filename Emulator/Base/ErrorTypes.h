// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

enum_long(ERROR_CODE)
{
    ERROR_OK,
    ERROR_UNKNOWN,
    
    // Emulator state
    ERROR_POWERED_OFF,
    ERROR_POWERED_ON,
    ERROR_DEBUG_OFF,
    ERROR_RUNNING,

    // Configuration
    ERROR_OPT_UNSUPPORTED,
    ERROR_OPT_INVARG,
    ERROR_OPT_LOCKED,               // DEPRECATED: Replace by ERROR_POWERED_ON

    // Property storage
    ERROR_INVALID_KEY,
    ERROR_SYNTAX,

    // CPU
    ERROR_BP_NOT_FOUND,
    ERROR_BP_ALREADY_SET,
    ERROR_WP_NOT_FOUND,
    ERROR_WP_ALREADY_SET,
    ERROR_CP_NOT_FOUND,
    ERROR_CP_ALREADY_SET,
    ERROR_CP_CANT_CATCH,

    // Memory
    ERROR_OUT_OF_MEMORY,

    // General
    ERROR_DIR_NOT_FOUND,
    ERROR_DIR_ACCESS_DENIED,
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_EXISTS,
    ERROR_FILE_IS_DIRECTORY,
    ERROR_FILE_ACCESS_DENIED,
    ERROR_FILE_TYPE_MISMATCH,
    ERROR_FILE_TYPE_UNSUPPORTED,
    ERROR_FILE_CANT_READ,
    ERROR_FILE_CANT_WRITE,
    ERROR_FILE_CANT_CREATE,

    // Ram
    ERROR_CHIP_RAM_MISSING,
    ERROR_CHIP_RAM_LIMIT,
    ERROR_AROS_RAM_LIMIT,

    // Rom
    ERROR_ROM_MISSING,
    ERROR_AROS_NO_EXTROM,
    
    // Drives
    ERROR_WT_BLOCKED,
    ERROR_WT,
    
    // Floppy disks
    ERROR_DISK_MISSING,
    ERROR_DISK_INCOMPATIBLE,
    ERROR_DISK_INVALID_DIAMETER,
    ERROR_DISK_INVALID_DENSITY,
    ERROR_DISK_INVALID_LAYOUT,
    ERROR_DISK_WRONG_SECTOR_COUNT,
    ERROR_DISK_INVALID_SECTOR_NUMBER,
    
    // Hard disks
    ERROR_HDR_TOO_LARGE,
    ERROR_HDR_UNSUPPORTED_CYL_COUNT,
    ERROR_HDR_UNSUPPORTED_HEAD_COUNT,
    ERROR_HDR_UNSUPPORTED_SEC_COUNT,
    ERROR_HDR_UNSUPPORTED_BSIZE,
    ERROR_HDR_UNMATCHED_GEOMETRY,
    ERROR_HDR_UNPARTITIONED,
    ERROR_HDR_CORRUPTED_PTABLE,
    ERROR_HDR_CORRUPTED_FSH,
    ERROR_HDR_CORRUPTED_LSEG,
    ERROR_HDR_UNSUPPORTED,

    // Hard drive controller
    ERROR_HDC_INIT,

    // Snapshots
    ERROR_SNAP_TOO_OLD,
    ERROR_SNAP_TOO_NEW,
    ERROR_SNAP_IS_BETA,
    ERROR_SNAP_CORRUPTED,
    
    // Media files
    ERROR_DMS_CANT_CREATE,
    ERROR_EXT_FACTOR5,
    ERROR_EXT_INCOMPATIBLE,
    ERROR_EXT_CORRUPTED,
    
    // Encrypted Roms
    ERROR_MISSING_ROM_KEY,
    ERROR_INVALID_ROM_KEY,
    
    // Recorder
    ERROR_REC_LAUNCH,

    // OS Debugger
    ERROR_OSDB,
    ERROR_HUNK_BAD_COOKIE,
    ERROR_HUNK_NO_SECTIONS,
    ERROR_HUNK_UNSUPPORTED,
    ERROR_HUNK_CORRUPTED,

    // Remote servers
    ERROR_SOCK_CANT_CREATE,
    ERROR_SOCK_CANT_CONNECT,
    ERROR_SOCK_CANT_BIND,
    ERROR_SOCK_CANT_LISTEN,
    ERROR_SOCK_CANT_ACCEPT,
    ERROR_SOCK_CANT_RECEIVE,
    ERROR_SOCK_CANT_SEND,
    ERROR_SOCK_DISCONNECTED,
    ERROR_SERVER_PORT_IN_USE,
    ERROR_SERVER_ON,
    ERROR_SERVER_OFF,
    ERROR_SERVER_RUNNING,
    ERROR_SERVER_NOT_RUNNING,
    ERROR_SERVER_NO_CLIENT,

    // GDB server
    ERROR_GDB_NO_ACK,
    ERROR_GDB_INVALID_FORMAT,
    ERROR_GDB_INVALID_CHECKSUM,
    ERROR_GDB_UNRECOGNIZED_CMD,
    ERROR_GDB_UNSUPPORTED_CMD,

    // File system
    ERROR_FS_UNKNOWN,
    ERROR_FS_UNSUPPORTED,
    ERROR_FS_UNFORMATTED,
    ERROR_FS_WRONG_BSIZE,
    ERROR_FS_WRONG_CAPACITY,
    ERROR_FS_WRONG_DOS_TYPE,
    ERROR_FS_HAS_CYCLES,
    ERROR_FS_CORRUPTED,

    // File system (import errors)
    ERROR_FS_OUT_OF_SPACE,
    
    // File system (export errors)
    ERROR_FS_DIR_NOT_EMPTY,
    ERROR_FS_CANNOT_CREATE_DIR,
    ERROR_FS_CANNOT_CREATE_FILE,

    // File system (block errors)
    ERROR_FS_INVALID_BLOCK_TYPE,
    ERROR_FS_EXPECTED_VALUE,
    ERROR_FS_EXPECTED_SMALLER_VALUE,
    ERROR_FS_EXPECTED_DOS_REVISION,
    ERROR_FS_EXPECTED_NO_REF,
    ERROR_FS_EXPECTED_REF,
    ERROR_FS_EXPECTED_SELFREF,
    ERROR_FS_PTR_TO_UNKNOWN_BLOCK,
    ERROR_FS_PTR_TO_EMPTY_BLOCK,
    ERROR_FS_PTR_TO_BOOT_BLOCK,
    ERROR_FS_PTR_TO_ROOT_BLOCK,
    ERROR_FS_PTR_TO_BITMAP_BLOCK,
    ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK,
    ERROR_FS_PTR_TO_USERDIR_BLOCK,
    ERROR_FS_PTR_TO_FILEHEADER_BLOCK,
    ERROR_FS_PTR_TO_FILELIST_BLOCK,
    ERROR_FS_PTR_TO_DATA_BLOCK,
    ERROR_FS_EXPECTED_DATABLOCK_NR,
    ERROR_FS_INVALID_HASHTABLE_SIZE
};
typedef ERROR_CODE ErrorCode;

#ifdef __cplusplus
struct ErrorCodeEnum : util::Reflection<ErrorCodeEnum, ErrorCode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = ERROR_FS_INVALID_HASHTABLE_SIZE;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "ERROR"; }
    static const char *key(ErrorCode value)
    {
        switch (value) {
                
            case ERROR_OK:                          return "OK";
            case ERROR_UNKNOWN:                     return "UNKNOWN";
                
            case ERROR_POWERED_OFF:                 return "POWERED_OFF";
            case ERROR_POWERED_ON:                  return "POWERED_ON";
            case ERROR_DEBUG_OFF:                   return "DEBUG_OFF";
            case ERROR_RUNNING:                     return "RUNNING";

            case ERROR_OPT_UNSUPPORTED:             return "OPT_UNSUPPORTED";
            case ERROR_OPT_INVARG:                  return "OPT_INVARG";
            case ERROR_OPT_LOCKED:                  return "OPT_LOCKED";
                
            case ERROR_INVALID_KEY:                 return "INVALID_KEY";
            case ERROR_SYNTAX:                      return "SYNTAX";
                
            case ERROR_BP_NOT_FOUND:                return "BP_NOT_FOUND";
            case ERROR_BP_ALREADY_SET:              return "BP_ALREADY_SET";
            case ERROR_WP_NOT_FOUND:                return "WP_NOT_FOUND";
            case ERROR_WP_ALREADY_SET:              return "WP_ALREADY_SET";
            case ERROR_CP_NOT_FOUND:                return "CP_NOT_FOUND";
            case ERROR_CP_ALREADY_SET:              return "CP_ALREADY_SET";
            case ERROR_CP_CANT_CATCH:               return "CP_CANT_CATCH";

            case ERROR_OUT_OF_MEMORY:               return "OUT_OF_MEMORY";

            case ERROR_DIR_NOT_FOUND:               return "DIR_NOT_FOUND";
            case ERROR_DIR_ACCESS_DENIED:           return "DIR_ACCESS_DENIED";
            case ERROR_FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case ERROR_FILE_EXISTS:                 return "FILE_EXISTS";
            case ERROR_FILE_IS_DIRECTORY:           return "FILE_IS_DIRECtORY";
            case ERROR_FILE_ACCESS_DENIED:          return "FILE_ACCESS_DENIED";
            case ERROR_FILE_TYPE_MISMATCH:          return "FILE_TYPE_MISMATCH";
            case ERROR_FILE_TYPE_UNSUPPORTED:       return "FILE_TYPE_UNSUPPORTED";
            case ERROR_FILE_CANT_READ:              return "FILE_CANT_READ";
            case ERROR_FILE_CANT_WRITE:             return "FILE_CANT_WRITE";
            case ERROR_FILE_CANT_CREATE:            return "FILE_CANT_CREATE";
                
            case ERROR_CHIP_RAM_MISSING:            return "CHIP_RAM_MISSING";
            case ERROR_CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case ERROR_AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";

            case ERROR_ROM_MISSING:                 return "ROM_MISSING";
            case ERROR_AROS_NO_EXTROM:              return "AROS_NO_EXTROM";

            case ERROR_WT_BLOCKED:                  return "WT_BLOCKED";
            case ERROR_WT:                          return "WT";

            case ERROR_DISK_MISSING:                return "DISK_MISSING";
            case ERROR_DISK_INCOMPATIBLE:           return "DISK_INCOMPATIBLE";
            case ERROR_DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case ERROR_DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
            case ERROR_DISK_INVALID_LAYOUT:         return "DISK_INVALID_LAYOUT";
            case ERROR_DISK_WRONG_SECTOR_COUNT:     return "DISK_WRONG_SECTOR_COUNT";
            case ERROR_DISK_INVALID_SECTOR_NUMBER:  return "DISK_INVALID_SECTOR_NUMBER";
                
            case ERROR_HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case ERROR_HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case ERROR_HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case ERROR_HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case ERROR_HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case ERROR_HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case ERROR_HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case ERROR_HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case ERROR_HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case ERROR_HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case ERROR_HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";
                
            case ERROR_HDC_INIT:                    return "HDC_INIT";

            case ERROR_SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case ERROR_SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case ERROR_SNAP_IS_BETA:                return "SNAP_IS_BETA";

            case ERROR_DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case ERROR_EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case ERROR_EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case ERROR_EXT_CORRUPTED:               return "EXT_CORRUPTED";
                
            case ERROR_MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case ERROR_INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
                
            case ERROR_REC_LAUNCH:                  return "REC_LAUNCH";

            case ERROR_OSDB:                        return "OSDB";
            case ERROR_HUNK_BAD_COOKIE:             return "HUNK_BAD_COOKIE";
            case ERROR_HUNK_NO_SECTIONS:            return "HUNK_NO_SECTIONS";
            case ERROR_HUNK_UNSUPPORTED:            return "HUNK_UNSUPPORTED";
            case ERROR_HUNK_CORRUPTED:              return "HUNK_CORRUPTED";

            case ERROR_SOCK_CANT_CREATE:            return "SOCK_CANT_CREATE";
            case ERROR_SOCK_CANT_CONNECT:           return "SOCK_CANT_CONNECT";
            case ERROR_SOCK_CANT_BIND:              return "SOCK_CANT_BIND";
            case ERROR_SOCK_CANT_LISTEN:            return "SOCK_CANT_LISTEN";
            case ERROR_SOCK_CANT_ACCEPT:            return "SOCK_CANT_ACCEPT";
            case ERROR_SOCK_CANT_RECEIVE:           return "SOCK_CANT_RECEIVE";
            case ERROR_SOCK_CANT_SEND:              return "SOCK_CANT_SEND";
            case ERROR_SOCK_DISCONNECTED:           return "SOCK_DISCONNECTED";
            case ERROR_SERVER_PORT_IN_USE:          return "SERVER_PORT_IN_USE";
            case ERROR_SERVER_ON:                   return "SERVER_ON";
            case ERROR_SERVER_OFF:                  return "SERVER_OFF";
            case ERROR_SERVER_RUNNING:              return "SERVER_RUNNING";
            case ERROR_SERVER_NOT_RUNNING:          return "SERVER_NOT_RUNNING";
            case ERROR_SERVER_NO_CLIENT:            return "SERVER_NO_CLIENT";
                
            case ERROR_GDB_NO_ACK:                  return "GDB_NO_ACK";
            case ERROR_GDB_INVALID_FORMAT:          return "GDB_INVALID_FORMAT";
            case ERROR_GDB_INVALID_CHECKSUM:        return "GDB_INVALID_CHECKSUM";
            case ERROR_GDB_UNRECOGNIZED_CMD:        return "GDB_UNRECOGNIZED_CMD";
            case ERROR_GDB_UNSUPPORTED_CMD:         return "GDB_UNSUPPORTED_CMD";

            case ERROR_FS_UNKNOWN:                  return "FS_UNKNOWN";
            case ERROR_FS_UNSUPPORTED:              return "FS_UNSUPPORTED";
            case ERROR_FS_UNFORMATTED:              return "FS_UNFORMATTED";
            case ERROR_FS_WRONG_BSIZE:              return "FS_WRONG_BSIZE";
            case ERROR_FS_WRONG_CAPACITY:           return "FS_WRONG_CAPACITY";
            case ERROR_FS_WRONG_DOS_TYPE:           return "FS_WRONG_DOS_TYPE";
            case ERROR_FS_HAS_CYCLES:               return "FS_HAS_CYCLES";
            case ERROR_FS_CORRUPTED:                return "FS_CORRUPTED";

            case ERROR_FS_OUT_OF_SPACE:             return "FS_OUT_OF_SPACE";
                
            case ERROR_FS_DIR_NOT_EMPTY:            return "FS_DIR_NOT_EMPTY";
            case ERROR_FS_CANNOT_CREATE_DIR:        return "FS_CANNOT_CREATE_DIR";
            case ERROR_FS_CANNOT_CREATE_FILE:       return "FS_CANNOT_CREATE_FILE";

            case ERROR_FS_INVALID_BLOCK_TYPE:       return "FS_INVALID_BLOCK_TYPE";
            case ERROR_FS_EXPECTED_VALUE:           return "FS_EXPECTED_VALUE";
            case ERROR_FS_EXPECTED_SMALLER_VALUE:   return "FS_EXPECTED_SMALLER_VALUE";
            case ERROR_FS_EXPECTED_DOS_REVISION:    return "FS_EXPECTED_DOS_REVISION";
            case ERROR_FS_EXPECTED_NO_REF:          return "FS_EXPECTED_NO_REF";
            case ERROR_FS_EXPECTED_REF:             return "FS_EXPECTED_REF";
            case ERROR_FS_EXPECTED_SELFREF:         return "FS_EXPECTED_SELFREF";
            case ERROR_FS_PTR_TO_UNKNOWN_BLOCK:     return "FS_PTR_TO_UNKNOWN_BLOCK";
            case ERROR_FS_PTR_TO_EMPTY_BLOCK:       return "FS_PTR_TO_EMPTY_BLOCK";
            case ERROR_FS_PTR_TO_BOOT_BLOCK:        return "FS_PTR_TO_BOOT_BLOCK";
            case ERROR_FS_PTR_TO_ROOT_BLOCK:        return "FS_PTR_TO_ROOT_BLOCK";
            case ERROR_FS_PTR_TO_BITMAP_BLOCK:      return "FS_PTR_TO_BITMAP_BLOCK";
            case ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK:  return "FS_PTR_TO_BITMAP_EXT_BLOCK";
            case ERROR_FS_PTR_TO_USERDIR_BLOCK:     return "FS_PTR_TO_USERDIR_BLOCK";
            case ERROR_FS_PTR_TO_FILEHEADER_BLOCK:  return "FS_PTR_TO_FILEHEADER_BLOCK";
            case ERROR_FS_PTR_TO_FILELIST_BLOCK:    return "FS_PTR_TO_FILELIST_BLOCK";
            case ERROR_FS_PTR_TO_DATA_BLOCK:        return "FS_PTR_TO_DATA_BLOCK";
            case ERROR_FS_EXPECTED_DATABLOCK_NR:    return "FS_EXPECTED_DATABLOCK_NR";
            case ERROR_FS_INVALID_HASHTABLE_SIZE:   return "FS_INVALID_HASHTABLE_SIZE";
        }
        return "???";
    }
};
#endif
