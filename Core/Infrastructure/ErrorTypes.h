// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

enum class Fault : long
{
    OK,                   ///< No error
    UNKNOWN,              ///< Unclassified error condition

    // Execution state
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

    // File IO
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
    ZLIB_ERROR,
    
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
    FS_OUT_OF_RANGE,
    FS_INVALID_PATH,
    FS_INVALID_REGEX,
    FS_NOT_A_DIRECTORY,
    FS_NOT_A_FILE,
    FS_NOT_A_FILE_OR_DIRECTORY,
    FS_NOT_FOUND,
    FS_EXISTS,
    FS_CANNOT_OPEN,

    FS_UNINITIALIZED,
    FS_UNFORMATTED,
    FS_UNSUPPORTED,
    FS_READ_ONLY,
    FS_WRONG_BSIZE,
    FS_WRONG_CAPACITY,
    FS_WRONG_DOS_TYPE,
    FS_WRONG_BLOCK_TYPE,
    FS_HAS_CYCLES,
    FS_CORRUPTED,

    // File system (import errors)
    FS_OUT_OF_SPACE,
    
    // File system (export errors)
    FS_DIR_NOT_EMPTY,
    FS_CANNOT_CREATE_DIR,
    FS_CANNOT_CREATE_FILE
};

struct FaultEnum : Reflection<FaultEnum, Fault>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Fault::FS_CANNOT_CREATE_FILE);
    
    static const char *_key(Fault value)
    {
        switch (value) {
                
            case Fault::OK:                          return "OK";
            case Fault::UNKNOWN:                     return "UNKNOWN";
                
            case Fault::LAUNCH:                      return "LAUNCH";
            case Fault::POWERED_OFF:                 return "POWERED_OFF";
            case Fault::POWERED_ON:                  return "POWERED_ON";
            case Fault::DEBUG_OFF:                   return "DEBUG_OFF";
            case Fault::RUNNING:                     return "RUNNING";
                
            case Fault::OPT_UNSUPPORTED:             return "OPT_UNSUPPORTED";
            case Fault::OPT_INV_ARG:                 return "OPT_INV_ARG";
            case Fault::OPT_INV_ID:                  return "OPT_INV_ID";
            case Fault::OPT_LOCKED:                  return "OPT_LOCKED";
                
            case Fault::INVALID_KEY:                 return "INVALID_KEY";
            case Fault::SYNTAX:                      return "SYNTAX";
                
            case Fault::CPU_UNSUPPORTED:             return "CPU_UNSUPPORTED";
            case Fault::GUARD_NOT_FOUND:             return "GUARD_NOT_FOUND";
            case Fault::GUARD_ALREADY_SET:           return "GUARD_ALREADY_SET";
            case Fault::BP_NOT_FOUND:                return "BP_NOT_FOUND";
            case Fault::BP_ALREADY_SET:              return "BP_ALREADY_SET";
            case Fault::WP_NOT_FOUND:                return "WP_NOT_FOUND";
            case Fault::WP_ALREADY_SET:              return "WP_ALREADY_SET";
            case Fault::CP_NOT_FOUND:                return "CP_NOT_FOUND";
            case Fault::CP_ALREADY_SET:              return "CP_ALREADY_SET";
                
            case Fault::OUT_OF_MEMORY:               return "OUT_OF_MEMORY";
                
            case Fault::DIR_NOT_FOUND:               return "DIR_NOT_FOUND";
            case Fault::DIR_ACCESS_DENIED:           return "DIR_ACCESS_DENIED";
            case Fault::DIR_CANT_CREATE:             return "DIR_CANT_CREATE";
            case Fault::DIR_NOT_EMPTY:               return "DIR_NOT_EMPTY";
            case Fault::FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case Fault::FILE_EXISTS:                 return "FILE_EXISTS";
            case Fault::FILE_IS_DIRECTORY:           return "FILE_IS_DIRECtORY";
            case Fault::FILE_ACCESS_DENIED:          return "FILE_ACCESS_DENIED";
            case Fault::FILE_TYPE_MISMATCH:          return "FILE_TYPE_MISMATCH";
            case Fault::FILE_TYPE_UNSUPPORTED:       return "FILE_TYPE_UNSUPPORTED";
            case Fault::FILE_CANT_READ:              return "FILE_CANT_READ";
            case Fault::FILE_CANT_WRITE:             return "FILE_CANT_WRITE";
            case Fault::FILE_CANT_CREATE:            return "FILE_CANT_CREATE";
                
            case Fault::CHIP_RAM_MISSING:            return "CHIP_RAM_MISSING";
            case Fault::CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case Fault::AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";
                
            case Fault::ROM_MISSING:                 return "ROM_MISSING";
            case Fault::AROS_NO_EXTROM:              return "AROS_NO_EXTROM";

            case Fault::DISK_MISSING:                return "DISK_MISSING";
            case Fault::DISK_INCOMPATIBLE:           return "DISK_INCOMPATIBLE";
            case Fault::DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case Fault::DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
            case Fault::DISK_INVALID_LAYOUT:         return "DISK_INVALID_LAYOUT";
            case Fault::DISK_WRONG_SECTOR_COUNT:     return "DISK_WRONG_SECTOR_COUNT";
            case Fault::DISK_INVALID_SECTOR_NUMBER:  return "DISK_INVALID_SECTOR_NUMBER";
                
            case Fault::HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case Fault::HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case Fault::HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case Fault::HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case Fault::HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case Fault::HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case Fault::HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case Fault::HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case Fault::HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case Fault::HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case Fault::HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case Fault::HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";
                
            case Fault::HDC_INIT:                    return "HDC_INIT";
                
            case Fault::SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case Fault::SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case Fault::SNAP_IS_BETA:                return "SNAP_IS_BETA";
            case Fault::SNAP_CORRUPTED:              return "SNAP_CORRUPTED";
                
            case Fault::DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case Fault::EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case Fault::EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case Fault::EXT_CORRUPTED:               return "EXT_CORRUPTED";
            case Fault::ZLIB_ERROR:                  return "ZLIB_ERROR";

            case Fault::MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case Fault::INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
                
            case Fault::REC_LAUNCH:                  return "REC_LAUNCH";
                
            case Fault::REG_READ_ONLY:               return "REG_READ_ONLY";
            case Fault::REG_WRITE_ONLY:              return "REG_WRITE_ONLY";
            case Fault::REG_UNUSED:                  return "REG_UNUSED";
            case Fault::ADDR_UNALIGNED:              return "ADDR_UNALIGNED";
                
            case Fault::OSDB:                        return "OSDB";
            case Fault::HUNK_BAD_COOKIE:             return "HUNK_BAD_COOKIE";
            case Fault::HUNK_BAD_HEADER:             return "HUNK_BAD_HEADER";
            case Fault::HUNK_NO_SECTIONS:            return "HUNK_NO_SECTIONS";
            case Fault::HUNK_UNSUPPORTED:            return "HUNK_UNSUPPORTED";
            case Fault::HUNK_CORRUPTED:              return "HUNK_CORRUPTED";
                
            case Fault::SOCK_CANT_CREATE:            return "SOCK_CANT_CREATE";
            case Fault::SOCK_CANT_CONNECT:           return "SOCK_CANT_CONNECT";
            case Fault::SOCK_CANT_BIND:              return "SOCK_CANT_BIND";
            case Fault::SOCK_CANT_LISTEN:            return "SOCK_CANT_LISTEN";
            case Fault::SOCK_CANT_ACCEPT:            return "SOCK_CANT_ACCEPT";
            case Fault::SOCK_CANT_RECEIVE:           return "SOCK_CANT_RECEIVE";
            case Fault::SOCK_CANT_SEND:              return "SOCK_CANT_SEND";
            case Fault::SOCK_DISCONNECTED:           return "SOCK_DISCONNECTED";
            case Fault::SERVER_PORT_IN_USE:          return "SERVER_PORT_IN_USE";
            case Fault::SERVER_ON:                   return "SERVER_ON";
            case Fault::SERVER_OFF:                  return "SERVER_OFF";
            case Fault::SERVER_RUNNING:              return "SERVER_RUNNING";
            case Fault::SERVER_NOT_RUNNING:          return "SERVER_NOT_RUNNING";
            case Fault::SERVER_NO_CLIENT:            return "SERVER_NO_CLIENT";
                
            case Fault::GDB_NO_ACK:                  return "GDB_NO_ACK";
            case Fault::GDB_INVALID_FORMAT:          return "GDB_INVALID_FORMAT";
            case Fault::GDB_INVALID_CHECKSUM:        return "GDB_INVALID_CHECKSUM";
            case Fault::GDB_UNRECOGNIZED_CMD:        return "GDB_UNRECOGNIZED_CMD";
            case Fault::GDB_UNSUPPORTED_CMD:         return "GDB_UNSUPPORTED_CMD";
                
            case Fault::FS_UNKNOWN:                  return "FS_UNKNOWN";
            case Fault::FS_OUT_OF_RANGE:             return "FS_OUT_OF_RANGE";
            case Fault::FS_INVALID_PATH:             return "FS_INVALID_PATH";
            case Fault::FS_INVALID_REGEX:            return "FS_INVALID_REGEX";
            case Fault::FS_NOT_A_DIRECTORY:          return "FS_NOT_A_DIRECTORY";
            case Fault::FS_NOT_A_FILE:               return "FS_NOT_A_FILE";
            case Fault::FS_NOT_A_FILE_OR_DIRECTORY:  return "FS_NOT_A_FILE_OR_DIRECTORY";
            case Fault::FS_NOT_FOUND:                return "FS_NOT_FOUND";
            case Fault::FS_EXISTS:                   return "FS_EXISTS";
            case Fault::FS_CANNOT_OPEN:              return "FS_CANNOT_OPEN";

            case Fault::FS_UNINITIALIZED:            return "FS_UNINITIALIZED";
            case Fault::FS_UNFORMATTED:              return "FS_UNFORMATTED";
            case Fault::FS_UNSUPPORTED:              return "FS_UNSUPPORTED";
            case Fault::FS_READ_ONLY:                return "FS_READ_ONLY";
            case Fault::FS_WRONG_BSIZE:              return "FS_WRONG_BSIZE";
            case Fault::FS_WRONG_CAPACITY:           return "FS_WRONG_CAPACITY";
            case Fault::FS_WRONG_DOS_TYPE:           return "FS_WRONG_DOS_TYPE";
            case Fault::FS_WRONG_BLOCK_TYPE:         return "FS_WRONG_BLOCK_TYPE";
            case Fault::FS_HAS_CYCLES:               return "FS_HAS_CYCLES";
            case Fault::FS_CORRUPTED:                return "FS_CORRUPTED";
                
            case Fault::FS_OUT_OF_SPACE:             return "FS_OUT_OF_SPACE";
                
            case Fault::FS_DIR_NOT_EMPTY:            return "FS_DIR_NOT_EMPTY";
            case Fault::FS_CANNOT_CREATE_DIR:        return "FS_CANNOT_CREATE_DIR";
            case Fault::FS_CANNOT_CREATE_FILE:       return "FS_CANNOT_CREATE_FILE";
        }
        return "???";
    }
    
    static const char *help(Fault value)
    {
        return "";
    }
};

}
