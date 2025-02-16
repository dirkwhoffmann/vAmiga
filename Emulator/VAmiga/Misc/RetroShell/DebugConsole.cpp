// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
    
    RetroShellCmd::currentGroup = "Program execution";
    
    root.add({
        
        .tokens = { "goto" },
        .extra  = { Arg::value },
        .help   = { "Goto address", "g[oto]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            argv.empty() ? emulator.run() : cpu.jump(parseAddr(argv[0]));
        }
    });
    
    root.clone({ "goto" }, "g");
    
    root.add({
        
        .tokens = { "step" },
        .help   = { "Step into the next instruction", "s[tep]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            emulator.stepInto();
        }
    });
    
    root.clone({ "step" }, "s");
    
    root.add({
        
        .tokens = { "next" },
        .help   = { "Step over the next instruction", "n[next]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            emulator.stepOver();
        }
    });
    
    root.clone({ "next" }, "n");
    
    root.add({
        
        .tokens = { "eol" },
        .help   = { "Complete the current line" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dmaDebugger.eolTrap = true;
            emulator.run();
        }
    });
    
    root.add({
        
        .tokens = { "eof" },
        .help   = { "Complete the current frame" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dmaDebugger.eofTrap = true;
            emulator.run();
        }
    });
    
    //
    // Breakpoints
    //
    
    root.add({
        
        .tokens = { "break" },
        .help   = { "Manage CPU breakpoints" }
    });
    
    root.add({
        
        .tokens = { "break", "" },
        .help   = { "List all breakpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "break", "at" },
        .args   = { Arg::address },
        .extra  = { Arg::ignores },
        .help   = { "Set a breakpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            if (IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);
            cpu.breakpoints.setAt(addr, parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "break", "delete" },
        .args   = { Arg::nr },
        .help   = { "Delete breakpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.breakpoints.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "break", "toggle" },
        .args   = { Arg::nr },
        .help   = { "Enable or disable breakpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.breakpoints.toggle(parseNum(argv[0]));
        }
    });
    
    //
    // Watchpoints
    //
    
    root.add({
        
        .tokens = { "watch" },
        .help   = { "Manage CPU watchpoints" }
    });
    
    root.add({
        
        .tokens = { "watch", "" },
        .help   = { "Lists all watchpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "watch", "at" },
        .args   = { Arg::address },
        .extra  = { Arg::ignores },
        .help   = { "Set a watchpoint at the specified address" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            cpu.watchpoints.setAt(addr, parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "delete" },
        .args   = { Arg::address },
        .help   = { "Delete a watchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.watchpoints.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "toggle" },
        .args   = { Arg::address },
        .help   = { "Enable or disable a watchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.watchpoints.toggle(parseNum(argv[0]));
        }
    });
    
    //
    // Catchpoints
    //
    
    root.add({
        
        .tokens = { "catch" },
        .help   = { "Manage CPU catchpoints" }
    });
    
    root.add({
        
        .tokens = { "catch", "" },
        .help   = { "List all catchpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Catchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "catch", "vector" },
        .args   = { Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Catch an exception vector" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 0 || nr > 255) throw CoreError(Fault::OPT_INV_ARG, "0...255");
            cpu.catchpoints.setAt(u32(nr), parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "interrupt" },
        .args   = { Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Catch an interrupt" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 1 || nr > 7) throw CoreError(Fault::OPT_INV_ARG, "1...7");
            cpu.catchpoints.setAt(u32(nr + 24), parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "trap" },
        .args   = { Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Catch a trap instruction" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            if (nr < 0 || nr > 15) throw CoreError(Fault::OPT_INV_ARG, "0...15");
            cpu.catchpoints.setAt(u32(nr + 32), parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "delete" },
        .args   = { Arg::value },
        .help   = { "Delete a catchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.catchpoints.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "toggle" },
        .args   = { Arg::value },
        .help   = { "Enable or disable a catchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            cpu.catchpoints.toggle(parseNum(argv[0]));
        }
    });
    
    
    //
    // Copper Breakpoints
    //
    
    root.add({
        
        .tokens = { "cbreak" },
        .help   = { "Manage Copper breakpoints" }
    });
    
    root.add({
        
        .tokens = { "cbreak", "" },
        .help   = { "List all breakpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "at" },
        .args   = { Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Set a breakpoint at the specified address" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            if (IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);
            copper.debugger.breakpoints.setAt(addr, parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "delete" },
        .args   = { Arg::value },
        .help   = { "Delete a breakpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "toggle" },
        .args   = { Arg::value },
        .help   = { "Enable or disable a breakpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.toggle(parseNum(argv[0]));
        }
    });
    
    
    //
    // Copper Watchpoints
    //
    
    root.add({
        
        .tokens = { "cwatch" },
        .help   = { "Manage Copper watchpoints" }
    });
    
    root.add({
        
        .tokens = { "cwatch", "" },
        .help   = { "List all watchpoints" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "at" },
        .args   = { Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Set a watchpoint at the specified address" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            if (IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);
            copper.debugger.watchpoints.setAt(addr, parseNum(argv, 1, 0));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "delete" },
        .args   = { Arg::value },
        .help   = { "Delete a watchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "toggle" },
        .args   = { Arg::value },
        .help   = { "Enable or disable a watchpoint" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.toggle(parseNum(argv[0]));
        }
    });
    
    
    //
    // Beamtraps
    //
    
    root.add({
        
        .tokens = { "btrap" },
        .help   = { "Manage beamtraps" }
    });
    
    root.add({
        
        .tokens = { "btrap", "" },
        .help   = { "List all beamtraps" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(agnus.dmaDebugger, Category::Beamtraps);
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "at" },
        .args   = { Arg::value, Arg::value },
        .extra  = { Arg::ignores },
        .help   = { "Set a beamtrap at the specified coordinate" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto v = parseNum(argv[0]);
            auto h = parseNum(argv[1]);
            agnus.dmaDebugger.beamtraps.setAt(HI_W_LO_W(v, h), parseNum(argv, 2, 0));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "delete"},
        .args   = { Arg::value },
        .help   = { "Delete a beamtrap" },
        .func    = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.remove(parseNum(argv[0]));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "toggle" },
        .args   = { Arg::value },
        .help   = { "Enable or disable a beamtrap" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.toggle(parseNum(argv[0]));
        }
    });
    
    
    //
    // Monitoring
    //
    
    RetroShellCmd::currentGroup = "Monitoring";
    
    root.add({
        
        .tokens = { "d" },
        .extra  = { Arg::address },
        .help   = { "Disassemble instructions" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            cpu.disassembleRange(ss, parseAddr(argv[0], cpu.getPC0()), 16);
            retroShell << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = { "a" },
        .extra  = { Arg::address },
        .help   = { "Dump memory in ASCII" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            mem.debugger.ascDump<Accessor::CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16);
            retroShell << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = {"m"},
        .extra  = { Arg::address },
        .help   = { "Dump memory", "m[.b|.w|.l]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            mem.debugger.memDump<Accessor::CPU>(ss, parseAddr(argv, 0, mem.debugger.current), 16, values[0]);
            retroShell << '\n' << ss << '\n';
        }, .values = {2}
    });
    
    root.clone({"m"}, "m.b", { 1 });
    root.clone({"m"}, "m.w", { 2 });
    root.clone({"m"}, "m.l", { 4 });
    
    root.add({
        
        .tokens = { "w" },
        .args   = { Arg::value },
        .extra  = { "{ " + Arg::address + " | " + RegEnum::argList() + " }" },
        .help   = { "Write into a register or memory", "w[.b|.w|.l]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        }, .values = {2}
    });
    
    root.clone({"w"}, "w.b", { 1 });
    root.clone({"w"}, "w.w", { 2 });
    root.clone({"w"}, "w.l", { 4 });
    
    root.add({
        
        .tokens = { "c" },
        .args   = { Arg::src, Arg::dst, Arg::count },
        .help   = { "Copy a chunk of memory", "c[.b|.w|.l]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        }, .values = {1}
    });
    
    root.clone({"c"}, "c.b", { 1 });
    root.clone({"c"}, "c.w", { 2 });
    root.clone({"c"}, "c.l", { 4 });
    
    root.add({
        
        .tokens = { "f" },
        .args   = { Arg::sequence },
        .extra  = { Arg::address },
        .help   = { "Find a sequence in memory", "f[.b|.w|.l]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        }, .values = {1}
    });
    
    root.clone({"f"}, "f.b", { 1 });
    root.clone({"f"}, "f.w", { 2 });
    root.clone({"f"}, "f.l", { 4 });
    
    root.add({
        
        .tokens = { "e" },
        .args   = { Arg::address, Arg::count },
        .extra  = { Arg::value },
        .help   = { "Erase memory", "e[.b|.w|.l]" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto addr = parseAddr(argv[0]);
            auto count = parseNum(argv[1]);
            auto val = u32(parseNum(argv, 2, 0));
            
            mem.debugger.write(addr, val, values[0], count);
        }, .values = {1}
    });
    
    root.clone({"e"}, "e.b", { 1 });
    root.clone({"e"}, "e.w", { 2 });
    root.clone({"e"}, "e.l", { 4 });
    
    root.add({
        
        .tokens = { "?" },
        .help   = { "Inspect a component" }
    });
    
    RetroShellCmd::currentGroup = "Components";
    
    root.add({
        
        .tokens = { "?", "amiga" },
        .help   = { "Main computer" }
    });
    
    root.add({
        
        .tokens = { "?", "amiga", "" },
        .help   = { "Inspects the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory" },
        .help   = { "RAM and ROM" }
    });
    
    root.add({
        
        .tokens = { "?", "memory", "" },
        .help   = { "Inspects the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(mem, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory", "bankmap" },
        .help   = { "Dumps the memory bank map" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(mem, Category::BankMap);
        }
    });
    
    root.add({
        
        .tokens = { "?", "cpu" },
        .help   = { "Motorola CPU" }
    });
    
    root.add({
        
        .tokens = { "?", "cpu", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(cpu, Category::State );
        }
    });
    
    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        root.add({
            
            .tokens = { "?", cia },
            .help   = { "Complex Interface Adapter" }
        });
        
        root.add({
            
            .tokens = { "?", cia, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                if (values[0] == 0) {
                    dump(ciaa, Category::State );
                } else {
                    dump(ciab, Category::State );
                }
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { "?", cia, "tod" },
            .help   = { "Display the state of the 24-bit counter" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                if (values[0] == 0) {
                    dump(ciaa.tod, Category::State );
                } else {
                    dump(ciab.tod, Category::State );
                }
            }, .values = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "agnus" },
        .help   = { "Custom Chipset" }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(agnus, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "beam" },
        .help   = { "Display the current beam position" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Beam);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "dma" },
        .help   = { "Print all scheduled DMA events" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Dma);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "sequencer" },
        .help   = { "Inspect the sequencer logic" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.agnus.sequencer, { Category::State, Category::Signals } );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "events" },
        .help   = { "Inspect the event scheduler" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Events);
        }
    });
    
    root.add({
        
        .tokens = { "?", "blitter" },
        .help   = { "Coprocessor" }
    });
    
    root.add({
        
        .tokens = { "?", "blitter", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(blitter, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper" },
        .help   = { "Coprocessor" }
    });
    
    root.add({
        
        .tokens = { "?", "copper", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper", "list" },
        .args   = { Arg::value },
        .help   = { "Print the Copper list" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            
            switch (nr) {
                    
                case 1: dump(amiga.agnus.copper, Category::List1); break;
                case 2: dump(amiga.agnus.copper, Category::List2); break;
                    
                default:
                    throw CoreError(Fault::OPT_INV_ARG, "1 or 2");
            }
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula" },
        .help   = { "Ports, Audio, Interrupts" }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "audio" },
        .help   = { "Audio unit" }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "dc" },
        .help  = { "Disk controller" }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "uart"},
        .help   = { "Universal Asynchronous Receiver Transmitter" }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "audio", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(audioPort, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "audio", "filter" },
        .help   = { "Inspect the internal filter state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(audioPort.filter, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "dc", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(diskController, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula", "uart", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(uart, Category::State);
        }
    });
    
    root.add({
        
        .tokens = { "?", "denise"},
        .help   = { "Graphics" }
    });
    
    root.add({
        
        .tokens = { "?", "denise", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(denise, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "rtc" },
        .help   = { "Real-time clock" },
    });
    
    root.add({
        
        .tokens = { "?", "rtc", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(rtc, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro" },
        .help   = { "Expansion boards" }
    });
    
    root.add({
        
        .tokens = { "?", "zorro", "" },
        .help   = { "List all connected boards" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(zorro, Category::Slots);
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro", "board" },
        .args   = { Arg::value },
        .help   = { "Inspect a specific Zorro board" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto nr = parseNum(argv[0]);
            
            if (auto board = zorro.getBoard(nr); board != nullptr) {
                
                dump(*board, { Category::Properties, Category::State, Category::Stats } );
            }
        }
    });
    
    root.add({
        
        .tokens = { "?", "controlport" },
        .help   = { "Control ports" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "controlport", nr },
            .help   = { "Control port " + nr }
        });
        
        root.add({
            
            .tokens = { "?", "controlport", nr, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(controlPort1, Category::State);
                if (values[0] == 2) dump(controlPort2, Category::State);
            }, .values = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "serial" },
        .help   = { "Serial port" }
    });
    
    
    root.add({
        
        .tokens = { "?", "serial", "" },
        .help   = { "Display the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(serialPort, Category::State );
        }
    });
    
    RetroShellCmd::currentGroup = "Peripherals";
    
    root.add({
        
        .tokens = { "?", "keyboard" },
        .help   = { "Keyboard" }
    });
    
    root.add({
        
        .tokens = { "?", "keyboard", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(keyboard, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "mouse" },
        .help   = { "Mouse" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "mouse", nr },
            .help   = { "Mouse in port " + nr }
        });
        
        root.add({
            
            .tokens = { "?", "mouse", nr, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(controlPort1.mouse, Category::State );
                if (values[0] == 2) dump(controlPort2.mouse, Category::State );
            }, .values = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "joystick" },
        .help   = { "Joystick" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "joystick", nr },
            .help   = { "Joystick in port " + nr }
        });
        
        root.add({
            
            .tokens = { "?", "joystick", nr, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(controlPort1.joystick, Category::State);
                if (values[0] == 2) dump(controlPort2.joystick, Category::State);
            }, .values = {i}
        });
    }
    
    for (isize i = 0; i < 4; i++) {
        
        string df = "df" + std::to_string(i);
        
        if (i == 0) {
            root.add({ .tokens = { "?", df }, .help = { "Floppy drive n", "df[n]" } });
        } else {
            root.add({ .tokens = { "?", df } });
        }
        
        root.add({
            
            .tokens = { "?", df, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::State );
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { "?", df, "disk" },
            .help   = { "Inspect the inserted disk" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::Disk);
            }, .values = {i}
        });
    }
    
    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);
        
        if (i == 0) {
            root.add({ .tokens = { "?", hd }, .help = { "Hard drive n", "hd[n]" } });
        } else {
            root.add({ .tokens = { "?", hd } });
        }
        
        root.add({
            
            .tokens = { "?", hd, "" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                dump(*amiga.hd[values[0]], Category::State );
            }, .values = {i}
        });
                
        root.add({
            
            .tokens = { "?", hd, "volumes" },
            .help   = { "Display summarized volume information" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::Volumes);
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { "?", hd, "partitions" },
            .help   = { "Display information about all partitions" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                dump(*amiga.hd[values[0]], Category::Partitions);
            }, .values = {i}
        });
    }
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "?", "thread" },
        .help   = { "Emulator thread" }
    });
    
    root.add({
        
        .tokens = { "?", "thread", "" },
        .help   = { "Display information about the thread state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(emulator, Category::State);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server" },
        .help   = { "Remote server" }
    });
    
    root.add({
        
        .tokens = { "?", "server", "" },
        .help   = { "Display a server status summary" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(remoteManager, Category::Status);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "serial" },
        .help   = { "Serial port server" }
    });
    
    root.add({
        
        .tokens = { "?", "server", "serial", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(remoteManager.serServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "rshell" },
        .help   = { "Retro shell server" }
    });
    
    root.add({
        
        .tokens = { "?", "server", "rshell", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(remoteManager.rshServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "gdb" },
        .help   = { "GDB server" }
    });
    
    root.add({
        
        .tokens = { "?", "server", "gdb", "" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(remoteManager.gdbServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "r" },
        .help   = { "Show registers" }
    });
    
    root.add({
        
        .tokens = { "r", "cpu" },
        .help   = { "Motorola CPU" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(cpu, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciaa" },
        .help   = { "Complex Interface Adapter A" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(ciaa, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciab" },
        .help   = { "Complex Interface Adapter B" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(ciab, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "agnus" },
        .help   = { "Custom Chipset" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(agnus, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "blitter" },
        .help   = { "Coprocessor" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(blitter, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "copper" },
        .help   = { "Coprocessor" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(copper, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "paula" },
        .help   = { "Ports, Audio, Interrupts" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(paula, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "denise" },
        .help   = { "Graphics" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(denise, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "rtc" },
        .help   = { "Real-time clock" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(rtc, Category::Registers);
        }
    });
    
    //
    // OSDebugger
    //
    
    root.add({
        
        .tokens = { "os" },
        .help   = { "Run the OS debugger" }
    });
    
    root.add({
             
        .tokens = { "os", "info" },
        .help   = { "Display basic system information" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpInfo(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        .tokens = { "os", "execbase" },
        .help   = { "Display information about the ExecBase struct" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpExecBase(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "interrupts" },
        .help   = { "List all interrupt handlers" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpIntVectors(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "libraries" },
        .extra  = { "<library>" },
        .help   = { "List all libraries" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        }
    });
    
    root.add({
        
        .tokens = { "os", "devices" },
        .extra  = { "<device>" },
        .help   = { "List all devices" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        
        .tokens = { "os", "resources" },
        .extra  = { "<resource>" },
        .help   = { "List all resources" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        
        .tokens = { "os", "tasks" },
        .extra  = { "<task>" },
        .help   = { "List all tasks" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        
        .tokens = { "os", "processes" },
        .extra  = { "<process>" },
        .help   = { "List all processes" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
        
        .tokens = { "os", "catch" },
        .args   = { "<task>" },
        .help   = { "Pause emulation on task launch" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            diagBoard.catchTask(argv.back());
            retroShell << "Waiting for task '" << argv.back() << "' to start...\n";
        }
    });
    
    root.add({
        
        .tokens = { "os", "set"},
        .help   = { "Configure the component" }
    });
    
    root.add({
        
        .tokens = { "os", "set", "diagboard" },
        .args   = { Arg::boolean },
        .help   = { "Attach or detach the debug expansion board" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            diagBoard.setOption(Opt::DIAG_BOARD, parseBool(argv[0]));
        }
    });
    
    
    //
    // Miscellaneous
    //
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "debug" },
        .help   = { "Debug variables" }
    });
    
    root.add({
        
        .tokens = { "debug", "" },
        .help   = { "Display all debug variables" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(emulator, Category::Debug);
        }
    });
    
    if (debugBuild) {
        
        for (auto i : DebugFlagEnum::elements()) {
            
            root.add({
                
                .tokens = { "debug", DebugFlagEnum::key(i) },
                .args   = { Arg::boolean },
                .help   = { DebugFlagEnum::help(i) },
                .func   = [] (Arguments& argv, const std::vector<isize> &values) {
                    
                    Emulator::setDebugVariable(DebugFlag(values[0]), int(util::parseNum(argv[0])));
                    
                }, .values = { isize(i) }
            });
            
            root.add({
                
                .tokens = { "debug", "verbosity" },
                .args   = { Arg::value },
                .help   = { "Set the verbosity level for generated debug output" },
                .func   = [] (Arguments& argv, const std::vector<isize> &values) {
                    
                    CoreObject::verbosity = isize(util::parseNum(argv[0]));
                }
            });
        }
    }
    root.add({
        
        .tokens = {"%"},
        .args   = { Arg::value },
        .help   = { "Convert a value into different formats" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
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
