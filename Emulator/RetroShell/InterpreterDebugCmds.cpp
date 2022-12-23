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
             &RetroShell::exec <Token::pause>);

    root.add({"run"},
             "Continues emulation",
             &RetroShell::exec <Token::run>);

    root.add({"step"},
             "Steps into the next instruction",
             &RetroShell::exec <Token::step>);

    root.add({"next"},
             "Steps over the next instruction",
             &RetroShell::exec <Token::next>);

    root.add({"goto"}, { Arg::address },
             "Redirects the program counter",
             &RetroShell::exec <Token::jump>);

    root.add({"disassemble"}, { Arg::address },
             "Runs disassembler",
             &RetroShell::exec <Token::disassemble>);


    root.newGroup("Guarding the program execution");

    root.add({"break"},     "Manages CPU breakpoints");
    root.add({"watch"},     "Manages CPU watchpoints");
    root.add({"catch"},     "Manages CPU catchpoints");
    root.add({"cbreak"},    "Manages Copper breakpoints");
    root.add({"cwatch"},    "Manages Copper watchpoints");

    root.newGroup("Debugging components");

    root.add({"amiga"},         "Main computer");
    root.add({"memory"},        "RAM and ROM");
    root.add({"cpu"},           "Motorola 68k CPU");
    root.add({"ciaa"},          "Complex Interface Adapter A");
    root.add({"ciab"},          "Complex Interface Adapter B");
    root.add({"agnus"},         "Custom Chipset");
    root.add({"blitter"},       "Coprocessor");
    root.add({"copper"},        "Coprocessor");
    root.add({"paula"},         "Custom Chipset");
    root.add({"denise"},        "Custom Chipset");
    root.add({"rtc"},           "Real-time clock");
    root.add({"zorro"},         "Expansion boards");
    root.add({"controlport"},   "Control ports");
    root.add({"serial"},        "Serial port");

    root.newGroup("Debugging peripherals");

    root.add({"keyboard"},      "Keyboard");
    root.add({"mouse"},         "Mouse");
    root.add({"joystick"},      "Joystick");
    root.add({"df0"},           "Floppy drive 0");
    root.add({"df1"},           "Floppy drive 1");
    root.add({"df2"},           "Floppy drive 2");
    root.add({"df3"},           "Floppy drive 3");
    root.add({"hd0"},           "Hard drive 0");
    root.add({"hd1"},           "Hard drive 1");
    root.add({"hd2"},           "Hard drive 2");
    root.add({"hd3"},           "Hard drive 3");

    root.newGroup("Miscellaneous");

    root.add({"os"},            "AmigaOS debugger");

    
    //
    // Breakpoints
    //

    root.newGroup("");

    root.add({"break", ""},
             "Lists all breakpoints",
             &RetroShell::exec <Token::bp>);

    root.add({"break", "at"}, { Arg::address },
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::bp, Token::at>);

    root.add({"break", "delete"}, { Arg::address },
             "Deletes a breakpoint",
             &RetroShell::exec <Token::bp, Token::del>);

    root.add({"break", "enable"}, { Arg::address },
             "Enables a breakpoint",
             &RetroShell::exec <Token::bp, Token::enable>);

    root.add({"break", "disable"}, { Arg::address },
             "Disables a breakpoint",
             &RetroShell::exec <Token::bp, Token::disable>);

    root.add({"break", "ignore"}, { Arg::address, Arg::value },
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::bp, Token::ignore>);

    //
    // Watchpoints
    //

    root.newGroup("");

    root.add({"watch", ""},
             "Lists all watchpoints",
             &RetroShell::exec <Token::wp>);

    root.add({"watch", "at"}, { Arg::address },
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::wp, Token::at>);

    root.add({"watch", "delete"}, { Arg::address },
             "Deletes a watchpoint",
             &RetroShell::exec <Token::wp, Token::del>);

    root.add({"watch", "enable"}, { Arg::address },
             "Enables a watchpoint",
             &RetroShell::exec <Token::wp, Token::enable>);

    root.add({"watch", "disable"}, { Arg::address },
             "Disables a watchpoint",
             &RetroShell::exec <Token::wp, Token::disable>);

    root.add({"watch", "ignore"}, { Arg::address, Arg::value },
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::wp, Token::ignore>);

    //
    // Catchpoints
    //

    root.newGroup("");

    root.add({"catch", ""},
             "Lists all catchpoints",
             &RetroShell::exec <Token::cp>);

    root.add({"catch", "vector"}, { Arg::value },
             "Catches an exception vector",
             &RetroShell::exec <Token::cp, Token::vector>);

    root.add({"catch", "interrupt"}, { Arg::value },
             "Catches an interrupt",
             &RetroShell::exec <Token::cp, Token::interrupt>);

    root.add({"catch", "trap"}, { Arg::value },
             "Catches a trap instruction",
             &RetroShell::exec <Token::cp, Token::trap>);

    root.add({"catch", "delete"}, { Arg::value },
             "Deletes a catchpoint",
             &RetroShell::exec <Token::cp, Token::del>);

    root.add({"catch", "enable"}, { Arg::value },
             "Enables a catchpoint",
             &RetroShell::exec <Token::cp, Token::enable>);

    root.add({"catch", "disable"}, { Arg::value },
             "Disables a catchpoint",
             &RetroShell::exec <Token::cp, Token::disable>);

    root.add({"catch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a catchpoint a certain number of times",
             &RetroShell::exec <Token::cp, Token::ignore>);


    //
    // Copper breakpoints
    //

    root.add({"cbreak", ""},
             "Lists all breakpoints",
             &RetroShell::exec <Token::cbp>);

    root.add({"cbreak", "at"}, { Arg::value },
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::cbp, Token::at>);

    root.add({"cbreak", "delete"}, { Arg::value },
             "Deletes a breakpoint",
             &RetroShell::exec <Token::cbp, Token::del>);

    root.add({"cbreak", "enable"}, { Arg::value },
             "Enables a breakpoint",
             &RetroShell::exec <Token::cbp, Token::enable>);

    root.add({"cbreak", "disable"}, { Arg::value },
             "Disables a breakpoint",
             &RetroShell::exec <Token::cbp, Token::disable>);

    root.add({"cbreak", "ignore"}, { Arg::value, Arg::value },
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::cbp, Token::ignore>);


    //
    // Copper watchpoints
    //

    root.add({"cwatch", ""},
             "Lists all watchpoints",
             &RetroShell::exec <Token::cwp>);

    root.add({"cwatch", "at"}, { Arg::value },
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::cwp, Token::at>);

    root.add({"cwatch", "delete"}, { Arg::value },
             "Deletes a watchpoint",
             &RetroShell::exec <Token::cwp, Token::del>);

    root.add({"cwatch", "enable"}, { Arg::value },
             "Enables a watchpoint",
             &RetroShell::exec <Token::cwp, Token::enable>);

    root.add({"cwatch", "disable"}, { Arg::value },
             "Disables a watchpoint",
             &RetroShell::exec <Token::cwp, Token::disable>);

    root.add({"cwatch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::cwp, Token::ignore>);


    //
    // Amiga
    //

    root.newGroup("");

    root.add({"amiga", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::amiga>);

    root.add({"amiga", "host"},
             "Displays information about the host machine",
             &RetroShell::exec <Token::amiga, Token::host>);

    root.add({"amiga", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::amiga, Token::debug>);


    //
    // Memory
    //

    root.add({"memory", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::memory>);

    root.add({"memory", "dump"}, { Arg::address },
             "Generates a memory hexdump",
             &RetroShell::exec <Token::memory, Token::memdump>);

    root.add({"memory", "banks"},
             "Dumps the memory bank map",
             &RetroShell::exec <Token::memory, Token::bankmap>);

    root.add({"memory", "checksum"},
             "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::checksums>);


    //
    // CPU
    //

    root.add({"cpu", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::cpu>);

    root.add({"cpu", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::cpu, Token::debug>);

    root.add({"cpu", "vectors"},
             "Dumps the vector table",
             &RetroShell::exec <Token::cpu, Token::vectors>);


    //
    // CIA
    //

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({cia, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::cia>, i);

        root.add({cia, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::cia, Token::debug>, i);

        root.add({cia, "tod"},
                 "Displays the state of the 24-bit counter",
                 &RetroShell::exec <Token::cia, Token::tod>, i);
    }


    //
    // Agnus
    //

    root.add({"agnus", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::agnus>);

    root.add({"agnus", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::agnus, Token::debug>);

    root.add({"agnus", "beam"},
             "Displays the current beam position",
             &RetroShell::exec <Token::agnus, Token::beam>);

    root.add({"agnus", "dma"},
             "Prints all scheduled DMA events",
             &RetroShell::exec <Token::agnus, Token::dma>);

    root.add({"agnus", "events"},
             "Inspects the event scheduler",
             &RetroShell::exec <Token::agnus, Token::events>);


    //
    // Blitter
    //

    root.add({"blitter", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::blitter>);

    root.add({"blitter", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::blitter, Token::debug>);


    //
    // Copper
    //

    root.add({"copper", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::copper>);

    root.add({"copper", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::copper, Token::debug>);

    root.add({"copper", "list"}, { Arg::value },
             "Prints the Copper list",
             &RetroShell::exec <Token::copper, Token::list>);


    //
    // Paula
    //

    root.add({"paula", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula>);

    root.add({"paula", "audio"},
             "Audio unit");

    root.add({"paula", "dc"},
             "Disk controller");

    root.add({"paula", "uart"},
             "Universal Asynchronous Receiver Transmitter");

    root.add({"paula", "audio", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::audio>);

    root.add({"paula", "audio", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::paula, Token::audio, Token::debug>);

    root.add({"paula", "dc", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::dc>);

    root.add({"paula", "dc", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::paula, Token::dc, Token::debug>);

    root.add({"paula", "uart", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::uart>);


    //
    // Denise
    //

    root.add({"denise", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::denise>);

    root.add({"denise", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::denise, Token::debug>);



    //
    // RTC
    //

    root.add({"rtc", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::rtc>);

    root.add({"rtc", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::rtc, Token::debug>);


    //
    // Zorro boards
    //

    root.add({"zorro", ""},
             "Lists all connected boards",
             &RetroShell::exec <Token::zorro, Token::list>);

    root.add({"zorro", "inspect"}, { Arg::value },
             "Inspects a specific Zorro board",
             &RetroShell::exec <Token::zorro, Token::inspect>);


    //
    // Control ports
    //

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"controlport", nr},
                 "Control port " + nr);

        root.add({"controlport", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::controlport>, i);

        root.add({"controlport", nr, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::controlport, Token::debug>, i);
    }


    //
    // Serial port
    //

    root.add({"serial", ""},
             "Displays the internal state",
             &RetroShell::exec <Token::serial>);


    //
    // Keyboard, Mice, Joystick
    //

    root.add({"keyboard", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::keyboard>);

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"mouse", nr},
                 "Mouse in port " + nr);

        root.add({"mouse", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::mouse>, i);

        root.add({"mouse", nr, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::mouse, Token::debug>, i);

        root.add({"joystick", nr},
                 "Joystick in port " + nr);

        root.add({"joystick", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::joystick>, i);
    }

    //
    // Df0, Df1, Df2, Df3
    //

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({df, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::dfn>, i);

        root.add({df, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::dfn, Token::debug>, i);

        root.add({df, "disk"},
                 "Inspects the inserted disk",
                 &RetroShell::exec <Token::dfn, Token::disk>, i);
    }

    //
    // Hd0, Hd1, Hd2, Hd3
    //

    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);

        root.add({hd, ""},
                 "Inspects the internal state");

        root.add({hd, "drive"},
                 "Displays hard drive parameters",
                 &RetroShell::exec <Token::hdn, Token::drive>, i);

        root.add({hd, "volumes"},
                 "Displays summarized volume information",
                 &RetroShell::exec <Token::hdn, Token::volumes>, i);

        root.add({hd, "partitions"},
                 "Displays information about all partitions",
                 &RetroShell::exec <Token::hdn, Token::partition>, i);

        root.add({hd, "debug"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::hdn, Token::debug>, i);
    }


    //
    // OSDebugger
    //

    root.add({"os", "info"},
             "Displays basic system information",
             &RetroShell::exec <Token::os, Token::info>);

    root.add({"os", "execbase"},
             "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>);

    root.add({"os", "interrupts"},
             "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>);

    root.add({"os", "libraries"}, { }, {"<library>"},
             "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>);

    root.add({"os", "devices"}, { }, {"<device>"},
             "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>);

    root.add({"os", "resources"}, { }, {"<resource>"},
             "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>);

    root.add({"os", "tasks"}, { }, {"<task>"},
             "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>);

    root.add({"os", "processes"}, { }, {"<process>"},
             "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>);

    root.add({"os", "catch"}, {"<task>"},
             "Pauses emulation on task launch",
             &RetroShell::exec <Token::os, Token::cp>);

    root.add({"os", "set"},
             "Configures the component");

    root.add({"os", "set", "diagboard" }, { Arg::boolean },
             "Attaches or detaches the debug expansion board",
             &RetroShell::exec <Token::os, Token::set, Token::diagboard>);
}

}
