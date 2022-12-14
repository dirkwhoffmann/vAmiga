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
    // Top-level commands
    //

    root.newGroup("Controlling the instruction stream");

    root.add({"pause"},
             "Pauses emulation",
             &RetroShell::exec <Token::pause>, 0);

    root.add({"run"},
             "Continues emulation",
             &RetroShell::exec <Token::run>, 0);

    root.add({"step"},
             "Steps into the next instruction",
             &RetroShell::exec <Token::step>, 0);

    root.add({"next"},
             "Steps over the next instruction",
             &RetroShell::exec <Token::next>, 0);

    root.add({"goto"},
             "Redirects the program counter",
             &RetroShell::exec <Token::jump>, 1);

    root.add({"disassemble"},
             "Runs disassembler",
             &RetroShell::exec <Token::disassemble>, 1);


    root.newGroup("Guarding the program execution");

    root.add({"break"},
             "Manages breakpoints");

    root.add({"watch"},
             "Manages watchpoints");

    root.add({"catch"},
             "Manages catchpoints");


    root.newGroup("Exploring components");

    root.add({"amiga"},
             "Main computer");

    root.add({"memory"},
             "RAM and ROM");

    root.add({"cpu"},
             "Motorola 68k CPU");

    root.add({"ciaa"},
             "Complex Interface Adapter A");

    root.add({"ciab"},
             "Complex Interface Adapter B");

    root.add({"agnus"},
             "Custom Chipset");

    root.add({"blitter"},
             "Coprocessor");

    root.add({"copper"},
             "Coprocessor");

    root.add({"paula"},
             "Custom Chipset");

    root.add({"denise"},
             "Custom Chipset");

    root.add({"os"},
             "AmigaOS debugger");

    
    //
    // Breakpoints
    //

    root.newGroup("");

    root.add({"break", ""},
             "Lists all breakpoints",
             &RetroShell::exec <Token::bp>, 0);

    root.add({"break", "at"},
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::bp, Token::at>, 1);

    root.add({"break", "delete"},
             "Deletes a breakpoint",
             &RetroShell::exec <Token::bp, Token::del>, 1);

    root.add({"break", "enable"},
             "Enables a breakpoint",
             &RetroShell::exec <Token::bp, Token::enable>, 1);

    root.add({"break", "disable"},
             "Disables a breakpoint",
             &RetroShell::exec <Token::bp, Token::disable>, 1);

    root.add({"break", "ignore"},
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::bp, Token::ignore>, 2);

    //
    // Watchpoints
    //

    root.newGroup("");

    root.add({"watch", ""},
             "Lists all watchpoints",
             &RetroShell::exec <Token::wp>, 0);

    root.add({"watch", "at"},
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::wp, Token::at>, 1);

    root.add({"watch", "delete"},
             "Deletes a watchpoint",
             &RetroShell::exec <Token::wp, Token::del>, 1);

    root.add({"watch", "enable"},
             "Enables a watchpoint",
             &RetroShell::exec <Token::wp, Token::enable>, 1);

    root.add({"watch", "disable"},
             "Disables a watchpoint",
             &RetroShell::exec <Token::wp, Token::disable>, 1);

    root.add({"watch", "ignore"},
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::wp, Token::ignore>, 2);

    //
    // Catchpoints
    //

    root.newGroup("");

    root.add({"catch", ""},
             "Lists all catchpoints",
             &RetroShell::exec <Token::cp>, 0);

    root.add({"catch", "vector"},
             "Catches an exception vector",
             &RetroShell::exec <Token::cp, Token::vector>, 1);

    root.add({"catch", "interrupt"},
             "Catches an interrupt",
             &RetroShell::exec <Token::cp, Token::interrupt>, 1);

    root.add({"catch", "trap"},
             "Catches a trap instruction",
             &RetroShell::exec <Token::cp, Token::trap>, 1);

    root.add({"catch", "delete"},
             "Deletes a catchpoint",
             &RetroShell::exec <Token::cp, Token::del>, 1);

    root.add({"catch", "enable"},
             "Enables a catchpoint",
             &RetroShell::exec <Token::cp, Token::enable>, 1);

    root.add({"catch", "disable"},
             "Disables a catchpoint",
             &RetroShell::exec <Token::cp, Token::disable>, 1);

    root.add({"catch", "ignore"},
             "Ignores a catchpoint a certain number of times",
             &RetroShell::exec <Token::cp, Token::ignore>, 2);


    //
    // Components
    //

    root.newGroup("");

    root.add({"amiga", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::amiga>, 0);

    root.add({"memory", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::memory>, 0);

    root.add({"memory", "dump"},
             "Generates a memory hexdump",
             &RetroShell::exec <Token::memory, Token::memdump>, 1);

    root.add({"memory", "banks"},
             "Dumps the memory bank map",
             &RetroShell::exec <Token::memory, Token::bankmap>, 0);

    root.add({"memory", "checksum"},
             "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::checksums>, 0);

    root.add({"cpu", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::cpu>, 0);

    root.add({"cpu", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::cpu, Token::state>, 0);

    root.add({"cpu", "vectors"},
             "Dumps the vector table",
             &RetroShell::exec <Token::cpu, Token::vectors>, 0);

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({cia, ""},
                 "Displays the internal state",
                 &RetroShell::exec <Token::cia>, 0, i);

        root.add({cia, "state"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::cia, Token::state>, 0, i);

        root.add({cia, "tod"},
                 "Displays the state of the 24-bit counter",
                 &RetroShell::exec <Token::cia, Token::tod>, 0, i);
    }

    root.add({"agnus", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::agnus>, 0);

    root.add({"agnus", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::agnus, Token::state>, 0);

    root.add({"agnus", "beam"},
             "Displays the current beam position",
             &RetroShell::exec <Token::agnus, Token::beam>, 0);

    root.add({"agnus", "dma"},
             "Prints all scheduled DMA events",
             &RetroShell::exec <Token::agnus, Token::dma>, 0);

    root.add({"agnus", "events"},
             "Inspects the event scheduler",
             &RetroShell::exec <Token::agnus, Token::events>, 0);

    root.add({"blitter", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::blitter>, 0);

    root.add({"blitter", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::blitter, Token::state>, 0);

    root.add({"copper", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::copper>, 0);

    root.add({"copper", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::copper, Token::state>, 0);

    root.add({"copper", "list"},
             "Inspects the internal state",
             &RetroShell::exec <Token::copper, Token::list>, 0);

    root.add({"paula", ""},
             "Custom Chipset",
             &RetroShell::exec <Token::paula>, 0);

    root.add({"paula", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::state>, 0);

    root.add({"denise", ""},
             "Lorem ipsum",
             &RetroShell::exec <Token::denise>, 0);

    root.add({"denise", "state"},
             "Inspects the internal state",
             &RetroShell::exec <Token::denise, Token::state>, 0);

    root.add({"os", "info"},
             "Displays basic system information",
             &RetroShell::exec <Token::os, Token::info>, 0);

    root.add({"os", "execbase"},
             "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>, 0);

    root.add({"os", "interrupts"},
             "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>, 0);

    root.add({"os", "libraries"},
             "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>, {0, 1});

    root.add({"os", "devices"},
             "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>, {0, 1});

    root.add({"os", "resources"},
             "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>, {0, 1});

    root.add({"os", "tasks"},
             "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>, {0, 1});

    root.add({"os", "processes"},
             "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>, {0, 1});

    root.add({"os", "catch"},
             "Pauses emulation on task launch",
             &RetroShell::exec <Token::os, Token::cp>, 1);

    root.add({"os", "set"},
             "Configures the component");

    root.add({"os", "set", "diagboard" },
             "Attaches or detaches the debug expansion board",
             &RetroShell::exec <Token::os, Token::set, Token::diagboard>, 1);
}

}
