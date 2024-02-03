// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "Amiga.h"

namespace vamiga {

void
Interpreter::initDebugShell(Command &root)
{
    //
    // Top-level commands
    //

    initCommons(root);

    root.setGroup("Monitor commands");

    root.add({"goto"}, { }, { Arg::value },
             "Goto address",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        if (argv.empty()) {
            amiga.run();
        } else {
            debugger.jump(u32(parseNum(argv)));
        }
    });

    root.add({"step"},
             "Step into the next instruction",
             [this](Arguments& argv, long value) {

        debugger.stepInto();
    });

    root.add({"next"},
             "Step over the next instruction",
             [this](Arguments& argv, long value) {

        debugger.stepOver();
    });

    root.add({"disassemble"}, { }, { Arg::address },
             "Disassemble instructions",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        auto addr = argv.empty() ? cpu.getPC0() : u32(parseNum(argv));
        cpu.disassembleRange(ss, addr, 16);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"ascii"}, { }, { Arg::address },
             "Dump memory in ASCII",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.ascDump<ACCESSOR_CPU>(ss, 16) :
        debugger.ascDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory"}, { }, { Arg::address },
             "Dump memory",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.memDump<ACCESSOR_CPU>(ss, 16, 1) :
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16, 1);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory.b"}, { }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.memDump<ACCESSOR_CPU>(ss, 16, 1) :
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16, 1);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory.w"}, { }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.memDump<ACCESSOR_CPU>(ss, 16, 2) :
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16, 2);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory.l"}, { }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.memDump<ACCESSOR_CPU>(ss, 16, 4) :
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16, 4);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"write"}, { Arg::address, Arg::value }, { Arg::count },
             "Modify memory",
             [this](Arguments& argv, long value) {

        execWrite(argv, 2);
    });

    root.add({"write.b"}, { Arg::address, Arg::value }, { Arg::count }, "",
             [this](Arguments& argv, long value) {

        execWrite(argv, 1);
    });

    root.add({"write.w"}, { Arg::address, Arg::value }, { Arg::count }, "",
             [this](Arguments& argv, long value) {

        execWrite(argv, 2);
    });

    root.add({"write.l"}, { Arg::address, Arg::value }, { Arg::count }, "",
             [this](Arguments& argv, long value) {

        execWrite(argv, 4);
    });

    root.add({"find"}, { Arg::sequence }, { Arg::address },
             "Find a byte sequence in memory",
             [this](Arguments& argv, long value) {

        execFind(argv, 1);
    });

    root.add({"find.b"}, { Arg::sequence }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        execFind(argv, 1);
    });

    root.add({"find.w"}, { Arg::sequence }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        execFind(argv, 2);
    });

    root.add({"find.l"}, { Arg::sequence }, { Arg::address }, "",
             [this](Arguments& argv, long value) {

        execFind(argv, 4);
    });

    root.add({"register"}, { ChipsetRegEnum::argList() }, { Arg::value },
             "Reads or modifies a custom chipset register",
             [this](Arguments& argv, long value) {

        auto reg = parseEnum<ChipsetRegEnum>(argv);

        if (argv.size() == 1) {

            std::stringstream ss;
            ss << ChipsetRegEnum::key(reg) << " = " << util::hex(debugger.readCs(reg));
            retroShell << ss;

        } else {

            debugger.writeCs(reg, u16(parseNum(argv, 1)));
        }
    });

    root.add({"inspect"},
             "Inspect component");

    root.add({"os"},
             "Runs the OS debugger");

    //
    // Second-level commands
    //

    root.setGroup("Inspecting components");

    root.add({"inspect", "amiga"},         "Main computer");
    root.add({"inspect", "memory"},        "RAM and ROM");
    root.add({"inspect", "cpu"},           "Motorola 68k CPU");
    root.add({"inspect", "ciaa"},          "Complex Interface Adapter A");
    root.add({"inspect", "ciab"},          "Complex Interface Adapter B");
    root.add({"inspect", "agnus"},         "Custom Chipset");
    root.add({"inspect", "blitter"},       "Coprocessor");
    root.add({"inspect", "copper"},        "Coprocessor");
    root.add({"inspect", "paula"},         "Custom Chipset");
    root.add({"inspect", "denise"},        "Custom Chipset");
    root.add({"inspect", "rtc"},           "Real-time clock");
    root.add({"inspect", "zorro"},         "Expansion boards");
    root.add({"inspect", "controlport"},   "Control ports");
    root.add({"inspect", "serial"},        "Serial port");

    root.setGroup("Inspecting peripherals");

    root.add({"inspect", "keyboard"},      "Keyboard");
    root.add({"inspect", "mouse"},         "Mouse");
    root.add({"inspect", "joystick"},      "Joystick");
    root.add({"inspect", "df0"},           "Floppy drive 0");
    root.add({"inspect", "df1"},           "Floppy drive 1");
    root.add({"inspect", "df2"},           "Floppy drive 2");
    root.add({"inspect", "df3"},           "Floppy drive 3");
    root.add({"inspect", "hd0"},           "Hard drive 0");
    root.add({"inspect", "hd1"},           "Hard drive 1");
    root.add({"inspect", "hd2"},           "Hard drive 2");
    root.add({"inspect", "hd3"},           "Hard drive 3");

    root.setGroup("Miscellaneous");

    root.add({"inspect", "host"},          "Host machine");
    root.add({"inspect", "server"},        "Remote server");

    //
    // Third-level commands
    //

    root.setGroup("");

    root.add({"inspect", "cpu", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "cpu", "vectors"},
             "Dumps the vector table",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Vectors);
    });

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({"inspect", cia, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa, { Category::Config, Category::State, Category::Registers } );
            } else {
                retroShell.dump(ciab, { Category::Config, Category::State, Category::Registers } );
            }
        }, i);

        root.add({"inspect", cia, "tod"},
                 "Displays the state of the 24-bit counter",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa.tod, Category::State );
            } else {
                retroShell.dump(ciab.tod, Category::State );
            }
        }, i);
    }

    root.add({"inspect", "agnus", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(agnus, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "agnus", "beam"},
             "Displays the current beam position",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Beam);
    });

    root.add({"inspect", "agnus", "dma"},
             "Prints all scheduled DMA events",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Dma);
    });

    root.add({"inspect", "agnus", "sequencer"},
             "Inspects the sequencer logic",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus.sequencer, { Category::State, Category::Registers, Category::Signals } );
    });

    root.add({"inspect", "agnus", "events"},
             "Inspects the event scheduler",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Events);
    });

    root.add({"inspect", "blitter", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(blitter, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "copper", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "copper", "list"}, { Arg::value },
             "Prints the Copper list",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv);

        switch (nr) {

            case 1: retroShell.dump(amiga.agnus.copper, Category::List1); break;
            case 2: retroShell.dump(amiga.agnus.copper, Category::List2); break;

            default:
                throw VAError(ERROR_OPT_INVARG, "1 or 2");
        }
    });

    root.add({"inspect", "paula", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, Category::Registers);
    });

    root.add({"inspect", "paula", "audio"},
             "Audio unit");

    root.add({"inspect", "paula", "dc"},
             "Disk controller");

    root.add({"inspect", "paula", "uart"},
             "Universal Asynchronous Receiver Transmitter");

    root.add({"inspect", "paula", "audio", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "paula", "audio", "filter"},
             "Inspects the internal filter state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula.muxer.filter, { Category::Config, Category::State } );
    });

    root.add({"inspect", "paula", "dc", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(diskController, { Category::Config, Category::State } );
    });

    root.add({"inspect", "paula", "uart", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(uart, Category::State);
    });

    root.add({"inspect", "denise", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(denise, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "rtc", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(rtc, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"inspect", "zorro", ""},
             "Lists all connected boards",
             [this](Arguments& argv, long value) {

        retroShell.dump(zorro, Category::Slots);
    });

    root.add({"inspect", "zorro", "inspect"}, { Arg::value },
             "Inspects a specific Zorro board",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv);

        if (auto board = zorro.getBoard(nr); board != nullptr) {

            retroShell.dump(*board, { Category::Properties, Category::State, Category::Stats } );
        }
    });

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"inspect", "controlport", nr},
                 "Control port " + nr);

        root.add({"inspect", "controlport", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1, Category::State);
            if (value == 2) retroShell.dump(controlPort2, Category::State);

        }, i);
    }

    root.add({"inspect", "serial", ""},
             "Displays the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(serialPort, { Category::Config, Category::State } );
    });

    root.add({"inspect", "keyboard", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(keyboard, { Category::Config, Category::State } );
    });

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"inspect", "mouse", nr},
                 "Mouse in port " + nr);

        root.add({"inspect", "mouse", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.mouse, { Category::Config, Category::State } );
            if (value == 2) retroShell.dump(controlPort2.mouse, { Category::Config, Category::State } );

        }, i);

        root.add({"inspect", "joystick", nr},
                 "Joystick in port " + nr);

        root.add({"inspect", "joystick", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.joystick, Category::State);
            if (value == 2) retroShell.dump(controlPort2.joystick, Category::State);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({"inspect", df, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], { Category::Config, Category::State } );

        }, i);

        root.add({"inspect", df, "disk"},
                 "Inspects the inserted disk",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Disk);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);

        root.add({"inspect", hd, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], { Category::Config, Category::State } );

        }, i);

        root.add({"inspect", hd, "drive"},
                 "Displays hard drive parameters",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Drive);

        }, i);

        root.add({"inspect", hd, "volumes"},
                 "Displays summarized volume information",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Volumes);

        }, i);

        root.add({"inspect", hd, "partitions"},
                 "Displays information about all partitions",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], Category::Partitions);

        }, i);
    }

    root.add({"inspect", "host", ""},
             "Displays information about the host machine",
             [this](Arguments& argv, long value) {

        retroShell.dump(host, Category::State);
    });

    // root.setGroup("");

    root.add({"inspect", "server", ""},
             "Displays a server status summary",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager, Category::Status);
    });

    root.add({"inspect", "server", "serial"},
             "Serial port server");

    root.add({"inspect", "server", "serial", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.serServer, { Category::Config, Category::State } );
    });

    root.add({"inspect", "server", "rshell"},
             "Retro shell server");

    root.add({"inspect", "server", "rshell", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.rshServer, { Category::Config, Category::State } );
    });

    root.add({"inspect", "server", "gdb"},
             "GDB server");

    root.add({"inspect", "server", "gdb", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.gdbServer, { Category::Config, Category::State } );
    });

    //
    // OSDebugger
    //

    root.add({"os", "info"},
             "Displays basic system information",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpInfo(ss);
        retroShell << ss;
    });

    root.add({"os", "execbase"},
             "Displays information about the ExecBase struct",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpExecBase(ss);
        retroShell << ss;
    });

    root.add({"os", "interrupts"},
             "Lists all interrupt handlers",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpIntVectors(ss);
        retroShell << ss;
    });

    root.add({"os", "libraries"}, { }, {"<library>"},
             "Lists all libraries",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        isize num;

        if (argv.empty()) {
            osDebugger.dumpLibraries(ss);
        } else if (util::parseHex(argv.front(), &num)) {
            osDebugger.dumpLibrary(ss, (u32)num);
        } else {
            osDebugger.dumpLibrary(ss, argv.front());
        }

        retroShell << ss;
    });

    root.add({"os", "devices"}, { }, {"<device>"},
             "Lists all devices",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        isize num;

        if (argv.empty()) {
            osDebugger.dumpDevices(ss);
        } else if (util::parseHex(argv.front(), &num)) {
            osDebugger.dumpDevice(ss, (u32)num);
        } else {
            osDebugger.dumpDevice(ss, argv.front());
        }

        retroShell << ss;
    });

    root.add({"os", "resources"}, { }, {"<resource>"},
             "Lists all resources",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        isize num;

        if (argv.empty()) {
            osDebugger.dumpResources(ss);
        } else if (util::parseHex(argv.front(), &num)) {
            osDebugger.dumpResource(ss, (u32)num);
        } else {
            osDebugger.dumpResource(ss, argv.front());
        }

        retroShell << ss;
    });

    root.add({"os", "tasks"}, { }, {"<task>"},
             "Lists all tasks",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        isize num;

        if (argv.empty()) {
            osDebugger.dumpTasks(ss);
        } else if (util::parseHex(argv.front(), &num)) {
            osDebugger.dumpTask(ss, (u32)num);
        } else {
            osDebugger.dumpTask(ss, argv.front());
        }

        retroShell << ss;
    });

    root.add({"os", "processes"}, { }, {"<process>"},
             "Lists all processes",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        isize num;

        if (argv.empty()) {
            osDebugger.dumpProcesses(ss);
        } else if (util::parseHex(argv.front(), &num)) {
            osDebugger.dumpProcess(ss, (u32)num);
        } else {
            osDebugger.dumpProcess(ss, argv.front());
        }

        retroShell << ss;
    });

    root.add({"os", "catch"}, {"<task>"},
             "Pauses emulation on task launch",
             [this](Arguments& argv, long value) {

        diagBoard.catchTask(argv.back());
        retroShell << "Waiting for task '" << argv.back() << "' to start...\n";
    });

    root.add({"os", "set"},
             "Configures the component");

    root.add({"os", "set", "diagboard" }, { Arg::boolean },
             "Attaches or detaches the debug expansion board",
             [this](Arguments& argv, long value) {

        diagBoard.setConfigItem(OPT_DIAG_BOARD, parseBool(argv));
    });

    root.setGroup("Guarding the program execution");

    root.add({"break"},     "Manages CPU breakpoints");
    root.add({"watch"},     "Manages CPU watchpoints");
    root.add({"catch"},     "Manages CPU catchpoints");
    root.add({"cbreak"},    "Manages Copper breakpoints");
    root.add({"cwatch"},    "Manages Copper watchpoints");


    //
    // Breakpoints
    //

    root.setGroup("");

    root.add({"break", ""},
             "Lists all breakpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Breakpoints);
    });

    root.add({"break", "at"}, { Arg::address },
             "Sets a breakpoint at the specified address",
             [this](Arguments& argv, long value) {

        cpu.setBreakpoint(u32(parseNum(argv)));
    });

    root.add({"break", "delete"}, { Arg::address },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteBreakpoint(parseNum(argv));
    });

    root.add({"break", "enable"}, { Arg::address },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.enableBreakpoint(parseNum(argv));
    });

    root.add({"break", "disable"}, { Arg::address },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.disableBreakpoint(parseNum(argv));
    });

    root.add({"break", "ignore"}, { Arg::address, Arg::value },
             "Ignores a breakpoint a certain number of times",
             [this](Arguments& argv, long value) {

        cpu.ignoreBreakpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });

    
    //
    // Watchpoints
    //

    root.setGroup("");

    root.add({"watch", ""},
             "Lists all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Watchpoints);
    });

    root.add({"watch", "at"}, { Arg::address },
             "Sets a watchpoint at the specified address",
             [this](Arguments& argv, long value) {

        cpu.setWatchpoint(u32(parseNum(argv)));
    });

    root.add({"watch", "delete"}, { Arg::address },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteWatchpoint(parseNum(argv));
    });

    root.add({"watch", "enable"}, { Arg::address },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableWatchpoint(parseNum(argv));
    });

    root.add({"watch", "disable"}, { Arg::address },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableWatchpoint(parseNum(argv));
    });

    root.add({"watch", "ignore"}, { Arg::address, Arg::value },
             "Ignores a watchpoint a certain number of times",
             [this](Arguments& argv, long value) {

        cpu.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });


    //
    // Catchpoints
    //

    root.setGroup("");

    root.add({"catch", ""},
             "Lists all catchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Catchpoints);
    });

    root.add({"catch", "vector"}, { Arg::value },
             "Catches an exception vector",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv);
        if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
        cpu.setCatchpoint(u8(nr));
    });

    root.add({"catch", "interrupt"}, { Arg::value },
             "Catches an interrupt",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv);
        if (nr < 1 || nr > 7) throw VAError(ERROR_OPT_INVARG, "1...7");
        cpu.setCatchpoint(u8(nr + 24));
    });

    root.add({"catch", "trap"}, { Arg::value },
             "Catches a trap instruction",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv);
        if (nr < 0 || nr > 15) throw VAError(ERROR_OPT_INVARG, "0...15");
        cpu.setCatchpoint(u8(nr + 32));
    });

    root.add({"catch", "delete"}, { Arg::value },
             "Deletes a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteCatchpoint(parseNum(argv));
    });

    root.add({"catch", "enable"}, { Arg::value },
             "Enables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableCatchpoint(parseNum(argv));
    });

    root.add({"catch", "disable"}, { Arg::value },
             "Disables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableCatchpoint(parseNum(argv));
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

        copper.debugger.setBreakpoint(u32(parseNum(argv)));
    });

    root.add({"cbreak", "delete"}, { Arg::value },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteBreakpoint(parseNum(argv));
    });

    root.add({"cbreak", "enable"}, { Arg::value },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableBreakpoint(parseNum(argv));
    });

    root.add({"cbreak", "disable"}, { Arg::value },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableBreakpoint(parseNum(argv));
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

        copper.debugger.setWatchpoint(u32(parseNum(argv)));
    });

    root.add({"cwatch", "delete"}, { Arg::value },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteWatchpoint(parseNum(argv));
    });

    root.add({"cwatch", "enable"}, { Arg::value },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableWatchpoint(parseNum(argv));
    });

    root.add({"cwatch", "disable"}, { Arg::value },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableWatchpoint(parseNum(argv));
    });

    root.add({"cwatch", "ignore"}, { Arg::value, Arg::value },
             "Ignores a watchpoint a certain number of times",
             [this](Arguments& argv, long value) {

        copper.debugger.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
    });

/*
    //
    // Amiga
    //

    root.setGroup("");

    root.add({"amiga", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, { Category::Config, Category::State } );
    });


    //
    // Memory
    //

    root.add({"memory", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, { Category::Config, Category::State } );
    });

    root.add({"memory", "dump"}, { Arg::address },
             "Generates a memory hexdump",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv)), 16, 2);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory", "bankmap"},
             "Dumps the memory bank map",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, Category::BankMap);
    });

    root.add({"memory", "write"}, { Arg::value, Arg::value },
             "Writes a word into memory",
             [this](Arguments& argv, long value) {

        auto addr = (u32)util::parseNum(argv[0]);
        auto val = (u16)util::parseNum(argv[1]);
        mem.patch(addr, val);
    });

*/



    //
    // Miscellaneous
    //

    root.setGroup("Miscellaneous");

    root.add({"set"}, { "<variable>", Arg::value },
             "Sets an internal debug variable",
             [this](Arguments& argv, long value) {

        Amiga::setDebugVariable(argv[0], int(parseNum(argv, 1)));
    });

    root.add({"?"}, { Arg::value },
             "Convert a value into different formats",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        if (isNum(argv)) {
            debugger.convertNumeric(ss, parseNum(argv));
        } else {
            debugger.convertNumeric(ss, argv.front());
        }

        retroShell << '\n' << ss << '\n';
    });
}

void
Interpreter::execWrite(Arguments &argv, isize sz)
{
    auto addr = parseNum(argv, 0);
    auto value = parseNum(argv, 1);
    auto repeats = argv.size() > 2 ? parseNum(argv, 2) : 1;

    printf("addr = %ld value = %ld repeats = %ld\n", addr, value, repeats);

    // Check alignment
    if (sz != 1 && IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);

    for (isize i = 0, a = addr; i < repeats && a <= 0xFFFFFF; i++, a += sz) {

        switch (sz) {

            case 1: 
                mem.poke8  <ACCESSOR_CPU> (u32(a), u8(value)); 
                break;

            case 2:
                mem.poke16 <ACCESSOR_CPU> (u32(a), u16(value)); 
                break;

            case 4:
                mem.poke16 <ACCESSOR_CPU> (u32(a), HI_WORD(value));
                mem.poke16 <ACCESSOR_CPU> (u32(a + 2), LO_WORD(value)); 
                break;

            default:
                fatalError;
        }
    }

    // Show modified memory
    std::stringstream ss;
    debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv, 0)), 1, sz);
    retroShell << ss;
}

void
Interpreter::execFind(Arguments &argv, isize sz)
{
    auto addr = argv.size() == 1 ?
    debugger.memSearch(parseSeq(argv, 0), sz == 1 ? 1 : 2) :
    debugger.memSearch(parseSeq(argv, 0), u32(parseNum(argv, 1)), sz == 1 ? 1 : 2) ;

    if (addr >= 0) {

        std::stringstream ss;
        debugger.memDump<ACCESSOR_CPU>(ss, u32(addr), 1, sz);
        retroShell << ss;

    } else {

        std::stringstream ss;
        ss << "Sequence not found";
        retroShell << ss;
    }
}

}
