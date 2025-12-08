// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

struct CoreError : public Error {

    static constexpr long OK                =   0; ///< No error
    static constexpr long UNKNOWN           =   1; ///< Unclassified error condition

    // Execution state
    static constexpr long LAUNCH            = 100; ///< Launch error
    static constexpr long POWERED_OFF       = 101; ///< The emulator is powered off
    static constexpr long POWERED_ON        = 102; ///< The emulator is powered on
    static constexpr long DEBUG_OFF         = 103;
    static constexpr long RUNNING           = 104; ///< The emulator is running

    // Configuration
    static constexpr long OPT_UNSUPPORTED   = 200; ///< Unsupported configuration option
    static constexpr long OPT_INV_ARG       = 201; ///< Invalid argument
    static constexpr long OPT_INV_ID        = 202; ///< Invalid component id
    static constexpr long OPT_LOCKED        = 203; ///< The option is temporarily locked

    // Property storage
    static constexpr long INVALID_KEY       = 300; ///< Invalid property key
    static constexpr long SYNTAX            = 301; ///< Syntax error

    // CPU
    static constexpr long CPU_UNSUPPORTED   = 400; ///< Unsupported CPU model
    static constexpr long GUARD_NOT_FOUND   = 401; ///< Guard is not set
    static constexpr long GUARD_ALREADY_SET = 402; ///< Guard is already set
    static constexpr long BP_NOT_FOUND      = 403; ///< Breakpoint is not set (DEPRECATED)
    static constexpr long BP_ALREADY_SET    = 404; ///< Breakpoint is already set (DEPRECATED)
    static constexpr long WP_NOT_FOUND      = 405; ///< Watchpoint is not set (DEPRECATED)
    static constexpr long WP_ALREADY_SET    = 406; ///< Watchpoint is already set (DEPRECATED)
    static constexpr long CP_NOT_FOUND      = 407; ///< Catchpoint is not set (DEPRECATED)
    static constexpr long CP_ALREADY_SET    = 408; ///< Catchpoint is already set (DEPRECATED)

    // Memory
    static constexpr long OUT_OF_MEMORY     = 500; ///< Out of memory
    static constexpr long CHIP_RAM_MISSING  = 501;
    static constexpr long CHIP_RAM_LIMIT    = 502;
    static constexpr long AROS_RAM_LIMIT    = 503;
    static constexpr long ROM_MISSING       = 504;
    static constexpr long AROS_NO_EXTROM    = 505;

    // Floppy disks
    static constexpr long DISK_MISSING               = 600;
    static constexpr long DISK_INCOMPATIBLE          = 601;
    static constexpr long DISK_INVALID_DIAMETER      = 602;
    static constexpr long DISK_INVALID_DENSITY       = 603;
    static constexpr long DISK_INVALID_LAYOUT        = 604;
    static constexpr long DISK_WRONG_SECTOR_COUNT    = 605;
    static constexpr long DISK_INVALID_SECTOR_NUMBER = 606;

    // Hard disks
    static constexpr long HDR_TOO_LARGE              = 700;
    static constexpr long HDR_UNSUPPORTED_CYL_COUNT  = 701;
    static constexpr long HDR_UNSUPPORTED_HEAD_COUNT = 702;
    static constexpr long HDR_UNSUPPORTED_SEC_COUNT  = 703;
    static constexpr long HDR_UNSUPPORTED_BSIZE      = 704;
    static constexpr long HDR_UNKNOWN_GEOMETRY       = 705;
    static constexpr long HDR_UNMATCHED_GEOMETRY     = 706;
    static constexpr long HDR_UNPARTITIONED          = 707;
    static constexpr long HDR_CORRUPTED_PTABLE       = 708;
    static constexpr long HDR_CORRUPTED_FSH          = 709;
    static constexpr long HDR_CORRUPTED_LSEG         = 710;
    static constexpr long HDR_UNSUPPORTED            = 711;

    // Hard drive controller
    static constexpr long HDC_INIT                   = 800;

    // Snapshots
    static constexpr long SNAP_TOO_OLD               = 901; ///< Snapshot was created with an older version
    static constexpr long SNAP_TOO_NEW               = 902; ///< Snapshot was created with a later version
    static constexpr long SNAP_IS_BETA               = 903; ///< Snapshot was created with a beta release
    static constexpr long SNAP_CORRUPTED             = 904; ///< Snapshot data is corrupted

    // Media files
    static constexpr long DMS_CANT_CREATE            = 1000;
    static constexpr long EXT_FACTOR5                = 1001;
    static constexpr long EXT_INCOMPATIBLE           = 1002;
    static constexpr long EXT_CORRUPTED              = 1003;
    static constexpr long ZLIB_ERROR                 = 1004;

    // Encrypted Roms
    static constexpr long MISSING_ROM_KEY            = 1104;
    static constexpr long INVALID_ROM_KEY            = 1105;

    // Debugger
    static constexpr long REG_READ_ONLY              = 1200;
    static constexpr long REG_WRITE_ONLY             = 1201;
    static constexpr long REG_UNUSED                 = 1202;
    static constexpr long ADDR_UNALIGNED             = 1203;

    // OS Debugger
    static constexpr long OSDB                       = 1300;
    static constexpr long HUNK_BAD_COOKIE            = 1301;
    static constexpr long HUNK_BAD_HEADER            = 1302;
    static constexpr long HUNK_NO_SECTIONS           = 1303;
    static constexpr long HUNK_UNSUPPORTED           = 1304;
    static constexpr long HUNK_CORRUPTED             = 1305;

    const char *errstr() const noexcept override {

        switch (payload) {

            case OK:                          return "OK";
            case UNKNOWN:                     return "UNKNOWN";

            case LAUNCH:                      return "LAUNCH";
            case POWERED_OFF:                 return "POWERED_OFF";
            case POWERED_ON:                  return "POWERED_ON";
            case DEBUG_OFF:                   return "DEBUG_OFF";
            case RUNNING:                     return "RUNNING";

            case OPT_UNSUPPORTED:             return "OPT_UNSUPPORTED";
            case OPT_INV_ARG:                 return "OPT_INV_ARG";
            case OPT_INV_ID:                  return "OPT_INV_ID";
            case OPT_LOCKED:                  return "OPT_LOCKED";

            case INVALID_KEY:                 return "INVALID_KEY";
            case SYNTAX:                      return "SYNTAX";

            case CPU_UNSUPPORTED:             return "CPU_UNSUPPORTED";
            case GUARD_NOT_FOUND:             return "GUARD_NOT_FOUND";
            case GUARD_ALREADY_SET:           return "GUARD_ALREADY_SET";
            case BP_NOT_FOUND:                return "BP_NOT_FOUND";
            case BP_ALREADY_SET:              return "BP_ALREADY_SET";
            case WP_NOT_FOUND:                return "WP_NOT_FOUND";
            case WP_ALREADY_SET:              return "WP_ALREADY_SET";
            case CP_NOT_FOUND:                return "CP_NOT_FOUND";
            case CP_ALREADY_SET:              return "CP_ALREADY_SET";

            case OUT_OF_MEMORY:               return "OUT_OF_MEMORY";
            case CHIP_RAM_MISSING:            return "CHIP_RAM_MISSING";
            case CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";
            case ROM_MISSING:                 return "ROM_MISSING";
            case AROS_NO_EXTROM:              return "AROS_NO_EXTROM";

            case DISK_MISSING:                return "DISK_MISSING";
            case DISK_INCOMPATIBLE:           return "DISK_INCOMPATIBLE";
            case DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
            case DISK_INVALID_LAYOUT:         return "DISK_INVALID_LAYOUT";
            case DISK_WRONG_SECTOR_COUNT:     return "DISK_WRONG_SECTOR_COUNT";
            case DISK_INVALID_SECTOR_NUMBER:  return "DISK_INVALID_SECTOR_NUMBER";

            case HDR_TOO_LARGE:               return "HDR_TOO_LARGE";
            case HDR_UNSUPPORTED_CYL_COUNT:   return "HDR_UNSUPPORTED_CYL_COUNT";
            case HDR_UNSUPPORTED_HEAD_COUNT:  return "HDR_UNSUPPORTED_HEAD_COUNT";
            case HDR_UNSUPPORTED_SEC_COUNT:   return "HDR_UNSUPPORTED_SEC_COUNT";
            case HDR_UNSUPPORTED_BSIZE:       return "HDR_UNSUPPORTED_BSIZE";
            case HDR_UNKNOWN_GEOMETRY:        return "HDR_UNKNOWN_GEOMETRY";
            case HDR_UNMATCHED_GEOMETRY:      return "HDR_UNMATCHED_GEOMETRY";
            case HDR_UNPARTITIONED:           return "HDR_UNPARTITIONED";
            case HDR_CORRUPTED_PTABLE:        return "HDR_CORRUPTED_PTABLE";
            case HDR_CORRUPTED_FSH:           return "HDR_CORRUPTED_FSH";
            case HDR_CORRUPTED_LSEG:          return "HDR_CORRUPTED_LSEG";
            case HDR_UNSUPPORTED:             return "HDR_UNSUPPORTED";

            case HDC_INIT:                    return "HDC_INIT";

            case SNAP_TOO_OLD:                return "SNAP_TOO_OLD";
            case SNAP_TOO_NEW:                return "SNAP_TOO_NEW";
            case SNAP_IS_BETA:                return "SNAP_IS_BETA";
            case SNAP_CORRUPTED:              return "SNAP_CORRUPTED";

            case DMS_CANT_CREATE:             return "DMS_CANT_CREATE";
            case EXT_FACTOR5:                 return "EXT_UNSUPPORTED";
            case EXT_INCOMPATIBLE:            return "EXT_INCOMPATIBLE";
            case EXT_CORRUPTED:               return "EXT_CORRUPTED";
            case ZLIB_ERROR:                  return "ZLIB_ERROR";

            case MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case INVALID_ROM_KEY:             return "INVALID_ROM_KEY";

            case REG_READ_ONLY:               return "REG_READ_ONLY";
            case REG_WRITE_ONLY:              return "REG_WRITE_ONLY";
            case REG_UNUSED:                  return "REG_UNUSED";
            case ADDR_UNALIGNED:              return "ADDR_UNALIGNED";

            case OSDB:                        return "OSDB";
            case HUNK_BAD_COOKIE:             return "HUNK_BAD_COOKIE";
            case HUNK_BAD_HEADER:             return "HUNK_BAD_HEADER";
            case HUNK_NO_SECTIONS:            return "HUNK_NO_SECTIONS";
            case HUNK_UNSUPPORTED:            return "HUNK_UNSUPPORTED";
            case HUNK_CORRUPTED:              return "HUNK_CORRUPTED";
        }
        return "???";
    }

    CoreError(long fault, const string &s);
    CoreError(long fault, const char *s) : CoreError(fault, string(s)) { };
    CoreError(long fault, const fs::path &p) : CoreError(fault, p.string()) { };
    CoreError(long fault, std::integral auto v) : CoreError(fault, std::to_string(v)) { };
    CoreError(long fault) : CoreError(fault, "") { }
};

}
