// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CoreError.h"
#include "Constants.h"
#include "Macros.h"
#include "DeviceTypes.h"

namespace vamiga {

CoreError::CoreError(long code, const string &s) : Error(code)
{
    // string _msg;
    
    switch (code)
    {

        case OK:
            fatalError;
            break;
            
        case LAUNCH:
            set_msg("Launch error.");
            break;

        case POWERED_OFF:
            set_msg("The emulator is powered off.");
            break;

        case POWERED_ON:
            set_msg("The emulator is powered on.");
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
            
        case HDC_INIT:
            set_msg("Failed to initialize hard drive: " + s);
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
