// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Console.h"
#include "Emulator.h"

namespace vamiga {

void
DebugConsole::_pause()
{
    *this << '\n' << '\n';
    exec("state");
    *this << getPrompt();
}

string
DebugConsole::getPrompt()
{
    std::stringstream ss;

    ss << "(";
    ss << std::right << std::setw(0) << std::dec << isize(agnus.pos.v);
    ss << ",";
    ss << std::right << std::setw(0) << std::dec << isize(agnus.pos.h);
    ss << ") $";
    ss << std::right << std::setw(6) << std::hex << std::setfill('0') << isize(cpu.getPC0());
    ss << ": ";

    return ss.str();
}

void
DebugConsole::welcome()
{
    printHelp();
    *this << '\n';
}

void
DebugConsole::printHelp()
{
    storage << "Type 'help' or press 'TAB' twice for help.\n";
    storage << "Type '.' or press 'SHIFT+RETURN' to exit debug mode.";

    remoteManager.rshServer << "Type 'help' for help.\n";
    remoteManager.rshServer << "Type '.' to exit debug mode.";

    *this << '\n';
}

void
DebugConsole::pressReturn(bool shift)
{
    if (!shift && input.empty()) {

        emulator.isRunning() ? emulator.pause() : emulator.stepInto();

    } else {

        Console::pressReturn(shift);
    }
}

void
DebugConsole::initCommands(Command &root)
{
    Console::initCommands(root);

    //
    // Top-level commands
    //

    {   Command::currentGroup = "Program execution";

        root.add({"goto"}, { }, { Arg::value },
                 std::pair <string, string>("g[oto]", "Goto address"),
                 [this](Arguments& argv, long value) {

            argv.empty() ? emulator.run() : cpu.jump(parseAddr(argv[0]));
        });

        root.clone("g", {"goto"});

        root.add({"step"}, { }, { },
                 std::pair <string, string>("s[tep]", "Step into the next instruction"),
                 [this](Arguments& argv, long value) {

            emulator.stepInto();
        });

        root.clone("s", {"step"});

        root.add({"next"}, { }, { },
                 std::pair <string, string>("n[next]", "Step over the next instruction"),
                 [this](Arguments& argv, long value) {

            emulator.stepOver();
        });

        root.clone("n", {"next"});

        root.add({"break"},     "Manage CPU breakpoints");

        {

            root.add({"break", ""},
                     "List all breakpoints",
                     [this](Arguments& argv, long value) {

                dump(amiga.cpu, Category::Breakpoints);
            });

            root.add({"break", "at"}, { Arg::address }, { Arg::ignores },
                     "Set a breakpoint",
                     [this](Arguments& argv, long value) {

                auto addr = parseAddr(argv[0]);
                if (IS_ODD(addr)) throw Error(VAERROR_ADDR_UNALIGNED);
                cpu.breakpoints.setAt(addr, parseNum(argv, 1, 0));
            });

            root.add({"break", "delete"}, { Arg::nr },
                     "Delete breakpoints",
                     [this](Arguments& argv, long value) {

                cpu.breakpoints.remove(parseNum(argv[0]));
            });

            root.add({"break", "toggle"}, { Arg::nr },
                     "Enable or disable breakpoints",
                     [this](Arguments& argv, long value) {

                cpu.breakpoints.toggle(parseNum(argv[0]));
            });
        }

        root.add({"watch"},     "Manage CPU watchpoints");

        {

            root.add({"watch", ""},
                     "Lists all watchpoints",
                     [this](Arguments& argv, long value) {

                dump(amiga.cpu, Category::Watchpoints);
            });

            root.add({"watch", "at"}, { Arg::address }, { Arg::ignores },
                     "Set a watchpoint at the specified address",
                     [this](Arguments& argv, long value) {

                auto addr = parseAddr(argv[0]);
                cpu.watchpoints.setAt(addr, parseNum(argv, 1, 0));
            });

            root.add({"watch", "delete"}, { Arg::address },
                     "Delete a watchpoint",
                     [this](Arguments& argv, long value) {

                cpu.watchpoints.remove(parseNum(argv[0]));
            });

            root.add({"watch", "toggle"}, { Arg::address },
                     "Enable or disable a watchpoint",
                     [this](Arguments& argv, long value) {

                cpu.watchpoints.toggle(parseNum(argv[0]));
            });
        }

        root.add({"catch"},     "Manage CPU catchpoints");

        {

            root.add({"catch", ""},
                     "List all catchpoints",
                     [this](Arguments& argv, long value) {

                dump(amiga.cpu, Category::Catchpoints);
            });

            root.add({"catch", "vector"}, { Arg::value }, { Arg::ignores },
                     "Catch an exception vector",
                     [this](Arguments& argv, long value) {

                auto nr = parseNum(argv[0]);
                if (nr < 0 || nr > 255) throw Error(VAERROR_OPT_INV_ARG, "0...255");
                cpu.catchpoints.setAt(u32(nr), parseNum(argv, 1, 0));
            });

            root.add({"catch", "interrupt"}, { Arg::value }, { Arg::ignores },
                     "Catch an interrupt",
                     [this](Arguments& argv, long value) {

                auto nr = parseNum(argv[0]);
                if (nr < 1 || nr > 7) throw Error(VAERROR_OPT_INV_ARG, "1...7");
                cpu.catchpoints.setAt(u32(nr + 24), parseNum(argv, 1, 0));
            });

            root.add({"catch", "trap"}, { Arg::value }, { Arg::ignores },
                     "Catch a trap instruction",
                     [this](Arguments& argv, long value) {

                auto nr = parseNum(argv[0]);
                if (nr < 0 || nr > 15) throw Error(VAERROR_OPT_INV_ARG, "0...15");
                cpu.catchpoints.setAt(u32(nr + 32), parseNum(argv, 1, 0));
            });

            root.add({"catch", "delete"}, { Arg::value },
                     "Delete a catchpoint",
                     [this](Arguments& argv, long value) {

                cpu.catchpoints.remove(parseNum(argv[0]));
            });

            root.add({"catch", "toggle"}, { Arg::value },
                     "Enable or disable a catchpoint",
                     [this](Arguments& argv, long value) {

                cpu.catchpoints.toggle(parseNum(argv[0]));
            });
        }

        root.add({"cbreak"},    "Manage Copper breakpoints");

        {

            root.add({"cbreak", ""},
                     "List all breakpoints",
                     [this](Arguments& argv, long value) {

                dump(copper.debugger, Category::Breakpoints);
            });

            root.add({"cbreak", "at"}, { Arg::value }, { Arg::ignores },
                     "Set a breakpoint at the specified address",
                     [this](Arguments& argv, long value) {

                auto addr = parseAddr(argv[0]);
                if (IS_ODD(addr)) throw Error(VAERROR_ADDR_UNALIGNED);
                copper.debugger.breakpoints.setAt(addr, parseNum(argv, 1, 0));
            });

            root.add({"cbreak", "delete"}, { Arg::value },
                     "Delete a breakpoint",
                     [this](Arguments& argv, long value) {

                copper.debugger.breakpoints.remove(parseNum(argv[0]));
            });

            root.add({"cbreak", "toggle"}, { Arg::value },
                     "Enable or disable a breakpoint",
                     [this](Arguments& argv, long value) {

                copper.debugger.breakpoints.toggle(parseNum(argv[0]));
            });
        }

        root.add({"cwatch"},    "Manage Copper watchpoints");

        {

            root.add({"cwatch", ""},
                     "List all watchpoints",
                     [this](Arguments& argv, long value) {

                dump(copper.debugger, Category::Watchpoints);
            });

            root.add({"cwatch", "at"}, { Arg::value }, { Arg::ignores },
                     "Set a watchpoint at the specified address",
                     [this](Arguments& argv, long value) {

                auto addr = parseAddr(argv[0]);
                if (IS_ODD(addr)) throw Error(VAERROR_ADDR_UNALIGNED);
                copper.debugger.watchpoints.setAt(addr, parseNum(argv, 1, 0));
            });

            root.add({"cwatch", "delete"}, { Arg::value },
                     "Delete a watchpoint",
                     [this](Arguments& argv, long value) {

                copper.debugger.watchpoints.remove(parseNum(argv[0]));
            });

            root.add({"cwatch", "toggle"}, { Arg::value },
                     "Enable or disable a watchpoint",
                     [this](Arguments& argv, long value) {

                copper.debugger.watchpoints.toggle(parseNum(argv[0]));
            });
        }

        root.add({"btrap"},    "Manage beamtraps");

        {

            root.add({"btrap", ""},
                     "List all beamtraps",
                     [this](Arguments& argv, long value) {

                dump(agnus.dmaDebugger, Category::Beamtraps);
            });

            root.add({"btrap", "at"}, { Arg::value, Arg::value }, { Arg::ignores },
                     "Set a beamtrap at the specified coordinate",
                     [this](Arguments& argv, long value) {

                auto v = parseNum(argv[0]);
                auto h = parseNum(argv[1]);
                agnus.dmaDebugger.beamtraps.setAt(HI_W_LO_W(v, h), parseNum(argv, 2, 0));
            });

            root.add({"btrap", "delete"}, { Arg::value },
                     "Delete a beamtrap",
                     [this](Arguments& argv, long value) {

                agnus.dmaDebugger.beamtraps.remove(parseNum(argv[0]));
            });

            root.add({"btrap", "toggle"}, { Arg::value },
                     "Enable or disable a beamtrap",
                     [this](Arguments& argv, long value) {

                agnus.dmaDebugger.beamtraps.toggle(parseNum(argv[0]));
            });
        }
    }

    {   Command::currentGroup = "Monitoring";

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
            mem.debugger.ascDump<ACCESSOR_CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16);
            retroShell << '\n' << ss << '\n';
        });

        root.add({"m"}, { }, { Arg::address },
                 std::pair<string, string>("m[.b|.w|.l]", "Dump memory"),
                 [this](Arguments& argv, long value) {

            std::stringstream ss;
            mem.debugger.memDump<ACCESSOR_CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16, value);
            retroShell << '\n' << ss << '\n';
        }, 2);

        root.clone("m.b",      {"m"}, 1);
        root.clone("m.w",      {"m"}, 2);
        root.clone("m.l",      {"m"}, 4);

        root.add({"w"}, { Arg::value }, { "{ " + Arg::address + " | " + ChipsetRegEnum::argList() + " }" },
                 std::pair<string, string>("w[.b|.w|.l]", "Write into a register or memory"),
                 [this](Arguments& argv, long value) {

            // Resolve address
            u32 addr = mem.debugger.current;

            if (argv.size() > 1) {
                try {
                    addr = 0xDFF000 + u32(parseEnum<ChipsetRegEnum>(argv[1]) << 1);
                } catch (...) {
                    addr = parseAddr(argv[1]);
                };
            }

            // Access memory
            mem.debugger.write(addr, u32(parseNum(argv[0])), value);
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
                auto addr = u32(parseNum(argv, 1, mem.debugger.current));
                auto found = mem.debugger.memSearch(pattern, addr, value == 1 ? 1 : 2);

                if (found >= 0) {

                    std::stringstream ss;
                    mem.debugger.memDump<ACCESSOR_CPU>(ss, u32(found), 1, value);
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
                auto val = u32(parseNum(argv, 2, 0));

                mem.debugger.write(addr, val, value, count);
            }
        }, 1);

        root.clone("e.b", {"e"}, "", 1);
        root.clone("e.w", {"e"}, "", 2);
        root.clone("e.l", {"e"}, "", 4);

        root.add({"?"},
                 "Inspect a component");

        {   Command::currentGroup = "Components";

            root.add({"?", "amiga"}, "Main computer");

            {

                root.add({"?", "amiga", ""},
                         "Inspects the internal state",
                         [this](Arguments& argv, long value) {

                    dump(amiga, Category::State );
                });
            }

            root.add({"?", "memory"}, "RAM and ROM");

            {

                root.add({"?", "memory", ""},
                         "Inspects the internal state",
                         [this](Arguments& argv, long value) {

                    dump(mem, Category::State );
                });

                root.add({"?", "memory", "bankmap"},
                         "Dumps the memory bank map",
                         [this](Arguments& argv, long value) {

                    dump(mem, Category::BankMap);
                });
            }

            root.add({"?", "cpu"}, "Motorola CPU");

            {

                root.add({"?", "cpu", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(cpu, Category::State );
                });
            }

            for (isize i = 0; i < 2; i++) {

                string cia = (i == 0) ? "ciaa" : "ciab";
                root.add({"?", cia}, "Complex Interface Adapter");

                {

                    root.add({"?", cia, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        if (value == 0) {
                            dump(ciaa, Category::State );
                        } else {
                            dump(ciab, Category::State );
                        }
                    }, i);

                    root.add({"?", cia, "tod"},
                             "Display the state of the 24-bit counter",
                             [this](Arguments& argv, long value) {

                        if (value == 0) {
                            dump(ciaa.tod, Category::State );
                        } else {
                            dump(ciab.tod, Category::State );
                        }
                    }, i);
                }
            }

            root.add({"?", "agnus"}, "Custom Chipset");

            {

                root.add({"?", "agnus", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(agnus, Category::State );
                });

                root.add({"?", "agnus", "beam"},
                         "Display the current beam position",
                         [this](Arguments& argv, long value) {

                    dump(amiga.agnus, Category::Beam);
                });

                root.add({"?", "agnus", "dma"},
                         "Print all scheduled DMA events",
                         [this](Arguments& argv, long value) {

                    dump(amiga.agnus, Category::Dma);
                });

                root.add({"?", "agnus", "sequencer"},
                         "Inspect the sequencer logic",
                         [this](Arguments& argv, long value) {

                    dump(amiga.agnus.sequencer, { Category::State, Category::Signals } );
                });

                root.add({"?", "agnus", "events"},
                         "Inspect the event scheduler",
                         [this](Arguments& argv, long value) {

                    dump(amiga.agnus, Category::Events);
                });
            }

            root.add({"?", "blitter"}, "Coprocessor");

            {

                root.add({"?", "blitter", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(blitter, Category::State );
                });
            }

            root.add({"?", "copper"}, "Coprocessor");

            {

                root.add({"?", "copper", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(copper, Category::State );
                });

                root.add({"?", "copper", "list"}, { Arg::value },
                         "Print the Copper list",
                         [this](Arguments& argv, long value) {

                    auto nr = parseNum(argv[0]);

                    switch (nr) {

                        case 1: dump(amiga.agnus.copper, Category::List1); break;
                        case 2: dump(amiga.agnus.copper, Category::List2); break;

                        default:
                            throw Error(VAERROR_OPT_INV_ARG, "1 or 2");
                    }
                });
            }

            root.add({"?", "paula"}, "Ports, Audio, Interrupts");

            {

                root.add({"?", "paula", "audio"},
                         "Audio unit");

                root.add({"?", "paula", "dc"},
                         "Disk controller");

                root.add({"?", "paula", "uart"},
                         "Universal Asynchronous Receiver Transmitter");

                root.add({"?", "paula", "audio", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(audioPort, Category::State );
                });

                root.add({"?", "paula", "audio", "filter"},
                         "Inspect the internal filter state",
                         [this](Arguments& argv, long value) {

                    dump(audioPort.filter, Category::State );
                });

                root.add({"?", "paula", "dc", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(diskController, Category::State );
                });

                root.add({"?", "paula", "uart", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(uart, Category::State);
                });
            }

            root.add({"?", "denise"}, "Graphics");

            {

                root.add({"?", "denise", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(denise, Category::State );
                });
            }

            root.add({"?", "rtc"}, "Real-time clock");

            {

                root.add({"?", "rtc", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(rtc, Category::State );
                });
            }

            root.add({"?", "zorro"}, "Expansion boards");

            {

                root.add({"?", "zorro", ""},
                         "List all connected boards",
                         [this](Arguments& argv, long value) {

                    dump(zorro, Category::Slots);
                });

                root.add({"?", "zorro", "board"}, { Arg::value },
                         "Inspect a specific Zorro board",
                         [this](Arguments& argv, long value) {

                    auto nr = parseNum(argv[0]);

                    if (auto board = zorro.getBoard(nr); board != nullptr) {

                        dump(*board, { Category::Properties, Category::State, Category::Stats } );
                    }
                });
            }

            root.add({"?", "controlport"}, "Control ports");

            {

                for (isize i = 1; i <= 2; i++) {

                    string nr = (i == 1) ? "1" : "2";

                    root.add({"?", "controlport", nr},
                             "Control port " + nr);

                    root.add({"?", "controlport", nr, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        if (value == 1) dump(controlPort1, Category::State);
                        if (value == 2) dump(controlPort2, Category::State);

                    }, i);
                }
            }

            root.add({"?", "serial"}, "Serial port");

            {

                root.add({"?", "serial", ""},
                         "Display the internal state",
                         [this](Arguments& argv, long value) {

                    dump(serialPort, Category::State );
                });
            }
        }
        {   Command::currentGroup = "Peripherals";

            root.add({"?", "keyboard"}, "Keyboard");

            {

                root.add({"?", "keyboard", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(keyboard, Category::State );
                });
            }

            root.add({"?", "mouse"}, "Mouse");

            {

                for (isize i = 1; i <= 2; i++) {

                    string nr = (i == 1) ? "1" : "2";

                    root.add({"?", "mouse", nr},
                             "Mouse in port " + nr);

                    root.add({"?", "mouse", nr, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        if (value == 1) dump(controlPort1.mouse, Category::State );
                        if (value == 2) dump(controlPort2.mouse, Category::State );

                    }, i);
                }
            }

            root.add({"?", "joystick"}, "Joystick");

            {

                for (isize i = 1; i <= 2; i++) {

                    string nr = (i == 1) ? "1" : "2";

                    root.add({"?", "joystick", nr},
                             "Joystick in port " + nr);

                    root.add({"?", "joystick", nr, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        if (value == 1) dump(controlPort1.joystick, Category::State);
                        if (value == 2) dump(controlPort2.joystick, Category::State);

                    }, i);
                }
            }

            for (isize i = 0; i < 4; i++) {

                string df = "df" + std::to_string(i);

                if (i == 0) {
                    root.add({"?", df}, std::pair<string,string>("df[n]", "Floppy drive n"));
                } else {
                    root.add({"?", df}, "");
                }

                {

                    root.add({"?", df, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.df[value], Category::State );

                    }, i);

                    root.add({"?", df, "disk"},
                             "Inspect the inserted disk",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.df[value], Category::Disk);

                    }, i);
                }
            }

            for (isize i = 0; i < 4; i++) {

                string hd = "hd" + std::to_string(i);

                if (i == 0) {
                    root.add({"?", hd}, std::pair<string,string>("hd[n]", "Hard drive n"));
                } else {
                    root.add({"?", hd}, "");
                }

                {

                    root.add({"?", hd, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.hd[value], Category::State );

                    }, i);

                    root.add({"?", hd, "drive"},
                             "Display hard drive parameters",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.df[value], Category::Drive);

                    }, i);

                    root.add({"?", hd, "volumes"},
                             "Display summarized volume information",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.df[value], Category::Volumes);

                    }, i);

                    root.add({"?", hd, "partitions"},
                             "Display information about all partitions",
                             [this](Arguments& argv, long value) {

                        dump(*amiga.hd[value], Category::Partitions);

                    }, i);
                }
            }
        }
        {   Command::currentGroup = "Miscellaneous";

            root.add({"?", "host"}, "Host machine");

            {

                root.add({"?", "host", ""},
                         "Display information about the host machine",
                         [this](Arguments& argv, long value) {

                    dump(host, Category::State);
                });
            }

            root.add({"?", "server"}, "Remote server");

            {

                root.add({"?", "server", ""},
                         "Display a server status summary",
                         [this](Arguments& argv, long value) {

                    dump(remoteManager, Category::Status);
                });

                root.add({"?", "server", "serial"},
                         "Serial port server");

                root.add({"?", "server", "serial", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(remoteManager.serServer, Category::State );
                });

                root.add({"?", "server", "rshell"},
                         "Retro shell server");

                root.add({"?", "server", "rshell", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(remoteManager.rshServer, Category::State );
                });

                root.add({"?", "server", "gdb"},
                         "GDB server");

                root.add({"?", "server", "gdb", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, long value) {

                    dump(remoteManager.gdbServer, Category::State );
                });
            }
        }


        root.add({"r"}, "Show registers");

        {

            root.add({"r", "cpu"},
                     "Motorola CPU",
                     [this](Arguments& argv, long value) {

                dump(cpu, Category::Registers);
            });

            root.add({"r", "ciaa"},
                     "Complex Interface Adapter A",
                     [this](Arguments& argv, long value) {

                dump(ciaa, Category::Registers);
            });

            root.add({"r", "ciab"},
                     "Complex Interface Adapter B",
                     [this](Arguments& argv, long value) {

                dump(ciab, Category::Registers);
            });

            root.add({"r", "agnus"},
                     "Custom Chipset",
                     [this](Arguments& argv, long value) {

                dump(agnus, Category::Registers);
            });

            root.add({"r", "blitter"},
                     "Coprocessor",
                     [this](Arguments& argv, long value) {

                dump(blitter, Category::Registers);
            });

            root.add({"r", "copper"},
                     "Coprocessor",
                     [this](Arguments& argv, long value) {

                dump(copper, Category::Registers);
            });

            root.add({"r", "paula"},
                     "Ports, Audio, Interrupts",
                     [this](Arguments& argv, long value) {

                dump(paula, Category::Registers);
            });

            root.add({"r", "denise"},
                     "Graphics",
                     [this](Arguments& argv, long value) {

                dump(denise, Category::Registers);
            });

            root.add({"r", "rtc"},
                     "Real-time clock",
                     [this](Arguments& argv, long value) {

                dump(rtc, Category::Registers);
            });
        }

        //
        // OSDebugger
        //

        root.add({"os"},
                 "Run the OS debugger");

        {

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

                diagBoard.setOption(OPT_DIAG_BOARD, parseBool(argv[0]));
            });
        }
    }

    //
    // Miscellaneous
    //

    {   Command::currentGroup = "Miscellaneous";

        root.add({"debug"}, "Debug variables");

        root.add({"debug", ""}, {},
                 "Display all debug variables",
                 [this](Arguments& argv, long value) {

            dump(emulator, Category::Debug);
        });

        if (debugBuild) {

            for (isize i = DebugFlagEnum::minVal; i < DebugFlagEnum::maxVal; i++) {

                root.add({"debug", DebugFlagEnum::key(i)}, { Arg::boolean },
                         DebugFlagEnum::help(i),
                         [](Arguments& argv, long value) {

                    Emulator::setDebugVariable(DebugFlag(value), int(util::parseNum(argv[0])));

                }, i);
            }

            root.add({"debug", "verbosity"}, { Arg::value },
                     "Set the verbosity level for generated debug output",
                     [](Arguments& argv, long value) {

                CoreObject::verbosity = isize(util::parseNum(argv[0]));
            });
        }

        root.add({"%"}, { Arg::value },
                 "Convert a value into different formats",
                 [this](Arguments& argv, long value) {

            std::stringstream ss;

            if (isNum(argv[0])) {
                mem.debugger.convertNumeric(ss, u32(parseNum(argv[0])));
            } else {
                mem.debugger.convertNumeric(ss, argv.front());
            }

            retroShell << '\n' << ss << '\n';
        });
    }
}

}
