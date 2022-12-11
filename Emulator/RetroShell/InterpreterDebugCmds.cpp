// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "RetroShell.h"

namespace vamiga {

void
Interpreter::initDebugShell(Command &root)
{
    initCommons(root);


    //
    // General
    //

    root.add({"."},
             "command", "Exits the debugger",
             &RetroShell::exec <Token::debug>, 0);


    //
    // State control
    //

    root.add({"pause"},
             "command", "Pauses emulation",
             &RetroShell::exec <Token::amiga, Token::pause>, 0);

    root.add({"run"},
             "command", "Continues emulation",
             &RetroShell::exec <Token::amiga, Token::run>, 0);

    root.add({"reset"},
             "command", "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>, 0);

    /*
     root.add({"stepover"},
     "command", "Steps over the next instruction",
     &RetroShell::exec <Token::step>, 0);
     */

    //
    //
    //

    root.add({"memdump"},
             "command", "Dumps a portion of memory",
             &RetroShell::exec <Token::memdump>, 1);


    //
    // Breakpoints
    //

    root.add({"break"},
             "command", "Manages breakpoints");

    root.add({"break", "info"},
             "command", "Lists all breakpoints",
             &RetroShell::exec <Token::cpu, Token::bp, Token::info>, 0);

    root.add({"break", "at"},
             "command", "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::cpu, Token::bp, Token::at>, 1);

    root.add({"break", "delete"},
             "command", "Deletes a breakpoint",
             &RetroShell::exec <Token::cpu, Token::bp, Token::del>, 1);

    root.add({"break", "enable"},
             "command", "Enables a breakpoint",
             &RetroShell::exec <Token::cpu, Token::bp, Token::enable>, 1);

    root.add({"break", "disable"},
             "command", "Disables a breakpoint",
             &RetroShell::exec <Token::cpu, Token::bp, Token::disable>, 1);

    root.add({"break", "ignore"},
             "command", "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::cpu, Token::bp, Token::ignore>, 2);


    //
    // Watchpoints
    //

    root.add({"watch"},
             "command", "Manages watchpoints");

    root.add({"watch", "info"},
             "command", "Lists all watchpoints",
             &RetroShell::exec <Token::cpu, Token::wp, Token::info>, 0);

    root.add({"watch", "at"},
             "command", "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::cpu, Token::wp, Token::at>, 1);

    root.add({"watch", "delete"},
             "command", "Deletes a watchpoint",
             &RetroShell::exec <Token::cpu, Token::wp, Token::del>, 1);

    root.add({"watch", "enable"},
             "command", "Enables a watchpoint",
             &RetroShell::exec <Token::cpu, Token::wp, Token::enable>, 1);

    root.add({"watch", "disable"},
             "command", "Disables a watchpoint",
             &RetroShell::exec <Token::cpu, Token::wp, Token::disable>, 1);

    root.add({"watch", "ignore"},
             "command", "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::cpu, Token::wp, Token::ignore>, 2);


    //
    // Catchpoints
    //

    root.add({"catch"},
             "command", "Manages catchpoints");

    root.add({"catch", "info"},
             "command", "Lists all catchpoints",
             &RetroShell::exec <Token::cpu, Token::cp, Token::info>, 0);

    root.add({"catch", "vector"},
             "command", "Catches an exception vector",
             &RetroShell::exec <Token::cpu, Token::cp, Token::vector>, 1);

    root.add({"catch", "interrupt"},
             "command", "Catches an interrupt",
             &RetroShell::exec <Token::cpu, Token::cp, Token::interrupt>, 1);

    root.add({"catch", "trap"},
             "command", "Catches a trap instruction",
             &RetroShell::exec <Token::cpu, Token::cp, Token::trap>, 1);

    root.add({"catch", "delete"},
             "command", "Deletes a catchpoint",
             &RetroShell::exec <Token::cpu, Token::cp, Token::del>, 1);

    root.add({"catch", "enable"},
             "command", "Enables a catchpoint",
             &RetroShell::exec <Token::cpu, Token::cp, Token::enable>, 1);

    root.add({"catch", "disable"},
             "command", "Disables a catchpoint",
             &RetroShell::exec <Token::cpu, Token::cp, Token::disable>, 1);

    root.add({"catch", "ignore"},
             "command", "Ignores a catchpoint a certain number of times",
             &RetroShell::exec <Token::cpu, Token::cp, Token::ignore>, 2);


    //
    // Software traps
    //

    root.add({"swtraps"},
             "command", "Lists all software traps",
             &RetroShell::exec <Token::cpu, Token::swtraps>, 0);


    //
    // OS Debugger
    //

    root.add({"os"},
             "component", "AmigaOS debugger");

    root.add({"os", "info"},
             "command", "Displays basic system information",
             &RetroShell::exec <Token::os, Token::info>, 0);

    root.add({"os", "execbase"},
             "command", "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>, 0);

    root.add({"os", "interrupts"},
             "command", "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>, 0);

    root.add({"os", "libraries"},
             "command", "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>, {0, 1});

    root.add({"os", "devices"},
             "command", "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>, {0, 1});

    root.add({"os", "resources"},
             "command", "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>, {0, 1});

    root.add({"os", "tasks"},
             "command", "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>, {0, 1});

    root.add({"os", "processes"},
             "command", "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>, {0, 1});

    root.add({"os", "catch"},
             "command", "Pauses emulation on task launch",
             &RetroShell::exec <Token::os, Token::cp>, 1);

    root.add({"os", "set"},
             "command", "Configures the component");

    root.add({"os", "set", "diagboard" },
             "command", "Attaches or detaches the debug expansion board",
             &RetroShell::exec <Token::os, Token::set, Token::diagboard>, 1);


    //
    // Misc
    //

    root.add({"jump"},
             "command", "Jumps to the specified address",
             &RetroShell::exec <Token::cpu, Token::jump>, 1);


    //
    // Components
    //

    root.add({"amiga"},
             "component", "Inspects the Amiga",
             &RetroShell::exec <Token::amiga, Token::state>, 0);

    root.add({"cpu"},
             "component", "Inspects the CPU",
             &RetroShell::exec <Token::cpu, Token::state>, 0);

    root.add({"cia"},
             "component", "Inspects the CIA",
             &RetroShell::exec <Token::cia, Token::state>, 0);

    root.add({"agnus"},
             "component", "Inspects Agnus",
             &RetroShell::exec <Token::agnus, Token::state>, 0);

    root.add({"paula"},
             "component", "Inspects Paula",
             &RetroShell::exec <Token::paula, Token::state>, 0);

    root.add({"denise"},
             "component", "Inspects Denise",
             &RetroShell::exec <Token::denise, Token::state>, 0);


}


}
