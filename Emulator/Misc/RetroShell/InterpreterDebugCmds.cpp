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

        argv.empty() ? amiga.run() : debugger.jump(parseAddr(argv[0]));
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
        cpu.disassembleRange(ss, parseAddr(argv[0], cpu.getPC0()), 16);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"a"}, { }, { Arg::address },
             "Dump memory in ASCII",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        debugger.ascDump<ACCESSOR_CPU>(ss, parseAddr(argv[0], debugger.current), 16);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"m"}, { }, { Arg::address },
             std::pair<string, string>("m[.b|.w|.l]", "Dump memory"),
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        debugger.memDump<ACCESSOR_CPU>(ss, parseAddr(argv[0], debugger.current), 16, value);
        retroShell << '\n' << ss << '\n';
    }, 2);

    root.clone("m.b",      {"m"}, 1);
    root.clone("m.w",      {"m"}, 2);
    root.clone("m.l",      {"m"}, 4);

    /*
    root.add({"r"}, { }, { Arg::address },
             std::pair<string, string>("r[.b|.w|.l]", "Read from a register or memory"),
             [this](Arguments& argv, long value) {

        // Resolve address
        u32 addr = debugger.current;

        if (argv.size() > 0) {
            try {
                addr = 0xDFF000 + u32(parseEnum<ChipsetRegEnum>(argv[0]) << 1);
            } catch (...) {
                addr = parseAddr(argv[0]);
            };
        }

        // Access memory
        std::stringstream ss;
        ss << util::hex(2 * value, addr) << ": ";
        ss << util::hex(2 * value, debugger.read(addr, value)) << '\n';
        retroShell << ss;
    }, 2);

    root.clone("r.b", {"r"}, "", 1);
    root.clone("r.w", {"r"}, "", 2);
    root.clone("r.l", {"r"}, "", 4);
    */

    root.add({"w"}, { Arg::value }, { "{ " + Arg::address + " | " + ChipsetRegEnum::argList() + " }" },
             std::pair<string, string>("w[.b|.w|.l]", "Write into a register or memory"),
             [this](Arguments& argv, long value) {

        // Resolve address
        u32 addr = debugger.current;

        if (argv.size() > 1) {
            try {
                addr = 0xDFF000 + u32(parseEnum<ChipsetRegEnum>(argv[1]) << 1);
            } catch (...) {
                addr = parseAddr(argv[1]);
            };
        }

        // Access memory
        debugger.write(addr, u32(parseNum(argv[0])), value);
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

            auto pattern = parseSeq(argv[0]);
            auto addr = u32(parseNum(argv[1], debugger.current));
            auto found = debugger.memSearch(pattern, addr, value == 1 ? 1 : 2);

            if (found >= 0) {

                std::stringstream ss;
                debugger.memDump<ACCESSOR_CPU>(ss, u32(found), 1, value);
                retroShell << ss;

            } else {

                std::stringstream ss;
                ss << "Not found";
                retroShell << ss;
            }
        }
    }, 1);

    root.clone("f.b", {"f"}, "", 1);
    root.clone("f.w", {"f"}, "", 2);
    root.clone("f.l", {"f"}, "", 4);


    root.clone("c.b", {"c"}, "", 1);
    root.clone("c.w", {"c"}, "", 2);
    root.clone("c.l", {"c"}, "", 4);

    root.add({"e"}, { Arg::address, Arg::count }, { Arg::value },
             std::pair<string, string>("e[.b|.w|.l]", "Erase memory"),
             [this](Arguments& argv, long value) {

        {   SUSPENDED

            auto addr = parseAddr(argv[0]);
            auto count = parseNum(argv[1]);
            auto val = u32(parseNum(argv[2], 0));

            debugger.write(addr, val, value, count);
        }
    }, 1);

    root.clone("e.b", {"e"}, "", 1);
    root.clone("e.w", {"e"}, "", 2);
    root.clone("e.l", {"e"}, "", 4);

    root.add({"i"},
             "Inspect a component");

    root.add({"r"},
             "Show registers");

    root.add({"os"},
             "Run the OS debugger");

    //
    // Second-level commands
    //

    root.setGroup("Components");

    root.add({"i", "amiga"},         "Main computer");
    root.add({"i", "memory"},        "RAM and ROM");
    root.add({"i", "cpu"},           "Motorola CPU");
    root.add({"i", "ciaa"},          "Complex Interface Adapter A");
    root.add({"i", "ciab"},          "Complex Interface Adapter B");
    root.add({"i", "agnus"},         "Custom Chipset");
    root.add({"i", "blitter"},       "Coprocessor");
    root.add({"i", "copper"},        "Coprocessor");
    root.add({"i", "paula"},         "Ports, Audio, Interrupts");
    root.add({"i", "denise"},        "Graphics");
    root.add({"i", "rtc"},           "Real-time clock");
    root.add({"i", "zorro"},         "Expansion boards");
    root.add({"i", "controlport"},   "Joystick ports");
    root.add({"i", "serial"},        "Serial port");

    root.setGroup("Peripherals");

    root.add({"i", "keyboard"},      "Keyboard");
    root.add({"i", "mouse"},         "Mouse");
    root.add({"i", "joystick"},      "Joystick");
    root.add({"i", "df0"},           std::pair<string,string>("df[n]", "Floppy drive n"));
    root.add({"i", "df1"},           "");
    root.add({"i", "df2"},           "");
    root.add({"i", "df3"},           "");
    root.add({"i", "hd0"},           std::pair<string,string>("hd[n]", "Hard drive n"));
    root.add({"i", "hd1"},           "");
    root.add({"i", "hd2"},           "");
    root.add({"i", "hd3"},           "");

    root.setGroup("Miscellaneous");

    root.add({"i", "host"},          "Host machine");
    root.add({"i", "server"},        "Remote server");

    root.setGroup("");

    root.add({"r", "cpu"},
             "Motorola CPU",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Registers);
    });

    root.add({"r", "ciaa"},          
             "Complex Interface Adapter A",
             [this](Arguments& argv, long value) {

        retroShell.dump(ciaa, Category::Registers);
    });

    root.add({"r", "ciab"},          
             "Complex Interface Adapter B",
             [this](Arguments& argv, long value) {

        retroShell.dump(ciab, Category::Registers);
    });

    root.add({"r", "agnus"},         
             "Custom Chipset",
             [this](Arguments& argv, long value) {

        retroShell.dump(agnus, Category::Registers);
    });

    root.add({"r", "blitter"},       
             "Coprocessor",
             [this](Arguments& argv, long value) {

        retroShell.dump(blitter, Category::Registers);
    });

    root.add({"r", "copper"},        
             "Coprocessor",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper, Category::Registers);
    });

    root.add({"r", "paula"},
             "Ports, Audio, Interrupts",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, Category::Registers);
    });

    root.add({"r", "denise"},        
             "Graphics",
             [this](Arguments& argv, long value) {

        retroShell.dump(denise, Category::Registers);
    });

    root.add({"r", "rtc"},           
             "Real-time clock",
             [this](Arguments& argv, long value) {

        retroShell.dump(rtc, Category::Registers);
    });

    //
    // Third-level commands
    //

    root.setGroup("");

    root.add({"i", "amiga", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, { Category::Config, Category::State } );
    });

    root.add({"i", "memory", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, { Category::Config, Category::State } );
    });

    root.add({"i", "memory", "bankmap"},
             "Dumps the memory bank map",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, Category::BankMap);
    });

    root.add({"i", "cpu", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, { Category::Config, Category::State } );
    });

    /*
    root.add({"i", "cpu", "vectors"},
             "Dump the vector table",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Vectors);
    });
    */

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "ciaa" : "ciab";

        root.add({"i", cia, ""},
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa, { Category::Config, Category::State } );
            } else {
                retroShell.dump(ciab, { Category::Config, Category::State } );
            }
        }, i);

        root.add({"i", cia, "tod"},
                 "Display the state of the 24-bit counter",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(ciaa.tod, Category::State );
            } else {
                retroShell.dump(ciab.tod, Category::State );
            }
        }, i);
    }

    root.add({"i", "agnus", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(agnus, { Category::Config, Category::State } );
    });

    root.add({"i", "agnus", "beam"},
             "Display the current beam position",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Beam);
    });

    root.add({"i", "agnus", "dma"},
             "Print all scheduled DMA events",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Dma);
    });

    root.add({"i", "agnus", "sequencer"},
             "Inspect the sequencer logic",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus.sequencer, { Category::State, Category::Signals } );
    });

    root.add({"i", "agnus", "events"},
             "Inspect the event scheduler",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.agnus, Category::Events);
    });

    root.add({"i", "blitter", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(blitter, { Category::Config, Category::State } );
    });

    root.add({"i", "copper", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper, { Category::Config, Category::State } );
    });

    root.add({"i", "copper", "list"}, { Arg::value },
             "Print the Copper list",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);

        switch (nr) {

            case 1: retroShell.dump(amiga.agnus.copper, Category::List1); break;
            case 2: retroShell.dump(amiga.agnus.copper, Category::List2); break;

            default:
                throw VAError(ERROR_OPT_INVARG, "1 or 2");
        }
    });

    /*
    root.add({"i", "paula", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula, Category::Registers);
    });
    */

    root.add({"i", "paula", "audio"},
             "Audio unit");

    root.add({"i", "paula", "dc"},
             "Disk controller");

    root.add({"i", "paula", "uart"},
             "Universal Asynchronous Receiver Transmitter");

    root.add({"i", "paula", "audio", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula.muxer, { Category::Config, Category::State } );
    });

    root.add({"i", "paula", "audio", "filter"},
             "Inspect the internal filter state",
             [this](Arguments& argv, long value) {

        retroShell.dump(paula.muxer.filter, { Category::Config, Category::State } );
    });

    root.add({"i", "paula", "dc", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(diskController, { Category::Config, Category::State } );
    });

    root.add({"i", "paula", "uart", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(uart, Category::State);
    });

    root.add({"i", "denise", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(denise, { Category::Config, Category::State } );
    });

    root.add({"i", "rtc", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(rtc, { Category::Config, Category::State } );
    });

    root.add({"i", "zorro", ""},
             "List all connected boards",
             [this](Arguments& argv, long value) {

        retroShell.dump(zorro, Category::Slots);
    });

    root.add({"i", "zorro", "i"}, { Arg::value },
             "Inspect a specific Zorro board",
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
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1, Category::State);
            if (value == 2) retroShell.dump(controlPort2, Category::State);

        }, i);
    }

    root.add({"i", "serial", ""},
             "Display the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(serialPort, { Category::Config, Category::State } );
    });

    root.add({"i", "keyboard", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(keyboard, { Category::Config, Category::State } );
    });

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"i", "mouse", nr},
                 "Mouse in port " + nr);

        root.add({"i", "mouse", nr, ""},
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.mouse, { Category::Config, Category::State } );
            if (value == 2) retroShell.dump(controlPort2.mouse, { Category::Config, Category::State } );

        }, i);

        root.add({"i", "joystick", nr},
                 "Joystick in port " + nr);

        root.add({"i", "joystick", nr, ""},
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            if (value == 1) retroShell.dump(controlPort1.joystick, Category::State);
            if (value == 2) retroShell.dump(controlPort2.joystick, Category::State);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({"i", df, ""},
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], { Category::Config, Category::State } );

        }, i);

        root.add({"i", df, "disk"},
                 "Inspect the inserted disk",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Disk);

        }, i);
    }

    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);

        root.add({"i", hd, ""},
                 "Inspect the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], { Category::Config, Category::State } );

        }, i);

        root.add({"i", hd, "drive"},
                 "Display hard drive parameters",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Drive);

        }, i);

        root.add({"i", hd, "volumes"},
                 "Display summarized volume information",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.df[value], Category::Volumes);

        }, i);

        root.add({"i", hd, "partitions"},
                 "Display information about all partitions",
                 [this](Arguments& argv, long value) {

            retroShell.dump(*amiga.hd[value], Category::Partitions);

        }, i);
    }

    root.add({"i", "host", ""},
             "Display information about the host machine",
             [this](Arguments& argv, long value) {

        retroShell.dump(host, Category::State);
    });

    // root.setGroup("");

    root.add({"i", "server", ""},
             "Display a server status summary",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager, Category::Status);
    });

    root.add({"i", "server", "serial"},
             "Serial port server");

    root.add({"i", "server", "serial", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.serServer, { Category::Config, Category::State } );
    });

    root.add({"i", "server", "rshell"},
             "Retro shell server");

    root.add({"i", "server", "rshell", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.rshServer, { Category::Config, Category::State } );
    });

    root.add({"i", "server", "gdb"},
             "GDB server");

    root.add({"i", "server", "gdb", ""},
             "Inspect the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager.gdbServer, { Category::Config, Category::State } );
    });

    //
    // OSDebugger
    //

    root.add({"os", "info"},
             "Display basic system information",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpInfo(ss);
        retroShell << ss;
    });

    root.add({"os", "execbase"},
             "Display information about the ExecBase struct",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpExecBase(ss);
        retroShell << ss;
    });

    root.add({"os", "interrupts"},
             "List all interrupt handlers",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        osDebugger.dumpIntVectors(ss);
        retroShell << ss;
    });

    root.add({"os", "libraries"}, { }, {"<library>"},
             "List all libraries",
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
             "List all devices",
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
             "List all resources",
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
             "List all tasks",
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
             "List all processes",
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
             "Pause emulation on task launch",
             [this](Arguments& argv, long value) {

        diagBoard.catchTask(argv.back());
        retroShell << "Waiting for task '" << argv.back() << "' to start...\n";
    });

    root.add({"os", "set"},
             "Configure the component");

    root.add({"os", "set", "diagboard" }, { Arg::boolean },
             "Attach or detach the debug expansion board",
             [this](Arguments& argv, long value) {

        diagBoard.setConfigItem(OPT_DIAG_BOARD, parseBool(argv[0]));
    });


    //
    // Breakpoints
    //

    root.setGroup("");

    root.add({"break", ""},
             "List all breakpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Breakpoints);
    });

    root.add({"break", "at"}, { Arg::address }, { Arg::ignores },
             "Set a breakpoint",
             [this](Arguments& argv, long value) {

        auto addr = parseAddr(argv[0]);
        if (IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);
        cpu.setBreakpoint(addr, parseNum(argv[1], 0));
    });

    root.add({"break", "delete"}, { Arg::nr },
             "Delete breakpoints",
             [this](Arguments& argv, long value) {

        cpu.deleteBreakpoint(parseNum(argv[0]));
    });

    root.add({"break", "toggle"}, { Arg::nr },
             "Enable or disable breakpoints",
             [this](Arguments& argv, long value) {

        cpu.toggleBreakpoint(parseNum(argv[0]));
    });

    root.setGroup("");

    root.add({"watch", ""},
             "Lists all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga.cpu, Category::Watchpoints);
    });

    root.add({"watch", "at"}, { Arg::address }, { Arg::ignores },
             "Set a watchpoint at the specified address",
             [this](Arguments& argv, long value) {

        cpu.setWatchpoint(u32(parseNum(argv[0])), parseNum(argv[1], 0));
    });

    root.add({"watch", "delete"}, { Arg::address },
             "Delete a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteWatchpoint(parseNum(argv[0]));
    });

    root.add({"watch", "toggle"}, { Arg::address },
             "Enable or disable a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.toggleWatchpoint(parseNum(argv[0]));
    });

    root.setGroup("");

    root.add({"catch", ""},
             "List all catchpoints",
             [this](Arguments& argv, long value) {

        if (argv.empty()) {

            retroShell.dump(amiga.cpu, Category::Catchpoints);

        } else {

            auto nr = parseNum(argv[0]);
            if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
            cpu.setCatchpoint(u8(nr), parseNum(argv[1], 0));
        }
    });

    root.add({"catch", "vector"}, { Arg::value }, { Arg::ignores },
             "Catch an exception vector",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);
        if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
        cpu.setCatchpoint(u8(nr), parseNum(argv[1], 0));
    });

    root.add({"catch", "interrupt"}, { Arg::value }, { Arg::ignores },
             "Catch an interrupt",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);
        if (nr < 1 || nr > 7) throw VAError(ERROR_OPT_INVARG, "1...7");
        cpu.setCatchpoint(u8(nr + 24), parseNum(argv[1], 0));
    });

    root.add({"catch", "trap"}, { Arg::value }, { Arg::ignores },
             "Catch a trap instruction",
             [this](Arguments& argv, long value) {

        auto nr = parseNum(argv[0]);
        if (nr < 0 || nr > 15) throw VAError(ERROR_OPT_INVARG, "0...15");
        cpu.setCatchpoint(u8(nr + 32));
    });

    root.add({"catch", "delete"}, { Arg::value },
             "Delete a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.deleteCatchpoint(parseNum(argv[0]));
    });

    root.add({"catch", "toggle"}, { Arg::value },
             "Enable or disable a catchpoint",
             [this](Arguments& argv, long value) {

        cpu.enableCatchpoint(parseNum(argv[0]));
    });

    root.add({"cbreak", ""},
             "List all breakpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper.debugger, Category::Breakpoints);
    });

    root.add({"cbreak", "at"}, { Arg::value }, { Arg::ignores },
             "Set a breakpoint at the specified address",
             [this](Arguments& argv, long value) {

        auto addr = parseAddr(argv[0]);
        if (IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);
        copper.debugger.setBreakpoint(addr, parseNum(argv[1], 0));
    });

    root.add({"cbreak", "delete"}, { Arg::value },
             "Delete a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteBreakpoint(parseNum(argv[0]));
    });

    root.add({"cbreak", "toggle"}, { Arg::value },
             "Enable or disable a breakpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.toggleBreakpoint(parseNum(argv[0]));
    });

    root.add({"cwatch", ""},
             "List all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(copper.debugger, Category::Watchpoints);
    });

    root.add({"cwatch", "at"}, { Arg::value }, { Arg::ignores },
             "Set a watchpoint at the specified address",
             [this](Arguments& argv, long value) {

        auto addr = parseAddr(argv[0]);
        if (IS_ODD(addr)) throw VAError(ERROR_ADDR_UNALIGNED);
        copper.debugger.setWatchpoint(addr, parseNum(argv[1], 0));
    });

    root.add({"cwatch", "delete"}, { Arg::value },
             "Delete a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.deleteWatchpoint(parseNum(argv[0]));
    });

    root.add({"cwatch", "toggle"}, { Arg::value },
             "Enable or disable a watchpoint",
             [this](Arguments& argv, long value) {

        copper.debugger.toggleWatchpoint(parseNum(argv[0]));
    });

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

}
