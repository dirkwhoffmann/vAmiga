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

    root.setGroup("Program execution");

    root.add({"goto"}, { }, { Arg::value },
             std::pair <string, string>("g[oto]", "Goto address"),
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        if (argv.empty()) {
            amiga.run();
        } else {
            debugger.jump(u32(parseNum(argv[0])));
        }
    });

    root.clone("g", {"goto"});

    root.add({"step"}, { }, { },
             std::pair <string, string>("s[tep]", "Step into the next instruction"),
             [this](Arguments& argv, long value) {

        debugger.stepInto();
    });

    root.clone("s", {"step"});

    root.add({"next"}, { }, { },
             std::pair <string, string>("n[next]", "Step over the next instruction"),
             [this](Arguments& argv, long value) {

        debugger.stepOver();
    });

    root.clone("n", {"next"});

    root.add({"break"},     "Manage CPU breakpoints");
    root.add({"watch"},     "Manage CPU watchpoints");
    root.add({"catch"},     "Manage CPU catchpoints");
    root.add({"cbreak"},    "Manage Copper breakpoints");
    root.add({"cwatch"},    "Manage Copper watchpoints");

    root.setGroup("Monitoring");

    root.add({"d"}, { }, { Arg::address },
             "Disassemble instructions",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        auto addr = argv.empty() ? cpu.getPC0() : u32(parseNum(argv[0]));
        cpu.disassembleRange(ss, addr, 16);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"a"}, { }, { Arg::address },
             "Dump memory in ASCII",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.ascDump<ACCESSOR_CPU>(ss, 16) :
        debugger.ascDump<ACCESSOR_CPU>(ss, u32(parseNum(argv[0])), 16);

        retroShell << '\n' << ss << '\n';
    });

    root.add({"m"}, { }, { Arg::address },
             std::pair<string, string>("m[.b|.w|.l]", "Dump memory"),
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        argv.empty() ?
        debugger.memDump<ACCESSOR_CPU>(ss, 16, value) :
        debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv[0])), 16, value);

        retroShell << '\n' << ss << '\n';
    }, 2);

    root.clone("m.b",      {"m"}, 1);
    root.clone("m.w",      {"m"}, 2);
    root.clone("m.l",      {"m"}, 4);

    root.add({"r"}, { Arg::address },
             std::pair<string, string>("r[.b|.w|.l]", "Read from a register or memory"),
             [this](Arguments& argv, long value) {

        auto addr = u32(parseNum(argv[0]));

        // Check alignment
        if (value != 1 && IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);

        {   SUSPENDED

            std::stringstream ss;

            switch (value) {

                case 1: debugger.convertNumeric(ss, mem.spypeek8  <ACCESSOR_CPU> (addr)); break;
                case 2: debugger.convertNumeric(ss, mem.spypeek16 <ACCESSOR_CPU> (addr)); break;
                case 4: debugger.convertNumeric(ss, mem.spypeek32 <ACCESSOR_CPU> (addr)); break;

                default:
                    fatalError;
            }

            retroShell << ss;
        }

    }, 2);

    root.clone("r.b", {"r"}, "", 1);
    root.clone("r.w", {"r"}, "", 2);
    root.clone("r.l", {"r"}, "", 4);

    root.add({"w"}, { Arg::address, Arg::value }, { Arg::count },
             std::pair<string, string>("w[.b|.w|.l]", "Write into a register or memory"),
             [this](Arguments& argv, long value) {

        auto addr = parseNum(argv[0]);
        auto val = parseNum(argv[1]);
        auto repeats = argv.size() > 2 ? parseNum(argv[2]) : 1;

        // Check alignment
        if (val != 1 && IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);

        {   SUSPENDED

            for (isize i = 0, a = addr; i < repeats && a <= 0xFFFFFF; i++, a += value) {

                switch (value) {

                    case 1:
                        mem.poke8  <ACCESSOR_CPU> (u32(a), u8(val));
                        break;

                    case 2:
                        mem.poke16 <ACCESSOR_CPU> (u32(a), u16(val));
                        break;

                    case 4:
                        mem.poke16 <ACCESSOR_CPU> (u32(a), HI_WORD(val));
                        mem.poke16 <ACCESSOR_CPU> (u32(a + 2), LO_WORD(val));
                        break;

                    default:
                        fatalError;
                }
            }

            // Show modified memory
            std::stringstream ss;
            debugger.memDump<ACCESSOR_CPU>(ss, u32(parseNum(argv[0])), 1, value);
            retroShell << ss;
        }
    }, 2);

    root.clone("w.b", {"w"}, "", 1);
    root.clone("w.w", {"w"}, "", 2);
    root.clone("w.l", {"w"}, "", 4);

    root.add({"c"}, { Arg::src, Arg::dst, Arg::count },
             std::pair<string, string>("c[.b|.w|.l]", "Copy a chunk of memory"),
             [this](Arguments& argv, long value) {

        auto src = parseNum(argv[0]);
        auto dst = parseNum(argv[1]);
        auto cnt = parseNum(argv[2]) * value;

        {   SUSPENDED

            if (src < dst) {

                for (isize i = cnt - 1; i >= 0; i--)
                    mem.poke8<ACCESSOR_CPU>(u32(dst + i), mem.spypeek8<ACCESSOR_CPU>(u32(src + i)));

            } else {

                for (isize i = 0; i <= cnt - 1; i++)
                    mem.poke8<ACCESSOR_CPU>(u32(dst + i), mem.spypeek8<ACCESSOR_CPU>(u32(src + i)));
            }
        }
    }, 1);

    root.clone("c.b", {"c"}, "", 1);
    root.clone("c.w", {"c"}, "", 2);
    root.clone("c.l", {"c"}, "", 4);

    root.add({"f"}, { Arg::sequence }, { Arg::address },
             std::pair<string, string>("f[.b|.w|.l]", "Find a sequence in memory"),
             [this](Arguments& argv, long value) {

        {   SUSPENDED

            auto addr = argv.size() == 1 ?
            debugger.memSearch(parseSeq(argv[0]), value == 1 ? 1 : 2) :
            debugger.memSearch(parseSeq(argv[0]), u32(parseNum(argv[1])), value == 1 ? 1 : 2) ;

            if (addr >= 0) {

                std::stringstream ss;
                debugger.memDump<ACCESSOR_CPU>(ss, u32(addr), 1, value);
                retroShell << ss;

            } else {

                std::stringstream ss;
                ss << "Sequence not found";
                retroShell << ss;
            }
        }
    }, 1);

    root.clone("f.b", {"f"}, "", 1);
    root.clone("f.w", {"f"}, "", 2);
    root.clone("f.l", {"f"}, "", 4);

    /*
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
    */

    root.add({"i"},
             "Inspect a component");

    root.add({"os"},
             "Runs the OS debugger");

    //
    // Second-level commands
    //

    root.setGroup("Inspecting components");

    root.add({"i", "amiga"},         "Main computer");
    root.add({"i", "memory"},        "RAM and ROM");
    root.add({"i", "cpu"},           "Motorola 68k CPU");
    root.add({"i", "ciaa"},          "Complex Interface Adapter A");
    root.add({"i", "ciab"},          "Complex Interface Adapter B");
    root.add({"i", "agnus"},         "Custom Chipset");
    root.add({"i", "blitter"},       "Coprocessor");
    root.add({"i", "copper"},        "Coprocessor");
    root.add({"i", "paula"},         "Custom Chipset");
    root.add({"i", "denise"},        "Custom Chipset");
    root.add({"i", "rtc"},           "Real-time clock");
    root.add({"i", "zorro"},         "Expansion boards");
    root.add({"i", "controlport"},   "Control ports");
    root.add({"i", "serial"},        "Serial port");

    root.setGroup("Inspecting peripherals");

    root.add({"i", "keyboard"},      "Keyboard");
    root.add({"i", "mouse"},         "Mouse");
    root.add({"i", "joystick"},      "Joystick");
    root.add({"i", "df0"},           "Floppy drive 0");
    root.add({"i", "df1"},           "Floppy drive 1");
    root.add({"i", "df2"},           "Floppy drive 2");
    root.add({"i", "df3"},           "Floppy drive 3");
    root.add({"i", "hd0"},           "Hard drive 0");
    root.add({"i", "hd1"},           "Hard drive 1");
    root.add({"i", "hd2"},           "Hard drive 2");
    root.add({"i", "hd3"},           "Hard drive 3");

    root.setGroup("Miscellaneous");

    root.add({"i", "host"},          "Host machine");
    root.add({"i", "server"},        "Remote server");

    //
    // Third-level commands
    //

    root.setGroup("");

    root.add({"i", "cpu", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "cpu", "vectors"},
             "Dumps the vector table",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Vectors);
    });

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({"i", cia, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa, { Category::Config, Category::State, Category::Registers } );
            } else {
                retroShell.dump(ciab, { Category::Config, Category::State, Category::Registers } );
            }
        }, i);

        root.add({"i", cia, "tod"},
                 "Displays the state of the 24-bit counter",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa.tod, Category::State );
            } else {
                retroShell.dump(ciab.tod, Category::State );
            }
        }, i);
    }

    root.add({"i", "agnus", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(agnus, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "agnus", "beam"},
             "Displays the current beam position",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Beam);
    });

    root.add({"i", "agnus", "dma"},
             "Prints all scheduled DMA events",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Dma);
    });

    root.add({"i", "agnus", "sequencer"},
             "Inspects the sequencer logic",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus.sequencer, { Category::State, Category::Registers, Category::Signals } );
    });

    root.add({"i", "agnus", "events"},
             "Inspects the event scheduler",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Events);
    });

    root.add({"i", "blitter", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(blitter, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "copper", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "copper", "list"}, { Arg::value },
             "Prints the Copper list",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);

        switch (nr) {

            case 1: retroShell.dump(amiga.agnus.copper, Category::List1); break;
            case 2: retroShell.dump(amiga.agnus.copper, Category::List2); break;

            default:
                throw VAError(ERROR_OPT_INVARG, "1 or 2");
        }
    });

    root.add({"i", "paula", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, Category::Registers);
    });

    root.add({"i", "paula", "audio"},
             "Audio unit");

    root.add({"i", "paula", "dc"},
             "Disk controller");

    root.add({"i", "paula", "uart"},
             "Universal Asynchronous Receiver Transmitter");

    root.add({"i", "paula", "audio", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "paula", "audio", "filter"},
             "Inspects the internal filter state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula.muxer.filter, { Category::Config, Category::State } );
    });

    root.add({"i", "paula", "dc", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(diskController, { Category::Config, Category::State } );
    });

    root.add({"i", "paula", "uart", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(uart, Category::State);
    });

    root.add({"i", "denise", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(denise, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "rtc", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(rtc, { Category::Config, Category::State, Category::Registers } );
    });

    root.add({"i", "zorro", ""},
             "Lists all connected boards",
             [this](Arguments& argv, long value) {

        retroShell.dump(zorro, Category::Slots);
    });

    root.add({"i", "zorro", "i"}, { Arg::value },
             "Inspects a specific Zorro board",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);

        if (auto board = zorro.getBoard(nr); board != nullptr) {

            retroShell.dump(*board, { Category::Properties, Category::State, Category::Stats } );
        }
    });

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"i", "controlport", nr},
                 "Control port " + nr);

        root.add({"i", "controlport", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1, Category::State);
            if (value == 2) retroShell.dump(controlPort2, Category::State);

        }, i);
    }

    root.add({"i", "serial", ""},
             "Displays the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(serialPort, { Category::Config, Category::State } );
    });

    root.add({"i", "keyboard", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(keyboard, { Category::Config, Category::State } );
    });

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"i", "mouse", nr},
                 "Mouse in port " + nr);

        root.add({"i", "mouse", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.mouse, { Category::Config, Category::State } );
            if (value == 2) retroShell.dump(controlPort2.mouse, { Category::Config, Category::State } );

        }, i);

        root.add({"i", "joystick", nr},
                 "Joystick in port " + nr);

        root.add({"i", "joystick", nr, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.joystick, Category::State);
            if (value == 2) retroShell.dump(controlPort2.joystick, Category::State);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({"i", df, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], { Category::Config, Category::State } );

        }, i);

        root.add({"i", df, "disk"},
                 "Inspects the inserted disk",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Disk);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);

        root.add({"i", hd, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], { Category::Config, Category::State } );

        }, i);

        root.add({"i", hd, "drive"},
                 "Displays hard drive parameters",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Drive);

        }, i);

        root.add({"i", hd, "volumes"},
                 "Displays summarized volume information",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Volumes);

        }, i);

        root.add({"i", hd, "partitions"},
                 "Displays information about all partitions",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], Category::Partitions);

        }, i);
    }

    root.add({"i", "host", ""},
             "Displays information about the host machine",
             [this](Arguments& argv, long value) {

        retroShell.dump(host, Category::State);
    });

    // root.setGroup("");

    root.add({"i", "server", ""},
             "Displays a server status summary",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager, Category::Status);
    });

    root.add({"i", "server", "serial"},
             "Serial port server");

    root.add({"i", "server", "serial", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.serServer, { Category::Config, Category::State } );
    });

    root.add({"i", "server", "rshell"},
             "Retro shell server");

    root.add({"i", "server", "rshell", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.rshServer, { Category::Config, Category::State } );
    });

    root.add({"i", "server", "gdb"},
             "GDB server");

    root.add({"i", "server", "gdb", ""},
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

        diagBoard.setConfigItem(OPT_DIAG_BOARD, parseBool(argv[0]));
    });


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

        cpu.setBreakpoint(u32(parseNum(argv[0])));
    });

    root.add({"break", "delete"}, { Arg::address },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteBreakpoint(parseNum(argv[0]));
    });

    root.add({"break", "enable"}, { Arg::address },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.enableBreakpoint(parseNum(argv[0]));
    });

    root.add({"break", "disable"}, { Arg::address },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.disableBreakpoint(parseNum(argv[0]));
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

        cpu.setWatchpoint(u32(parseNum(argv[0])));
    });

    root.add({"watch", "delete"}, { Arg::address },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteWatchpoint(parseNum(argv[0]));
    });

    root.add({"watch", "enable"}, { Arg::address },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableWatchpoint(parseNum(argv[0]));
    });

    root.add({"watch", "disable"}, { Arg::address },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableWatchpoint(parseNum(argv[0]));
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

        auto nr = parseNum(argv[0]);
        if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
        cpu.setCatchpoint(u8(nr));
    });

    root.add({"catch", "interrupt"}, { Arg::value },
             "Catches an interrupt",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);
        if (nr < 1 || nr > 7) throw VAError(ERROR_OPT_INVARG, "1...7");
        cpu.setCatchpoint(u8(nr + 24));
    });

    root.add({"catch", "trap"}, { Arg::value },
             "Catches a trap instruction",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);
        if (nr < 0 || nr > 15) throw VAError(ERROR_OPT_INVARG, "0...15");
        cpu.setCatchpoint(u8(nr + 32));
    });

    root.add({"catch", "delete"}, { Arg::value },
             "Deletes a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteCatchpoint(parseNum(argv[0]));
    });

    root.add({"catch", "enable"}, { Arg::value },
             "Enables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableCatchpoint(parseNum(argv[0]));
    });

    root.add({"catch", "disable"}, { Arg::value },
             "Disables a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.disableCatchpoint(parseNum(argv[0]));
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

        copper.debugger.setBreakpoint(u32(parseNum(argv[0])));
    });

    root.add({"cbreak", "delete"}, { Arg::value },
             "Deletes a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteBreakpoint(parseNum(argv[0]));
    });

    root.add({"cbreak", "enable"}, { Arg::value },
             "Enables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableBreakpoint(parseNum(argv[0]));
    });

    root.add({"cbreak", "disable"}, { Arg::value },
             "Disables a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableBreakpoint(parseNum(argv[0]));
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

        copper.debugger.setWatchpoint(u32(parseNum(argv[0])));
    });

    root.add({"cwatch", "delete"}, { Arg::value },
             "Deletes a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteWatchpoint(parseNum(argv[0]));
    });

    root.add({"cwatch", "enable"}, { Arg::value },
             "Enables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.enableWatchpoint(parseNum(argv[0]));
    });

    root.add({"cwatch", "disable"}, { Arg::value },
             "Disables a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.disableWatchpoint(parseNum(argv[0]));
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

        Amiga::setDebugVariable(argv[0], int(parseNum(argv[1])));
    });

    root.add({"?"}, { Arg::value },
             "Convert a value into different formats",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        if (isNum(argv[0])) {
            debugger.convertNumeric(ss, u32(parseNum(argv[0])));
        } else {
            debugger.convertNumeric(ss, argv.front());
        }

        retroShell << '\n' << ss << '\n';
    });
}

/*
void
Interpreter::execRead(Arguments &argv, isize sz)
{
    auto addr = u32(parseNum(argv, 0));

    // Check alignment
    if (sz != 1 && IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);

    {   SUSPENDED

        std::stringstream ss;

        switch (sz) {

            case 1: debugger.convertNumeric(ss, mem.spypeek8  <ACCESSOR_CPU> (addr)); break;
            case 2: debugger.convertNumeric(ss, mem.spypeek16 <ACCESSOR_CPU> (addr)); break;
            case 4: debugger.convertNumeric(ss, mem.spypeek32 <ACCESSOR_CPU> (addr)); break;

            default:
                fatalError;
        }

        retroShell << ss;
    }
}
*/

/*
void
Interpreter::execWrite(Arguments &argv, isize sz)
{
    auto addr = parseNum(argv, 0);
    auto value = parseNum(argv, 1);
    auto repeats = argv.size() > 2 ? parseNum(argv, 2) : 1;

    // Check alignment
    if (sz != 1 && IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);

    {   SUSPENDED

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
}
*/
/*
void
Interpreter::execCopy(Arguments &argv, isize sz)
{
    auto src = parseNum(argv, 0);
    auto dst = parseNum(argv, 1);
    auto cnt = parseNum(argv, 2) * sz;

    {   SUSPENDED

        if (src < dst) {

            for (isize i = cnt - 1; i >= 0; i--)
                mem.poke8<ACCESSOR_CPU>(u32(dst + i), mem.spypeek8<ACCESSOR_CPU>(u32(src + i)));

        } else {

            for (isize i = 0; i <= cnt - 1; i++)
                mem.poke8<ACCESSOR_CPU>(u32(dst + i), mem.spypeek8<ACCESSOR_CPU>(u32(src + i)));
        }
    }
}
*/
/*
void
Interpreter::execFind(Arguments &argv, isize sz)
{
    {   SUSPENDED

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
*/

}
