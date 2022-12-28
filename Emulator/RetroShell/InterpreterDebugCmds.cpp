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

    root.add({"pause"},
             "Pauses emulation",
             [this](Arguments& argv, long value) {

        amiga.pause();
    });

    root.add({"continue"},
             "Continues emulation",
                [this](Arguments& argv, long value) {

        amiga.run();
    });

    root.add({"step"},
             "Steps into the next instruction",
                [this](Arguments& argv, long value) {

        amiga.stepInto();
    });

    root.add({"next"},
             "Steps over the next instruction",
                [this](Arguments& argv, long value) {

        amiga.stepOver();
    });

    root.add({"goto"}, { Arg::address },
             "Redirects the program counter",
             [this](Arguments& argv, long value) {

        amiga.cpu.jump((u32)util::parseNum(argv.front()));
    });

    root.add({"disassemble"}, { }, { Arg::address },
             "Runs disassembler",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        auto addr = argv.empty() ? cpu.getPC0() : u32(util::parseNum(argv.front()));
        cpu.disassembleRange(ss, addr, 16);

        retroShell << '\n' << ss << '\n';
    });


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
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Breakpoints);
    });

    root.add({"break", "at"}, { Arg::address },
             "Sets a breakpoint at the specified address",
             [this](Arguments& argv, long value) {

        cpu.setBreakpoint(u32(util::parseNum(argv.front())));
    });

    root.add({"break", "delete"}, { Arg::address },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"break", "enable"}, { Arg::address },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.enableBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"break", "disable"}, { Arg::address },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.disableBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"break", "ignore"}, { Arg::address, Arg::value },
             "Ignores a breakpoint a certain number of times",
             [this](Arguments& argv, long value) {

        cpu.ignoreBreakpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });

    
    //
    // Watchpoints
    //

    root.newGroup("");

    root.add({"watch", ""},
             "Lists all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Watchpoints);
    });

    root.add({"watch", "at"}, { Arg::address },
             "Sets a watchpoint at the specified address",
             [this](Arguments& argv, long value) {

        cpu.setWatchpoint(u32(util::parseNum(argv.front())));
    });

    root.add({"watch", "delete"}, { Arg::address },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"watch", "enable"}, { Arg::address },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"watch", "disable"}, { Arg::address },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"watch", "ignore"}, { Arg::address, Arg::value },
             "Ignores a watchpoint a certain number of times",
             [this](Arguments& argv, long value) {

        cpu.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });


    //
    // Catchpoints
    //

    root.newGroup("");

    root.add({"catch", ""},
             "Lists all catchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Catchpoints);
    });

    root.add({"catch", "vector"}, { Arg::value },
             "Catches an exception vector",
             [this](Arguments& argv, long value) {

        auto nr = util::parseNum(argv.front());
        if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
        cpu.setCatchpoint(u8(nr));
    });

    root.add({"catch", "interrupt"}, { Arg::value },
             "Catches an interrupt",
             [this](Arguments& argv, long value) {

        auto nr = util::parseNum(argv.front());
        if (nr < 1 || nr > 7) throw VAError(ERROR_OPT_INVARG, "1...7");
        cpu.setCatchpoint(u8(nr + 24));
    });

    root.add({"catch", "trap"}, { Arg::value },
             "Catches a trap instruction",
             [this](Arguments& argv, long value) {

        auto nr = util::parseNum(argv.front());
        if (nr < 0 || nr > 15) throw VAError(ERROR_OPT_INVARG, "0...15");
        cpu.setCatchpoint(u8(nr + 32));
    });

    root.add({"catch", "delete"}, { Arg::value },
             "Deletes a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteCatchpoint(util::parseNum(argv.front()));
    });

    root.add({"catch", "enable"}, { Arg::value },
             "Enables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableCatchpoint(util::parseNum(argv.front()));
    });

    root.add({"catch", "disable"}, { Arg::value },
             "Disables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableCatchpoint(util::parseNum(argv.front()));
    });

    root.add({"catch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a catchpoint a certain number of times",
             [this](Arguments& argv, long value) {

        cpu.ignoreCatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });


    //
    // Copper breakpoints
    //

    root.add({"cbreak", ""},
             "Lists all breakpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper.debugger, Category::Breakpoints);
    });

    root.add({"cbreak", "at"}, { Arg::value },
             "Sets a breakpoint at the specified address",
             [this](Arguments& argv, long value) {

        copper.debugger.setBreakpoint(u32(util::parseNum(argv.front())));
    });

    root.add({"cbreak", "delete"}, { Arg::value },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"cbreak", "enable"}, { Arg::value },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"cbreak", "disable"}, { Arg::value },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableBreakpoint(util::parseNum(argv.front()));
    });

    root.add({"cbreak", "ignore"}, { Arg::value, Arg::value },
             "Ignores a breakpoint a certain number of times",
             [this](Arguments& argv, long value) {

        copper.debugger.ignoreBreakpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });


    //
    // Copper watchpoints
    //

    root.add({"cwatch", ""},
             "Lists all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper.debugger, Category::Watchpoints);
    });

    root.add({"cwatch", "at"}, { Arg::value },
             "Sets a watchpoint at the specified address",
             [this](Arguments& argv, long value) {

        copper.debugger.setWatchpoint(u32(util::parseNum(argv.front())));
    });

    root.add({"cwatch", "delete"}, { Arg::value },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"cwatch", "enable"}, { Arg::value },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"cwatch", "disable"}, { Arg::value },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableWatchpoint(util::parseNum(argv.front()));
    });

    root.add({"cwatch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a watchpoint a certain number of times",
             [this](Arguments& argv, long value) {

        copper.debugger.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });


    //
    // Amiga
    //

    root.newGroup("");

    root.add({"amiga", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, Category::Inspection);
    });

    root.add({"amiga", "host"},
             "Displays information about the host machine",
             [this](Arguments& argv, long value) {

        retroShell.dump(host, Category::Inspection);
    });

    root.add({"amiga", "debug"},
             "Displays additional debug information",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, Category::Debug);
    });


    //
    // Memory
    //

    root.add({"memory", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, Category::BankMap);
    });

    root.add({"memory", "dump"}, { Arg::address },
             "Generates a memory hexdump",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        mem.memDump<ACCESSOR_CPU>(ss, u32(util::parseNum(argv.front())));
        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory", "banks"},
             "Dumps the memory bank map",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, Category::BankMap);
    });

    root.add({"memory", "checksum"},
             "Computes memory checksums",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.mem, Category::Checksums);
    });

    root.add({"memory", "write"}, { Arg::value, Arg::value },
             "Writes a word into memory",
             [this](Arguments& argv, long value) {

        auto addr = (u32)util::parseNum(argv[0]);
        auto val = (u16)util::parseNum(argv[1]);
        mem.patch(addr, val);
    });


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

    root.add({"rtc", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dumpInspection(rtc);
    });

    root.add({"rtc", "debug"},
             "Displays additional debug information",
             [this](Arguments& argv, long value) {

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
