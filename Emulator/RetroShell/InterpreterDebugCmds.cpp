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
// #include "RetroShell.h"
#include "Amiga.h"

namespace vamiga {

void
Interpreter::initDebugShell(Command &root)
{
    initCommons(root);

    //
    // Top-level commands
    //

    root.newGroup("Controlling the instruction stream");

    root.oldadd({"pause"},
             "Pauses emulation",
             &RetroShell::exec <Token::pause>);

    root.oldadd({"continue"},
             "Continues emulation",
             &RetroShell::exec <Token::run>);

    root.oldadd({"step"},
             "Steps into the next instruction",
             &RetroShell::exec <Token::step>);

    root.oldadd({"next"},
             "Steps over the next instruction",
             &RetroShell::exec <Token::next>);

    root.oldadd({"goto"}, { Arg::address },
             "Redirects the program counter",
             &RetroShell::exec <Token::jump>);

    root.oldadd({"disassemble"}, { Arg::address },
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

    root.oldadd({"break", ""},
             "Lists all breakpoints",
             &RetroShell::exec <Token::bp>);

    root.oldadd({"break", "at"}, { Arg::address },
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::bp, Token::at>);

    root.oldadd({"break", "delete"}, { Arg::address },
             "Deletes a breakpoint",
             &RetroShell::exec <Token::bp, Token::del>);

    root.oldadd({"break", "enable"}, { Arg::address },
             "Enables a breakpoint",
             &RetroShell::exec <Token::bp, Token::enable>);

    root.oldadd({"break", "disable"}, { Arg::address },
             "Disables a breakpoint",
             &RetroShell::exec <Token::bp, Token::disable>);

    root.oldadd({"break", "ignore"}, { Arg::address, Arg::value },
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::bp, Token::ignore>);

    //
    // Watchpoints
    //

    root.newGroup("");

    root.oldadd({"watch", ""},
             "Lists all watchpoints",
             &RetroShell::exec <Token::wp>);

    root.oldadd({"watch", "at"}, { Arg::address },
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::wp, Token::at>);

    root.oldadd({"watch", "delete"}, { Arg::address },
             "Deletes a watchpoint",
             &RetroShell::exec <Token::wp, Token::del>);

    root.oldadd({"watch", "enable"}, { Arg::address },
             "Enables a watchpoint",
             &RetroShell::exec <Token::wp, Token::enable>);

    root.oldadd({"watch", "disable"}, { Arg::address },
             "Disables a watchpoint",
             &RetroShell::exec <Token::wp, Token::disable>);

    root.oldadd({"watch", "ignore"}, { Arg::address, Arg::value },
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::wp, Token::ignore>);

    //
    // Catchpoints
    //

    root.newGroup("");

    root.oldadd({"catch", ""},
             "Lists all catchpoints",
             &RetroShell::exec <Token::cp>);

    root.oldadd({"catch", "vector"}, { Arg::value },
             "Catches an exception vector",
             &RetroShell::exec <Token::cp, Token::vector>);

    root.oldadd({"catch", "interrupt"}, { Arg::value },
             "Catches an interrupt",
             &RetroShell::exec <Token::cp, Token::interrupt>);

    root.oldadd({"catch", "trap"}, { Arg::value },
             "Catches a trap instruction",
             &RetroShell::exec <Token::cp, Token::trap>);

    root.oldadd({"catch", "delete"}, { Arg::value },
             "Deletes a catchpoint",
             &RetroShell::exec <Token::cp, Token::del>);

    root.oldadd({"catch", "enable"}, { Arg::value },
             "Enables a catchpoint",
             &RetroShell::exec <Token::cp, Token::enable>);

    root.oldadd({"catch", "disable"}, { Arg::value },
             "Disables a catchpoint",
             &RetroShell::exec <Token::cp, Token::disable>);

    root.oldadd({"catch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a catchpoint a certain number of times",
             &RetroShell::exec <Token::cp, Token::ignore>);


    //
    // Copper breakpoints
    //

    root.oldadd({"cbreak", ""},
             "Lists all breakpoints",
             &RetroShell::exec <Token::cbp>);

    root.oldadd({"cbreak", "at"}, { Arg::value },
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::cbp, Token::at>);

    root.oldadd({"cbreak", "delete"}, { Arg::value },
             "Deletes a breakpoint",
             &RetroShell::exec <Token::cbp, Token::del>);

    root.oldadd({"cbreak", "enable"}, { Arg::value },
             "Enables a breakpoint",
             &RetroShell::exec <Token::cbp, Token::enable>);

    root.oldadd({"cbreak", "disable"}, { Arg::value },
             "Disables a breakpoint",
             &RetroShell::exec <Token::cbp, Token::disable>);

    root.oldadd({"cbreak", "ignore"}, { Arg::value, Arg::value },
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::cbp, Token::ignore>);


    //
    // Copper watchpoints
    //

    root.oldadd({"cwatch", ""},
             "Lists all watchpoints",
             &RetroShell::exec <Token::cwp>);

    root.oldadd({"cwatch", "at"}, { Arg::value },
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::cwp, Token::at>);

    root.oldadd({"cwatch", "delete"}, { Arg::value },
             "Deletes a watchpoint",
             &RetroShell::exec <Token::cwp, Token::del>);

    root.oldadd({"cwatch", "enable"}, { Arg::value },
             "Enables a watchpoint",
             &RetroShell::exec <Token::cwp, Token::enable>);

    root.oldadd({"cwatch", "disable"}, { Arg::value },
             "Disables a watchpoint",
             &RetroShell::exec <Token::cwp, Token::disable>);

    root.oldadd({"cwatch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::cwp, Token::ignore>);


    //
    // Amiga
    //

    root.newGroup("");

    root.oldadd({"amiga", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::amiga>);

    root.oldadd({"amiga", "host"},
             "Displays information about the host machine",
             &RetroShell::exec <Token::amiga, Token::host>);

    root.oldadd({"amiga", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::amiga, Token::debug>);


    //
    // Memory
    //

    root.oldadd({"memory", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::memory>);

    root.oldadd({"memory", "dump"}, { Arg::address },
             "Generates a memory hexdump",
             &RetroShell::exec <Token::memory, Token::memdump>);

    root.oldadd({"memory", "banks"},
             "Dumps the memory bank map",
             &RetroShell::exec <Token::memory, Token::bankmap>);

    root.oldadd({"memory", "checksum"},
             "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::checksums>);

    root.oldadd({"memory", "write"}, { Arg::value, Arg::value },
             "Writes a word into memory",
             &RetroShell::exec <Token::memory, Token::write>);


    //
    // CPU
    //

    root.oldadd({"cpu", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::cpu>);

    root.oldadd({"cpu", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::cpu, Token::debug>);

    root.oldadd({"cpu", "vectors"},
             "Dumps the vector table",
             &RetroShell::exec <Token::cpu, Token::vectors>);


    //
    // CIA
    //

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.oldadd({cia, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::cia>, i);

        root.oldadd({cia, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::cia, Token::debug>, i);

        root.oldadd({cia, "tod"},
                 "Displays the state of the 24-bit counter",
                 &RetroShell::exec <Token::cia, Token::tod>, i);
    }


    //
    // Agnus
    //

    root.oldadd({"agnus", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::agnus>);

    root.oldadd({"agnus", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::agnus, Token::debug>);

    root.oldadd({"agnus", "beam"},
             "Displays the current beam position",
             &RetroShell::exec <Token::agnus, Token::beam>);

    root.oldadd({"agnus", "dma"},
             "Prints all scheduled DMA events",
             &RetroShell::exec <Token::agnus, Token::dma>);

    root.oldadd({"agnus", "events"},
             "Inspects the event scheduler",
             &RetroShell::exec <Token::agnus, Token::events>);


    //
    // Blitter
    //

    root.oldadd({"blitter", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::blitter>);

    root.oldadd({"blitter", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::blitter, Token::debug>);


    //
    // Copper
    //

    root.oldadd({"copper", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::copper>);

    root.oldadd({"copper", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::copper, Token::debug>);

    root.oldadd({"copper", "list"}, { Arg::value },
             "Prints the Copper list",
             &RetroShell::exec <Token::copper, Token::list>);


    //
    // Paula
    //

    root.oldadd({"paula", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula>);

    root.add({"paula", "audio"},
             "Audio unit");

    root.add({"paula", "dc"},
             "Disk controller");

    root.add({"paula", "uart"},
             "Universal Asynchronous Receiver Transmitter");

    root.oldadd({"paula", "audio", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::audio>);

    root.oldadd({"paula", "audio", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::paula, Token::audio, Token::debug>);

    root.oldadd({"paula", "dc", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::dc>);

    root.oldadd({"paula", "dc", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::paula, Token::dc, Token::debug>);

    root.oldadd({"paula", "uart", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::paula, Token::uart>);


    //
    // Denise
    //

    root.oldadd({"denise", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::denise>);

    root.oldadd({"denise", "debug"},
             "Displays additional debug information",
             &RetroShell::exec <Token::denise, Token::debug>);



    //
    // RTC
    //

    /*
    root.oldadd({"rtc", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::rtc>);
     */
    root.add({"rtc", ""},
             "Inspects the internal state",
             [this](Arguments& args, long value) {

        retroShell.dumpInspection(rtc);
    });

    /*
    root.add({"rtc", "debug"},
             "Displays additional debug information",
     &RetroShell::exec <Token::rtc, Token::debug>);
     */
    root.add({"rtc", "debug"},
             "Displays additional debug information",
             [this](Arguments& args, long value) {

        retroShell.dumpDebug(rtc);
    });

    //
    // Zorro boards
    //

    root.oldadd({"zorro", ""},
             "Lists all connected boards",
             &RetroShell::exec <Token::zorro, Token::list>);

    root.oldadd({"zorro", "inspect"}, { Arg::value },
             "Inspects a specific Zorro board",
             &RetroShell::exec <Token::zorro, Token::inspect>);


    //
    // Control ports
    //

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"controlport", nr},
                 "Control port " + nr);

        root.oldadd({"controlport", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::controlport>, i);

        root.oldadd({"controlport", nr, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::controlport, Token::debug>, i);
    }


    //
    // Serial port
    //

    root.oldadd({"serial", ""},
             "Displays the internal state",
             &RetroShell::exec <Token::serial>);


    //
    // Keyboard, Mice, Joystick
    //

    root.oldadd({"keyboard", ""},
             "Inspects the internal state",
             &RetroShell::exec <Token::keyboard>);

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"mouse", nr},
                 "Mouse in port " + nr);

        root.oldadd({"mouse", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::mouse>, i);

        root.oldadd({"mouse", nr, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::mouse, Token::debug>, i);

        root.add({"joystick", nr},
                 "Joystick in port " + nr);

        root.oldadd({"joystick", nr, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::joystick>, i);
    }

    //
    // Df0, Df1, Df2, Df3
    //

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.oldadd({df, ""},
                 "Inspects the internal state",
                 &RetroShell::exec <Token::dfn>, i);

        root.oldadd({df, "debug"},
                 "Displays additional debug information",
                 &RetroShell::exec <Token::dfn, Token::debug>, i);

        root.oldadd({df, "disk"},
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

        root.oldadd({hd, "drive"},
                 "Displays hard drive parameters",
                 &RetroShell::exec <Token::hdn, Token::drive>, i);

        root.oldadd({hd, "volumes"},
                 "Displays summarized volume information",
                 &RetroShell::exec <Token::hdn, Token::volumes>, i);

        root.oldadd({hd, "partitions"},
                 "Displays information about all partitions",
                 &RetroShell::exec <Token::hdn, Token::partition>, i);

        root.oldadd({hd, "debug"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::hdn, Token::debug>, i);
    }


    //
    // OSDebugger
    //

    root.oldadd({"os", "info"},
             "Displays basic system information",
             &RetroShell::exec <Token::os, Token::info>);

    root.oldadd({"os", "execbase"},
             "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>);

    root.oldadd({"os", "interrupts"},
             "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>);

    root.oldadd({"os", "libraries"}, { }, {"<library>"},
             "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>);

    root.oldadd({"os", "devices"}, { }, {"<device>"},
             "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>);

    root.oldadd({"os", "resources"}, { }, {"<resource>"},
             "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>);

    root.oldadd({"os", "tasks"}, { }, {"<task>"},
             "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>);

    root.oldadd({"os", "processes"}, { }, {"<process>"},
             "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>);

    root.oldadd({"os", "catch"}, {"<task>"},
             "Pauses emulation on task launch",
             &RetroShell::exec <Token::os, Token::cp>);

    root.add({"os", "set"},
             "Configures the component");

    root.oldadd({"os", "set", "diagboard" }, { Arg::boolean },
             "Attaches or detaches the debug expansion board",
             &RetroShell::exec <Token::os, Token::set, Token::diagboard>);
}

}
