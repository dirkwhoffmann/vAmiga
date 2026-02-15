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
    static constexpr long RUNNING           = 103; ///< The emulator is running

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

    // Hard drive controller
    static constexpr long HDC_INIT          = 600;

    // Debugger
    static constexpr long REG_READ_ONLY     = 700;
    static constexpr long REG_WRITE_ONLY    = 701;
    static constexpr long REG_UNUSED        = 702;
    static constexpr long ADDR_UNALIGNED    = 703;

    // OS Debugger
    static constexpr long OSDB              = 800;
    static constexpr long HUNK_BAD_COOKIE   = 801;
    static constexpr long HUNK_BAD_HEADER   = 802;
    static constexpr long HUNK_NO_SECTIONS  = 803;
    static constexpr long HUNK_UNSUPPORTED  = 804;
    static constexpr long HUNK_CORRUPTED    = 805;

    const char *errstr() const noexcept override {

        switch (payload) {

            case OK:                          return "OK";
            case UNKNOWN:                     return "UNKNOWN";

            case LAUNCH:                      return "LAUNCH";
            case POWERED_OFF:                 return "POWERED_OFF";
            case POWERED_ON:                  return "POWERED_ON";
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

            case HDC_INIT:                    return "HDC_INIT";

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

    explicit CoreError(long fault, const string &s);
    explicit CoreError(long fault, const char *s) : CoreError(fault, string(s)) { };
    explicit CoreError(long fault, const fs::path &p) : CoreError(fault, p.string()) { };
    explicit CoreError(long fault, std::integral auto v) : CoreError(fault, std::to_string(v)) { };
    explicit CoreError(long fault) : CoreError(fault, "") { }
};

}
