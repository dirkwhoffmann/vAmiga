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
DebugConsole::initCommands(RetroShellCmd &root)
{
    Console::initCommands(root);

    //
    // Top-level commands
    //

    {   RetroShellCmd::currentGroup = "Program execution";
        
        root.add({
            
            .tokens     = { "goto" },
            .optArgs    = { Arg::value },
            .helpName   = "g[oto]",
            .help       = "Goto address",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                argv.empty() ? emulator.run() : cpu.jump(parseAddr(argv[0]));
            }
        });
        
        root.clone({"goto"}, "g", "[g]oto");
        // root.clone("g", {"goto"});
        
        root.add({
            
            .tokens     = { "step" },
            .helpName   = "s[tep]",
            .help       = "Step into the next instruction",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                emulator.stepInto();
            }
        });
        
        root.clone({"step"}, "s", "[s]tep");
        
        root.add({
            
            .tokens     = { "next" },
            .helpName   = "n[next]",
            .help       = "Step over the next instruction",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                emulator.stepOver();
            }
        });
        
        root.clone({"next"}, "n", "[n]ext");
        
        root.add({
            
            .tokens     = { "eol" },
            .helpName   = "Complete the current line",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                dmaDebugger.eolTrap = true;
                emulator.run();
            }
        });
        
        root.add({
            
            .tokens     = { "eof" },
            .help       = "Complete the current frame",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                dmaDebugger.eofTrap = true;
                emulator.run();
            }
        });
        
        //
        // Breakpoints
        //
        
        root.add({"break"},     "Manage CPU breakpoints");
        
        root.add({
            
            .tokens     = { "break", "" },
            .help       = "List all breakpoints",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                dump(amiga.cpu, Category::Breakpoints);
            }
        });
        
        root.add({
            
            .tokens     = { "break", "at" },
            .args       = { Arg::address },
            .optArgs    = { Arg::ignores },
            .help       = "Set a breakpoint",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                auto addr = parseAddr(argv[0]);
                if (IS_ODD(addr)) throw VAException(VAError::ADDR_UNALIGNED);
                cpu.breakpoints.setAt(addr, parseNum(argv, 1, 0));
            }
        });
        
        root.add({
            
            .tokens     = { "break", "delete" },
            .args       = { Arg::nr },
            .help       = "Delete breakpoints",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                cpu.breakpoints.remove(parseNum(argv[0]));
            }
        });
        
        root.add({
            
            .tokens     = { "break", "toggle" },
            .args       = { Arg::nr },
            .help       = "Enable or disable breakpoints",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                cpu.breakpoints.toggle(parseNum(argv[0]));
            }
        });
        
        //
        // Watchpoints
        //
        
        root.add({
            
            .tokens     = { "watch" },
            .help       = "Manage CPU watchpoints"
        });
        
        root.add({
            
            .tokens     = { "watch", "" },
            .help       = "Lists all watchpoints",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                dump(amiga.cpu, Category::Watchpoints);
            }
        });
        
        root.add({
            
            .tokens     = { "watch", "at" },
            .args       = { Arg::address },
            .optArgs    = { Arg::ignores },
            .help       = "Set a watchpoint at the specified address",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                auto addr = parseAddr(argv[0]);
                cpu.watchpoints.setAt(addr, parseNum(argv, 1, 0));
            }
        });
        
        root.add({
            
            .tokens     = { "watch", "delete" },
            .args       = { Arg::address },
            .help       = "Delete a watchpoint",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                cpu.watchpoints.remove(parseNum(argv[0]));
            }
        });
        
        root.add({
            
            .tokens     = { "watch", "toggle" },
            .args       = { Arg::address },
            .help       = "Enable or disable a watchpoint",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                cpu.watchpoints.toggle(parseNum(argv[0]));
            }
        });
        
        //
        // Catchpoints
        //
        
        root.add({"catch"},     "Manage CPU catchpoints");
        
        root.add({"catch", ""},
                 "List all catchpoints",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Catchpoints);
        });
        
        root.add({"catch", "vector"}, { Arg::value }, { Arg::ignores },
                 "Catch an exception vector",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 0 || nr > 255) throw VAException(VAError::OPT_INV_ARG, "0...255");
            cpu.catchpoints.setAt(u32(nr), parseNum(argv, 1, 0));
        });
        
        root.add({"catch", "interrupt"}, { Arg::value }, { Arg::ignores },
                 "Catch an interrupt",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 1 || nr > 7) throw VAException(VAError::OPT_INV_ARG, "1...7");
            cpu.catchpoints.setAt(u32(nr + 24), parseNum(argv, 1, 0));
        });
        
        root.add({"catch", "trap"}, { Arg::value }, { Arg::ignores },
                 "Catch a trap instruction",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 0 || nr > 15) throw VAException(VAError::OPT_INV_ARG, "0...15");
            cpu.catchpoints.setAt(u32(nr + 32), parseNum(argv, 1, 0));
        });
        
        root.add({"catch", "delete"}, { Arg::value },
                 "Delete a catchpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            cpu.catchpoints.remove(parseNum(argv[0]));
        });
        
        root.add({"catch", "toggle"}, { Arg::value },
                 "Enable or disable a catchpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            cpu.catchpoints.toggle(parseNum(argv[0]));
        });
        
        //
        // Copper Breakpoints
        //
        
        root.add({"cbreak"},    "Manage Copper breakpoints");
        
        root.add({"cbreak", ""},
                 "List all breakpoints",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Breakpoints);
        });
        
        root.add({"cbreak", "at"}, { Arg::value }, { Arg::ignores },
                 "Set a breakpoint at the specified address",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            if (IS_ODD(addr)) throw VAException(VAError::ADDR_UNALIGNED);
            copper.debugger.breakpoints.setAt(addr, parseNum(argv, 1, 0));
        });
        
        root.add({"cbreak", "delete"}, { Arg::value },
                 "Delete a breakpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.remove(parseNum(argv[0]));
        });
        
        root.add({"cbreak", "toggle"}, { Arg::value },
                 "Enable or disable a breakpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.toggle(parseNum(argv[0]));
        });
        
        //
        // Copper Watchpoints
        //
        
        root.add({"cwatch"},    "Manage Copper watchpoints");
        
        root.add({"cwatch", ""},
                 "List all watchpoints",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Watchpoints);
        });
        
        root.add({"cwatch", "at"}, { Arg::value }, { Arg::ignores },
                 "Set a watchpoint at the specified address",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            if (IS_ODD(addr)) throw VAException(VAError::ADDR_UNALIGNED);
            copper.debugger.watchpoints.setAt(addr, parseNum(argv, 1, 0));
        });
        
        root.add({"cwatch", "delete"}, { Arg::value },
                 "Delete a watchpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.remove(parseNum(argv[0]));
        });
        
        root.add({"cwatch", "toggle"}, { Arg::value },
                 "Enable or disable a watchpoint",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.toggle(parseNum(argv[0]));
        });
        
        //
        // Beamtraps
        //
        
        root.add({"btrap"},    "Manage beamtraps");
        
        root.add({"btrap", ""},
                 "List all beamtraps",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            dump(agnus.dmaDebugger, Category::Beamtraps);
        });
        
        root.add({"btrap", "at"}, { Arg::value, Arg::value }, { Arg::ignores },
                 "Set a beamtrap at the specified coordinate",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto v = parseNum(argv[0]);
            auto h = parseNum(argv[1]);
            agnus.dmaDebugger.beamtraps.setAt(HI_W_LO_W(v, h), parseNum(argv, 2, 0));
        });
        
        root.add({"btrap", "delete"}, { Arg::value },
                 "Delete a beamtrap",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.remove(parseNum(argv[0]));
        });
        
        root.add({"btrap", "toggle"}, { Arg::value },
                 "Enable or disable a beamtrap",
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.toggle(parseNum(argv[0]));
        });
    }

    {   RetroShellCmd::currentGroup = "Monitoring";

        root.add({"d"}, { }, { Arg::address },
                 "Disassemble instructions",
                 [this](Arguments& argv, const std::vector<isize> &values) {

            std::stringstream ss;
            cpu.disassembleRange(ss, parseAddr(argv[0], cpu.getPC0()), 16);
            retroShell << '\n' << ss << '\n';
        });

        root.add({"a"}, { }, { Arg::address },
                 "Dump memory in ASCII",
                 [this](Arguments& argv, const std::vector<isize> &values) {

            std::stringstream ss;
            mem.debugger.ascDump<Accessor::CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16);
            retroShell << '\n' << ss << '\n';
        });

        root.add({
            
            .tokens     = {"m"},
            .optArgs    = { Arg::address },
            .helpName   = "m[.b|.w|.l]",
            .help       = "Dump memory",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                std::stringstream ss;
                mem.debugger.memDump<Accessor::CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16, values[0]);
                retroShell << '\n' << ss << '\n';
            }, .values = {2}
        });

        root.clone({"m"}, "m.b", "m[.b|.w|.l]", { 1 });
        root.clone({"m"}, "m.w", "m[.b|.w|.l]", { 2 });
        root.clone({"m"}, "m.l", "m[.b|.w|.l]", { 3 });
        // root.clone("m.w",      {"m"}, 2);
        // root.clone("m.l",      {"m"}, 4);

        root.add({"w"}, { Arg::value }, { "{ " + Arg::address + " | " + RegEnum::argList() + " }" },
                 std::pair<string, string>("w[.b|.w|.l]", "Write into a register or memory"),
                 [this](Arguments& argv, const std::vector<isize> &values) {

            // Resolve address
            u32 addr = mem.debugger.current;

            if (argv.size() > 1) {
                try {
                    addr = 0xDFF000 + u32(parseEnum<RegEnum>(argv[1]) << 1);
                } catch (...) {
                    addr = parseAddr(argv[1]);
                };
            }

            // Access memory
            mem.debugger.write(addr, u32(parseNum(argv[0])), values[0]);
        }, 2);

        root.clone({"w"}, "w.b", "w[.b|.w|.l]", { 1 });
        root.clone({"w"}, "w.w", "w[.b|.w|.l]", { 2 });
        root.clone({"w"}, "w.l", "w[.b|.w|.l]", { 4 });
        // root.clone("w.b", {"w"}, "", 1);
        // root.clone("w.w", {"w"}, "", 2);
        // root.clone("w.l", {"w"}, "", 4);

        root.add({"c"}, { Arg::src, Arg::dst, Arg::count },
                 std::pair<string, string>("c[.b|.w|.l]", "Copy a chunk of memory"),
                 [this](Arguments& argv, const std::vector<isize> &values) {

            auto src = parseNum(argv[0]);
            auto dst = parseNum(argv[1]);
            auto cnt = parseNum(argv[2]) * values[0];
            
            if (src < dst) {
                
                for (isize i = cnt - 1; i >= 0; i--)
                    mem.poke8<Accessor::CPU>(u32(dst + i), mem.spypeek8<Accessor::CPU>(u32(src + i)));
                
            } else {
                
                for (isize i = 0; i <= cnt - 1; i++)
                    mem.poke8<Accessor::CPU>(u32(dst + i), mem.spypeek8<Accessor::CPU>(u32(src + i)));
            }
        }, 1);

        root.clone({"c"}, "c.b", "c[.b|.w|.l]", { 1 });
        root.clone({"c"}, "c.w", "c[.b|.w|.l]", { 2 });
        root.clone({"c"}, "c.l", "c[.b|.w|.l]", { 4 });
        /*
        root.clone("c.b", {"c"}, "", 1);
        root.clone("c.w", {"c"}, "", 2);
        root.clone("c.l", {"c"}, "", 4);
        */
        root.add({"f"}, { Arg::sequence }, { Arg::address },
                 std::pair<string, string>("f[.b|.w|.l]", "Find a sequence in memory"),
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto pattern = parseSeq(argv[0]);
            auto addr = u32(parseNum(argv, 1, mem.debugger.current));
            auto found = mem.debugger.memSearch(pattern, addr, values[0] == 1 ? 1 : 2);
            
            if (found >= 0) {
                
                std::stringstream ss;
                mem.debugger.memDump<Accessor::CPU>(ss, u32(found), 1, values[0]);
                retroShell << ss;
                
            } else {
                
                std::stringstream ss;
                ss << "Not found";
                retroShell << ss;
            }
        }, 1);

        root.clone({"f"}, "f.b", "f[.b|.w|.l]", { 1 });
        root.clone({"f"}, "f.w", "f[.b|.w|.l]", { 2 });
        root.clone({"f"}, "f.l", "f[.b|.w|.l]", { 4 });
        /*
        root.clone("f.b", {"f"}, "", 1);
        root.clone("f.w", {"f"}, "", 2);
        root.clone("f.l", {"f"}, "", 4);
        */

        root.add({"e"}, { Arg::address, Arg::count }, { Arg::value },
                 std::pair<string, string>("e[.b|.w|.l]", "Erase memory"),
                 [this](Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            auto count = parseNum(argv[1]);
            auto val = u32(parseNum(argv, 2, 0));
            
            mem.debugger.write(addr, val, values[0], count);
        }, 1);

        root.clone({"e"}, "e.b", "e[.b|.w|.l]", { 1 });
        root.clone({"e"}, "e.w", "e[.b|.w|.l]", { 2 });
        root.clone({"e"}, "e.l", "e[.b|.w|.l]", { 4 });
        /*
        root.clone("e.b", {"e"}, "", 1);
        root.clone("e.w", {"e"}, "", 2);
        root.clone("e.l", {"e"}, "", 4);
        */
        root.add({"?"},
                 "Inspect a component");

        {   RetroShellCmd::currentGroup = "Components";

            root.add({"?", "amiga"}, "Main computer");

            {

                root.add({"?", "amiga", ""},
                         "Inspects the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(amiga, Category::State );
                });
            }

            root.add({"?", "memory"}, "RAM and ROM");

            {

                root.add({"?", "memory", ""},
                         "Inspects the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(mem, Category::State );
                });

                root.add({"?", "memory", "bankmap"},
                         "Dumps the memory bank map",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(mem, Category::BankMap);
                });
            }

            root.add({"?", "cpu"}, "Motorola CPU");

            {

                root.add({"?", "cpu", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(cpu, Category::State );
                });
            }

            for (isize i = 0; i < 2; i++) {

                string cia = (i == 0) ? "ciaa" : "ciab";
                root.add({"?", cia}, "Complex Interface Adapter");

                {

                    root.add({"?", cia, ""},
                             "Inspect the internal state",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        if (values[0] == 0) {
                            dump(ciaa, Category::State );
                        } else {
                            dump(ciab, Category::State );
                        }
                    }, i);

                    root.add({"?", cia, "tod"},
                             "Display the state of the 24-bit counter",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        if (values[0] == 0) {
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
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(agnus, Category::State );
                });

                root.add({"?", "agnus", "beam"},
                         "Display the current beam position",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(amiga.agnus, Category::Beam);
                });

                root.add({"?", "agnus", "dma"},
                         "Print all scheduled DMA events",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(amiga.agnus, Category::Dma);
                });

                root.add({"?", "agnus", "sequencer"},
                         "Inspect the sequencer logic",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(amiga.agnus.sequencer, { Category::State, Category::Signals } );
                });

                root.add({"?", "agnus", "events"},
                         "Inspect the event scheduler",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(amiga.agnus, Category::Events);
                });
            }

            root.add({"?", "blitter"}, "Coprocessor");

            {

                root.add({"?", "blitter", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(blitter, Category::State );
                });
            }

            root.add({"?", "copper"}, "Coprocessor");

            {

                root.add({"?", "copper", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(copper, Category::State );
                });

                root.add({"?", "copper", "list"}, { Arg::value },
                         "Print the Copper list",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    auto nr = parseNum(argv[0]);

                    switch (nr) {

                        case 1: dump(amiga.agnus.copper, Category::List1); break;
                        case 2: dump(amiga.agnus.copper, Category::List2); break;

                        default:
                            throw VAException(VAError::OPT_INV_ARG, "1 or 2");
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
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(audioPort, Category::State );
                });

                root.add({"?", "paula", "audio", "filter"},
                         "Inspect the internal filter state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(audioPort.filter, Category::State );
                });

                root.add({"?", "paula", "dc", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(diskController, Category::State );
                });

                root.add({"?", "paula", "uart", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(uart, Category::State);
                });
            }

            root.add({"?", "denise"}, "Graphics");

            {

                root.add({"?", "denise", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(denise, Category::State );
                });
            }

            root.add({"?", "rtc"}, "Real-time clock");

            {

                root.add({"?", "rtc", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(rtc, Category::State );
                });
            }

            root.add({"?", "zorro"}, "Expansion boards");

            {

                root.add({"?", "zorro", ""},
                         "List all connected boards",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(zorro, Category::Slots);
                });

                root.add({"?", "zorro", "board"}, { Arg::value },
                         "Inspect a specific Zorro board",
                         [this](Arguments& argv, const std::vector<isize> &values) {

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
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        if (values[0] == 1) dump(controlPort1, Category::State);
                        if (values[0] == 2) dump(controlPort2, Category::State);

                    }, i);
                }
            }

            root.add({"?", "serial"}, "Serial port");

            {

                root.add({"?", "serial", ""},
                         "Display the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(serialPort, Category::State );
                });
            }
        }
        {   RetroShellCmd::currentGroup = "Peripherals";

            root.add({"?", "keyboard"}, "Keyboard");

            {

                root.add({"?", "keyboard", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

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
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        if (values[0] == 1) dump(controlPort1.mouse, Category::State );
                        if (values[0] == 2) dump(controlPort2.mouse, Category::State );

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
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        if (values[0] == 1) dump(controlPort1.joystick, Category::State);
                        if (values[0] == 2) dump(controlPort2.joystick, Category::State);

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
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.df[values[0]], Category::State );

                    }, i);

                    root.add({"?", df, "disk"},
                             "Inspect the inserted disk",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.df[values[0]], Category::Disk);

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
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.hd[values[0]], Category::State );

                    }, i);

                    root.add({"?", hd, "drive"},
                             "Display hard drive parameters",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.df[values[0]], Category::Drive);

                    }, i);

                    root.add({"?", hd, "volumes"},
                             "Display summarized volume information",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.df[values[0]], Category::Volumes);

                    }, i);

                    root.add({"?", hd, "partitions"},
                             "Display information about all partitions",
                             [this](Arguments& argv, const std::vector<isize> &values) {

                        dump(*amiga.hd[values[0]], Category::Partitions);

                    }, i);
                }
            }
        }
        {   RetroShellCmd::currentGroup = "Miscellaneous";

            root.add({"?", "thread"}, "Emulator thread");

            {

                root.add({"?", "thread", ""},
                         "Display information about the thread state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(emulator, Category::State);
                });
            }
            
            root.add({"?", "server"}, "Remote server");

            {

                root.add({"?", "server", ""},
                         "Display a server status summary",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(remoteManager, Category::Status);
                });

                root.add({"?", "server", "serial"},
                         "Serial port server");

                root.add({"?", "server", "serial", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(remoteManager.serServer, Category::State );
                });

                root.add({"?", "server", "rshell"},
                         "Retro shell server");

                root.add({"?", "server", "rshell", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(remoteManager.rshServer, Category::State );
                });

                root.add({"?", "server", "gdb"},
                         "GDB server");

                root.add({"?", "server", "gdb", ""},
                         "Inspect the internal state",
                         [this](Arguments& argv, const std::vector<isize> &values) {

                    dump(remoteManager.gdbServer, Category::State );
                });
            }
        }


        root.add({"r"}, "Show registers");

        {

            root.add({"r", "cpu"},
                     "Motorola CPU",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(cpu, Category::Registers);
            });

            root.add({"r", "ciaa"},
                     "Complex Interface Adapter A",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(ciaa, Category::Registers);
            });

            root.add({"r", "ciab"},
                     "Complex Interface Adapter B",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(ciab, Category::Registers);
            });

            root.add({"r", "agnus"},
                     "Custom Chipset",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(agnus, Category::Registers);
            });

            root.add({"r", "blitter"},
                     "Coprocessor",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(blitter, Category::Registers);
            });

            root.add({"r", "copper"},
                     "Coprocessor",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(copper, Category::Registers);
            });

            root.add({"r", "paula"},
                     "Ports, Audio, Interrupts",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(paula, Category::Registers);
            });

            root.add({"r", "denise"},
                     "Graphics",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                dump(denise, Category::Registers);
            });

            root.add({"r", "rtc"},
                     "Real-time clock",
                     [this](Arguments& argv, const std::vector<isize> &values) {

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
                     [this](Arguments& argv, const std::vector<isize> &values) {

                std::stringstream ss;
                osDebugger.dumpInfo(ss);
                retroShell << ss;
            });

            root.add({"os", "execbase"},
                     "Display information about the ExecBase struct",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                std::stringstream ss;
                osDebugger.dumpExecBase(ss);
                retroShell << ss;
            });

            root.add({"os", "interrupts"},
                     "List all interrupt handlers",
                     [this](Arguments& argv, const std::vector<isize> &values) {

                std::stringstream ss;
                osDebugger.dumpIntVectors(ss);
                retroShell << ss;
            });

            root.add({"os", "libraries"}, { }, {"<library>"},
                     "List all libraries",
                     [this](Arguments& argv, const std::vector<isize> &values) {

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

            root.add({
                
                .tokens     = {"os", "devices"},
                .optArgs    = {"<device>"},
                .help       = "List all devices",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
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
                }
            });

            root.add({
                
                .tokens     = {"os", "resources"},
                .optArgs    = {"<resource>"},
                .help       = "List all resources",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
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
                }
            });

            root.add({
                
                .tokens     = {"os", "tasks"},
                .optArgs    = {"<task>"},
                .help       = "List all tasks",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
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
                }
            });

            root.add({
                     
                .tokens     = {"os", "processes"},
                .optArgs    = {"<process>"},
                .help       = "List all processes",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
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
                }
            });

            root.add({
                
                .tokens     = {"os", "catch"},
                .args       = {"<task>"},
                .help       = "Pause emulation on task launch",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
                    diagBoard.catchTask(argv.back());
                    retroShell << "Waiting for task '" << argv.back() << "' to start...\n";
                }
            });

            root.add({
                
                .tokens     = {"os", "set"},
                .help       = "Configure the component"
            });

            root.add({
                
                .tokens     = {"os", "set", "diagboard" },
                .args       = { Arg::boolean },
                .help       = "Attach or detach the debug expansion board",
                .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                    
                    diagBoard.setOption(Opt::DIAG_BOARD, parseBool(argv[0]));
                }
            });
        }
    }

    //
    // Miscellaneous
    //

    {   RetroShellCmd::currentGroup = "Miscellaneous";

        root.add({"debug"}, "Debug variables");

        root.add({
            
            .tokens     = {"debug", ""},
            .help       = "Display all debug variables",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                dump(emulator, Category::Debug);
            }
        });
        
        if (debugBuild) {
            
            for (auto i : DebugFlagEnum::elements()) {
                
                root.add({
                    
                    .tokens     = {"debug", DebugFlagEnum::key(i)},
                    .args       = { Arg::boolean },
                    .help       = DebugFlagEnum::help(i),
                    .func       = [](Arguments& argv, const std::vector<isize> &values) {
                        
                        Emulator::setDebugVariable(DebugFlag(values[0]), int(util::parseNum(argv[0])));
                        
                    }, .values = { isize(i) }
                });
                
                root.add({"debug", "verbosity"}, { Arg::value },
                         "Set the verbosity level for generated debug output",
                         [](Arguments& argv, const std::vector<isize> &values) {
                    
                    CoreObject::verbosity = isize(util::parseNum(argv[0]));
                });
            }
        }
        root.add({
            
            .tokens     = {"%"},
            .args       = { Arg::value },
            .help       = "Convert a value into different formats",
            .func       = [this](Arguments& argv, const std::vector<isize> &values) {
                
                std::stringstream ss;
                
                if (isNum(argv[0])) {
                    mem.debugger.convertNumeric(ss, u32(parseNum(argv[0])));
                } else {
                    mem.debugger.convertNumeric(ss, argv.front());
                }
                
                retroShell << '\n' << ss << '\n';
            }
        });
    }
}
    
}
