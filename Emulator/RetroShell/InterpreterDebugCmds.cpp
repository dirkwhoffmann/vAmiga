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
    // Instruction Stream
    //

    root.addGroup("Managing the instruction stream...");

    root.add({"pause"},
             "command", "Pauses emulation",
             &RetroShell::exec <Token::pause>, 0);

    root.add({"run"},
             "command", "Continues emulation",
             &RetroShell::exec <Token::run>, 0);

    root.add({"step"},
             "command", "Steps into the next instruction",
             &RetroShell::exec <Token::step>, 0);

    root.add({"next"},
             "command", "Steps over the next instruction",
             &RetroShell::exec <Token::next>, 0);

    root.add({"goto"},
             "command", "Redirects the program counter",
             &RetroShell::exec <Token::jump>, 1);

    root.add({"disassemble"},
             "command", "Runs disassembler",
             &RetroShell::exec <Token::disassemble>, 1);

    // Breakpoints

    root.add({"break"},
             "command", "Manages breakpoints",
             &RetroShell::exec <Token::bp>, 0);

    root.add({"break", "at"},
             "command", "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::bp, Token::at>, 1);

    root.add({"break", "delete"},
             "command", "Deletes a breakpoint",
             &RetroShell::exec <Token::bp, Token::del>, 1);

    root.add({"break", "enable"},
             "command", "Enables a breakpoint",
             &RetroShell::exec <Token::bp, Token::enable>, 1);

    root.add({"break", "disable"},
             "command", "Disables a breakpoint",
             &RetroShell::exec <Token::bp, Token::disable>, 1);

    root.add({"break", "ignore"},
             "command", "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::bp, Token::ignore>, 2);

    // Watchpoints

    root.add({"watch"},
             "command", "Manages watchpoints",
             &RetroShell::exec <Token::wp>, 0);

    root.add({"watch", "at"},
             "command", "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::wp, Token::at>, 1);

    root.add({"watch", "delete"},
             "command", "Deletes a watchpoint",
             &RetroShell::exec <Token::wp, Token::del>, 1);

    root.add({"watch", "enable"},
             "command", "Enables a watchpoint",
             &RetroShell::exec <Token::wp, Token::enable>, 1);

    root.add({"watch", "disable"},
             "command", "Disables a watchpoint",
             &RetroShell::exec <Token::wp, Token::disable>, 1);

    root.add({"watch", "ignore"},
             "command", "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::wp, Token::ignore>, 2);

    // Catchpoints

    root.add({"catch"},
             "command", "Manages catchpoints",
             &RetroShell::exec <Token::cp>, 0);

    root.add({"catch", "vector"},
             "command", "Catches an exception vector",
             &RetroShell::exec <Token::cp, Token::vector>, 1);

    root.add({"catch", "interrupt"},
             "command", "Catches an interrupt",
             &RetroShell::exec <Token::cp, Token::interrupt>, 1);

    root.add({"catch", "trap"},
             "command", "Catches a trap instruction",
             &RetroShell::exec <Token::cp, Token::trap>, 1);

    root.add({"catch", "delete"},
             "command", "Deletes a catchpoint",
             &RetroShell::exec <Token::cp, Token::del>, 1);

    root.add({"catch", "enable"},
             "command", "Enables a catchpoint",
             &RetroShell::exec <Token::cp, Token::enable>, 1);

    root.add({"catch", "disable"},
             "command", "Disables a catchpoint",
             &RetroShell::exec <Token::cp, Token::disable>, 1);

    root.add({"catch", "ignore"},
             "command", "Ignores a catchpoint a certain number of times",
             &RetroShell::exec <Token::cp, Token::ignore>, 2);


    //
    // Components
    //

    root.addGroup("Exploring components...");

    root.add({"amiga"},
             "component", "The virtual Amiga",
             &RetroShell::exec <Token::amiga>, 0);

    root.add({"memory"},
             "component", "RAM and ROM",
             &RetroShell::exec <Token::memory>, 0);

    root.add({"memory", "dump"},
             "command", "Generates a memory hexdump",
             &RetroShell::exec <Token::memory, Token::memdump>, 1);

    root.add({"memory", "banks"},
             "command", "Dumps the memory bank map",
             &RetroShell::exec <Token::memory, Token::bankmap>, 0);

    root.add({"memory", "checksum"},
             "command", "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::checksums>, 0);

    root.add({"cpu"},
             "component", "Motorola 68k CPU",
             &RetroShell::exec <Token::cpu>, 0);

    root.add({"cpu", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::cpu, Token::state>, 0);

    root.add({"cpu", "vectors"},
             "component", "Dumps the vector table",
             &RetroShell::exec <Token::cpu, Token::vectors>, 0);

    //
    // CIA
    //

    root.add({"ciaa"},
             "component", "Complex Interface Adapter A",
             &RetroShell::exec <Token::cia>, 0, 0);

    root.add({"ciab"},
             "component", "Complex Interface Adapter B",
             &RetroShell::exec <Token::cia>, 0, 1);

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({cia, "state"},
                 "component", "Displays the internal state",
                 &RetroShell::exec <Token::cia>, 0, i);

        root.add({cia, "tod"},
                 "category", "Displays the state of the 24-bit counter",
                 &RetroShell::exec <Token::cia, Token::tod>, 0, i);
    }

    root.add({"agnus"},
             "component", "Custom Chipset",
             &RetroShell::exec <Token::agnus>, 0);

    root.add({"agnus", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::agnus, Token::state>, 0);

    root.add({"agnus", "beam"},
             "category", "Displays the current beam position",
             &RetroShell::exec <Token::agnus, Token::beam>, 0);

    root.add({"agnus", "dma"},
             "component", "Prints all scheduled DMA events",
             &RetroShell::exec <Token::agnus, Token::dma>, 0);

    root.add({"agnus", "events"},
             "component", "Inspects the event scheduler",
             &RetroShell::exec <Token::agnus, Token::events>, 0);

    root.add({"blitter"},
             "component", "Custom Chipset",
             &RetroShell::exec <Token::blitter>, 0);

    root.add({"blitter", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::blitter, Token::state>, 0);

    root.add({"copper"},
             "component", "Custom Chipset",
             &RetroShell::exec <Token::copper>, 0);

    root.add({"copper", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::copper, Token::state>, 0);

    root.add({"copper", "list"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::copper, Token::list>, 0);

    root.add({"paula"},
             "component", "Custom Chipset",
             &RetroShell::exec <Token::paula>, 0);

    root.add({"paula", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::state>, 0);

    root.add({"denise"},
             "component", "Custom Chipset",
             &RetroShell::exec <Token::denise>, 0);

    root.add({"denise", "state"},
             "component", "Inspects the internal state",
             &RetroShell::exec <Token::denise, Token::state>, 0);

    // OS Debugger

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
}

}
