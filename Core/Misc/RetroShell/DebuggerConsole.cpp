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
DebuggerConsole::_pause()
{
    *this << '\n' << '\n';
    exec("state");
    *this << getPrompt();
}

string
DebuggerConsole::getPrompt()
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
DebuggerConsole::welcome()
{
    storage << "RetroShell Debugger ";
    remoteManager.rshServer << "vAmiga RetroShell Remote Server ";
    *this << Amiga::build() << '\n';
    *this << '\n';
    *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
    *this << "https://github.com/dirkwhoffmann/vAmiga" << '\n';
    *this << '\n';
    
    printHelp();
}

void
DebuggerConsole::printHelp()
{
    Console::printHelp();
}

void
DebuggerConsole::pressReturn(bool shift)
{
    if (!shift && input.empty()) {

        emulator.isRunning() ? emulator.put(Cmd::PAUSE) : emulator.stepInto();

    } else {
        
        Console::pressReturn(shift);
    }
}

void
DebuggerConsole::initCommands(RetroShellCmd &root)
{
    Console::initCommands(root);
    
    //
    // Program execution
    //
    
    RetroShellCmd::currentGroup = "Program execution";
    
    root.add({
        
        .tokens = { "goto" },
        .thelp  = { "g[oto]" },
        .help   = { "Goto address" },
        .argx   = { { .name = { "address", "Memory address" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            argv.empty() ? emulator.run() : cpu.jump(parseAddr(args.at("address")));
        }
    });
    
    root.clone({ "goto" }, "g");
    
    root.add({
        
        .tokens = { "step" },
        .thelp  = { "s[tep]" },
        .help   = { "Step into the next instruction" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            emulator.stepInto();
        }
    });
    
    root.clone({ "step" }, "s");
    
    root.add({

        .tokens = { "next" },
        .thelp  = { "n[next]" },
        .help   = { "Step over the next instruction" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            emulator.stepOver();
        }
    });
    
    root.clone({ "next" }, "n");
    
    root.add({
        
        .tokens = { "eol" },
        .help   = { "Complete the current line" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dmaDebugger.eolTrap = true;
            emulator.run();
        }
    });
    
    root.add({
        
        .tokens = { "eof" },
        .help   = { "Complete the current frame" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dmaDebugger.eofTrap = true;
            emulator.run();
        }
    });
    
    //
    // Breakpoints
    //
    
    root.add({
        
        .tokens = { "break" },
        .ghelp  = { "Manage CPU breakpoints" },
        .help   = { "List all breakpoints" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "break", "at" },
        .help   = { "Set a breakpoint" },
        .argx   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args.at("address"));
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            cpu.breakpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "break", "delete" },
        .help   = { "Delete breakpoints" },
        .argx   = { { .name = { "nr", "Breakpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.breakpoints.remove(parseNum(args.at("nr")));
        }
    });
    
    root.add({
        
        .tokens = { "break", "toggle" },
        .help   = { "Enable or disable breakpoints" },
        .argx   = { { .name = { "nr", "Breakpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.breakpoints.toggle(parseNum(args.at("nr")));
        }
    });
    
    //
    // Watchpoints
    //
    
    root.add({
        
        .tokens = { "watch" },
        .ghelp  = { "Manage CPU watchpoints" },
        .help   = { "Lists all watchpoints" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "watch", "at" },
        .help   = { "Set a watchpoint at the specified address" },
        .argx   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            cpu.watchpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "delete" },
        .help   = { "Delete a watchpoint" },
        .argx   = { { .name = { "nr", "Watchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.watchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "toggle" },
        .help   = { "Enable or disable a watchpoint" },
        .argx   = { { .name = { "nr", "Watchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.watchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    //
    // Catchpoints
    //
    
    root.add({
        
        .tokens = { "catch" },
        .ghelp  = { "Manage CPU catchpoints" },
        .help   = { "List all catchpoints" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.cpu, Category::Catchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "catch", "vector" },
        .help   = { "Catch an exception vector" },
        .argx   = {
            { .name = { "vector", "Exception vector number" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "vector");
            if (nr < 0 || nr > 255) throw AppError(Fault::OPT_INV_ARG, "0...255");
            cpu.catchpoints.setAt(u32(nr), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "interrupt" },
        .help   = { "Catch an interrupt" },
        .argx   = {
            { .name = { "interrupt", "Interrupt number" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "interrupt");
            if (nr < 1 || nr > 7) throw AppError(Fault::OPT_INV_ARG, "1...7");
            cpu.catchpoints.setAt(u32(nr + 24), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "trap" },
        .help   = { "Catch a trap instruction" },
        .argx   = {
            { .name = { "trap", "Trap number" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "trap");
            if (nr < 0 || nr > 15) throw AppError(Fault::OPT_INV_ARG, "0...15");
            cpu.catchpoints.setAt(u32(nr + 32), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "delete" },
        .help   = { "Delete a catchpoint" },
        .argx   = { { .name = { "nr", "Catchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.catchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "toggle" },
        .help   = { "Enable or disable a catchpoint" },
        .argx   = { { .name = { "nr", "Catchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            cpu.catchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Copper Breakpoints
    //
    
    root.add({
        
        .tokens = { "cbreak" },
        .ghelp  = { "Manage Copper breakpoints" },
        .help   = { "List all breakpoints" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "at" },
        .help   = { "Set a breakpoint at the specified address" },
        .argx   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            copper.debugger.breakpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "delete" },
        .help   = { "Delete a breakpoint" },
        .argx   = { { .name = { "nr", "Copper breakpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "toggle" },
        .help   = { "Enable or disable a breakpoint" },
        .argx   = { { .name = { "nr", "Copper breakpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Copper Watchpoints
    //
    
    root.add({
        
        .tokens = { "cwatch" },
        .ghelp  = { "Manage Copper watchpoints" },
        .help   = { "List all watchpoints" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(copper.debugger, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "at" },
        .help   = { "Set a watchpoint at the specified address" },
        .argx   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            copper.debugger.watchpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "delete" },
        .help   = { "Delete a watchpoint" },
        .argx   = { { .name = { "nr", "Copper watchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "toggle" },
        .help   = { "Enable or disable a watchpoint" },
        .argx   = { { .name = { "nr", "Copper watchpoint number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Beamtraps
    //
    
    root.add({
        
        .tokens = { "btrap" },
        .ghelp   = { "Manage beamtraps" },
        .help   = { "List all beamtraps" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(agnus.dmaDebugger, Category::Beamtraps);
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "at" },
        .help   = { "Set a beamtrap at the specified coordinate" },
        .argx   = {
            { .name = { "x", "Vertical trigger position" }, .flags = arg::keyval },
            { .name = { "y", "Horizontal trigger position" }, .flags = arg::keyval },
            { .name = { "ignores", "Ignore count" }, .flags = arg::opt }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto h = parseNum(args, "x");
            auto v = parseNum(args, "y");
            agnus.dmaDebugger.beamtraps.setAt(HI_W_LO_W(v, h), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "delete"},
        .help   = { "Delete a beamtrap" },
        .argx   = { { .name = { "nr", "Beamtrap number" } } },
        .func    = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "toggle" },
        .help   = { "Enable or disable a beamtrap" },
        .argx   = { { .name = { "nr", "Beamtrap number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Monitoring
    //
    
    RetroShellCmd::currentGroup = "Monitoring";
    
    root.add({
        
        .tokens = { "d" },
        .help   = { "Disassemble instructions" },
        .argx   = { { .name = { "address", "Memory address" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            cpu.disassembleRange(ss, parseAddr(args, "address", cpu.getPC0()), 16);
            retroShell << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = { "a" },
        .help   = { "Dump memory in ASCII" },
        .argx   = { { .name = { "address", "Memory address" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            if (argv.size() > 0) { current = parseAddr(args, "address"); }

            std::stringstream ss;
            current += (u32)mem.debugger.ascDump<Accessor::CPU>(ss, current, 16);
            retroShell << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = { "m" },
        .thelp  = { "m[.b|.w|.l]" },
        .help   = { "Dump memory" },
        .argx   = { { .name = { "address", "Memory address" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                        
            if (argv.size() > 0) { current = parseAddr(args, "address"); }

            std::stringstream ss;
            current += (u32)mem.debugger.memDump<Accessor::CPU>(ss, current, 16, values[0]);
            retroShell << '\n' << ss << '\n';
        }, .values = {2}
    });
    
    root.clone({"m"}, "m.b", { 1 });
    root.clone({"m"}, "m.w", { 2 });
    root.clone({"m"}, "m.l", { 4 });
    
    root.add({
        
        .tokens = { "w" },
        .thelp  = { "w[.b|.w|.l]" },
        .help   = { "Write into a register or memory" },
        .argx   = {
            { .name = { "value", "Payload" } },
            { .name = { "target", "Memory address or custom register" }, .flags = arg::opt } },
        /*
        .args   = { arg::value },
        .extra  = { "{ " + arg::address + " | " + RegEnum::argList() + " }" },
        */
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            u32 addr = current;
            
            if (args.contains("target")) {

                try {
                    
                    addr = 0xDFF000 + u32(parseEnum<RegEnum>(args.at("target")) << 1);
                    mem.debugger.write(addr, u32(parseNum(args, "value")), values[0]);
                    return;
                    
                } catch (...) {
                    
                    addr = parseAddr(args, "target");
                };
            }
            
            mem.debugger.write(addr, u32(parseNum(argv[0])), values[0]);
            current = addr + u32(values[0]);
            
        }, .values = {2}
    });
    
    root.clone({"w"}, "w.b", { 1 });
    root.clone({"w"}, "w.w", { 2 });
    root.clone({"w"}, "w.l", { 4 });
    
    root.add({

        .tokens = { "c" },
        .thelp  = { "c[.b|.w|.l]" },
        .help   = { "Copy a chunk of memory" },
        .argx   = {
            { .name = { "src", "Source address" }, .flags = arg::keyval },
            { .name = { "dest", "Destination address" }, .flags = arg::keyval },
            { .name = { "count", "Number of bytes" }, .flags = arg::keyval } },

        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto src = parseNum(args.at("src"));
            auto dst = parseNum(args.at("dest"));
            auto cnt = parseNum(args.at("count")) * values[0];

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
        .thelp  = {  "f[.b|.w|.l]" },
        .help   = { "Find a sequence in memory" },
        .argx   = {
            { .name = { "sequence", "Search string" } },
            { .name = { "address", "Start address" }, .flags = arg::opt } },

        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto pattern = parseSeq(args.at("sequence"));
            auto addr = u32(parseNum(args, "address", current));
            auto found = mem.debugger.memSearch(pattern, addr, values[0] == 1 ? 1 : 2);
            
            if (found >= 0) {
                
                std::stringstream ss;
                mem.debugger.memDump<Accessor::CPU>(ss, u32(found), 1, values[0]);
                retroShell << ss;
                current = u32(found);
                
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
        .thelp  = { "e[.b|.w|.l]" },
        .help   = { "Erase memory" },
        .argx   = {
            { .name = { "address", "Start address" } },
            { .name = { "count", "Number of bytes to erase" } },
            { .name = { "value", "Replacement value" }, .flags = arg::opt } },

        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto addr = parseAddr(args.at("address"));
            auto count = parseNum(args, "count");
            auto val = u32(parseNum(args, "value", 0));

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
        .ghelp  = { "Main computer" },
        .help   = { "Inspects the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory" },
        .ghelp  = { "RAM and ROM" },
        .help   = { "Inspects the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(mem, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory", "bankmap" },
        .help   = { "Dumps the memory bank map" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(mem, Category::BankMap);
        }
    });
    
    root.add({
        
        .tokens = { "?", "cpu" },
        .ghelp  = { "Motorola CPU" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(cpu, Category::State );
        }
    });
    
    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        root.add({
            
            .tokens = { "?", cia },
            .ghelp  = { "Complex Interface Adapter" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
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
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
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
        .ghelp  = { "Custom Chipset" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(agnus, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "beam" },
        .help   = { "Display the current beam position" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Beam);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "dma" },
        .help   = { "Print all scheduled DMA events" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Dma);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "sequencer" },
        .help   = { "Inspect the sequencer logic" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.agnus.sequencer, { Category::State, Category::Signals } );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "events" },
        .help   = { "Inspect the event scheduler" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(amiga.agnus, Category::Events);
        }
    });
    
    root.add({
        
        .tokens = { "?", "blitter" },
        .ghelp  = { "Coprocessor" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(blitter, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper" },
        .ghelp  = { "Coprocessor" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(copper, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper", "list" },
        .help   = { "Print the Copper list" },
        .argx   = { { .name = { "nr", "Copper list (1 or 2)" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "nr");

            std::stringstream ss;
            
            switch (nr) {
                    
                case 1: copper.debugger.disassemble(ss, 1, true); break;
                case 2: copper.debugger.disassemble(ss, 2, true); break;
                    
                default:
                    throw AppError(Fault::OPT_INV_ARG, "1 or 2");
            }
            
            *this << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula" },
        .help   = { "" },
        .ghelp  = { "Ports, Audio, Interrupts" }
    });

    root.add({
        
        .tokens = { "?", "paula", "uart"},
        .ghelp  = { "Universal Asynchronous Receiver Transmitter" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(uart, Category::State);
        }
    });

    root.add({

        .tokens = { "?", "paula", "audio" },
        .ghelp  = { "Audio unit" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            dump(audioPort, Category::State );
        }
    });

    root.add({
        
        .tokens = { "?", "paula", "audio", "filter" },
        .help   = { "Inspect the internal filter state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(audioPort.filter, Category::State );
        }
    });

    root.add({

        .tokens = { "?", "paula", "dc" },
        .ghelp  = { "Disk controller" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(diskController, Category::State );
        }
    });

    root.add({
        
        .tokens = { "?", "denise"},
        .ghelp  = { "Graphics" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(denise, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "rtc" },
        .ghelp  = { "Real-time clock" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(rtc, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro" },
        .ghelp  = { "Expansion boards" },
        .help   = { "List all connected boards" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(zorro, Category::Slots);
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro", "board" },
        .help   = { "Inspect a specific Zorro board" },
        .argx   = { { .name = { "nr", "Board number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "nr");

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
            .ghelp  = { "Control port " + nr },
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(controlPort1, Category::State);
                if (values[0] == 2) dump(controlPort2, Category::State);
            }, .values = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "serial" },
        .ghelp  = { "Serial port" },
        .help   = { "Display the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(serialPort, Category::State );
        }
    });
    
    RetroShellCmd::currentGroup = "Peripherals";
    
    root.add({
        
        .tokens = { "?", "keyboard" },
        .ghelp   = { "Keyboard" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
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
            .ghelp  = { "Mouse in port " + nr },
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
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
            .ghelp  = { "Joystick in port " + nr },
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(controlPort1.joystick, Category::State);
                if (values[0] == 2) dump(controlPort2.joystick, Category::State);
            }, .values = {i}
        });
    }
    
    for (isize i = 0; i < 4; i++) {
        
        string df = "df" + std::to_string(i);

        /*
        if (i == 0) {
            root.add({ .tokens = { "?", df }, .help = { "Floppy drive n", "df[n]" } });
        } else {
            root.add({ .tokens = { "?", df } });
        }
        */

        root.add({
            
            .tokens = { "?", df },
            .thelp  = { "df[n]" },
            .ghelp  = { "Floppy drive n" },
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::State );
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { "?", df, "disk" },
            .help   = { "Inspect the inserted disk" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::Disk);
            }, .values = {i}
        });
    }
    
    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);

        /*
        if (i == 0) {
            root.add({ .tokens = { "?", hd }, .help = { "Hard drive n", "hd[n]" } });
        } else {
            root.add({ .tokens = { "?", hd } });
        }
        */

        root.add({
            
            .tokens = { "?", hd },
            .thelp  = { "hd[n]" },
            .ghelp  = "Hard drive n",
            .help   = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                dump(*amiga.hd[values[0]], Category::State );
            }, .values = {i}
        });
                
        root.add({
            
            .tokens = { "?", hd, "volumes" },
            .help   = { "Display summarized volume information" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                dump(*amiga.df[values[0]], Category::Volumes);
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { "?", hd, "partitions" },
            .help   = { "Display information about all partitions" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                dump(*amiga.hd[values[0]], Category::Partitions);
            }, .values = {i}
        });
    }

    RetroShellCmd::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "?", "thread" },
        .ghelp  = { "Emulator thread" },
        .help   = { "Display information about the thread state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(emulator, Category::State);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server" },
        .ghelp  = { "Remote server" },
        .help   = { "Display a server status summary" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(remoteManager, Category::Status);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "serial" },
        .ghelp  = { "Serial port server" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(remoteManager.serServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "rshell" },
        .ghelp  = { "Retro shell server" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(remoteManager.rshServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "gdb" },
        .ghelp  = { "GDB server" },
        .help   = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
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
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(cpu, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciaa" },
        .help   = { "Complex Interface Adapter A" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(ciaa, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciab" },
        .help   = { "Complex Interface Adapter B" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(ciab, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "agnus" },
        .help   = { "Custom Chipset" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(agnus, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "blitter" },
        .help   = { "Coprocessor" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(blitter, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "copper" },
        .help   = { "Coprocessor" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(copper, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "paula" },
        .help   = { "Ports, Audio, Interrupts" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(paula, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "denise" },
        .help   = { "Graphics" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(denise, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "rtc" },
        .help   = { "Real-time clock" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
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
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpInfo(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        .tokens = { "os", "execbase" },
        .help   = { "Display information about the ExecBase struct" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpExecBase(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "interrupts" },
        .help   = { "List all interrupt handlers" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpIntVectors(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "libraries" },
        .help   = { "List all libraries" },
        .argx   = { { .name = { "nr", "Library number" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            isize num;
            
            if (!args.contains("nr")) {
                osDebugger.dumpLibraries(ss);
            } else if (util::parseHex(args.at("nr"), &num)) {
                osDebugger.dumpLibrary(ss, (u32)num);
            } else {
                osDebugger.dumpLibrary(ss, args.at("nr"));
            }
            
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "devices" },
        .help   = { "List all devices" },
        .argx   = { { .name = { "nr", "Device number" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            isize num;
            
            if (!args.contains("nr")) {
                osDebugger.dumpDevices(ss);
            } else if (util::parseHex(args.at("nr"), &num)) {
                osDebugger.dumpDevice(ss, (u32)num);
            } else {
                osDebugger.dumpDevice(ss, args.at("nr"));
            }
            
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "resources" },
        .help   = { "List all resources" },
        .argx   = { { .name = { "nr", "Resource number" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            isize num;
            
            if (!args.contains("nr")) {
                osDebugger.dumpResources(ss);
            } else if (util::parseHex(args.at("nr"), &num)) {
                osDebugger.dumpResource(ss, (u32)num);
            } else {
                osDebugger.dumpResource(ss, args.at("nr"));
            }
            
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "tasks" },
        .help   = { "List all tasks" },
        .argx   = { { .name = { "nr", "Task number" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            isize num;
            
            if (!args.contains("nr")) {
                osDebugger.dumpTasks(ss);
            } else if (util::parseHex(args.at("nr"), &num)) {
                osDebugger.dumpTask(ss, (u32)num);
            } else {
                osDebugger.dumpTask(ss, args.at("nr"));
            }
            
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "processes" },
        .help   = { "List all processes" },
        .argx   = { { .name = { "nr", "Process number" }, .flags = arg::opt } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            isize num;
            
            if (!args.contains("nr")) {
                osDebugger.dumpProcesses(ss);
            } else if (util::parseHex(args.at("nr"), &num)) {
                osDebugger.dumpProcess(ss, (u32)num);
            } else {
                osDebugger.dumpProcess(ss, args.at("nr"));
            }
            
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "catch" },
        .help   = { "Pause emulation on task launch" },
        .argx   = { { .name = { "task", "Task name" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            const auto &task = args.at("task");
            diagBoard.catchTask(args.at(task));
            retroShell << "Waiting for task '" << args.at(task) << "' to start...\n";
        }
    });
    
    root.add({
        
        .tokens = { "os", "set"},
        .help   = { "Configure the component" }
    });
    
    root.add({
        
        .tokens = { "os", "set", "diagboard" },
        .help   = { "Attach or detach the debug expansion board" },
        .argx   = { { .name = { "switch", "Is the board plugged in?" }, .key = arg::boolean } },
        // .args   = { arg::boolean },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            diagBoard.setOption(Opt::DIAG_BOARD, parseBool(args.at("switch")));
        }
    });
    
    
    //
    // Miscellaneous
    //
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "debug" },
        .ghelp  = { "Debug variables" },
        .help   = { "Display all debug variables" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(emulator, Category::Debug);
        }
    });
    
    if (debugBuild) {
        
        for (auto i : DebugFlagEnum::elements()) {
            
            root.add({
                
                .tokens = { "debug", DebugFlagEnum::key(i) },
                .help   = { DebugFlagEnum::help(i) },
                .argx   = { { .name = { "level", "Debug level" } } },
                .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                    
                    Emulator::setDebugVariable(DebugFlag(values[0]), int(parseNum(args, "level")));

                }, .values = { isize(i) }
            });
        }
        
        root.add({
            
            .tokens = { "debug", "verbosity" },
            .help   = { "Set the verbosity level for generated debug output" },
            .argx   = { { .name = { "level", "Verbosity level" } } },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

                CoreObject::verbosity = isize(parseNum(args, "level"));
            }
        });
    }
    
    root.add({
        
        .tokens = {"%"},
        .help   = { "Convert a value into different formats" },
        .argx   = { { .name = { "value", "Payload" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            auto value = args.at("value");

            if (isNum(value)) {
                mem.debugger.convertNumeric(ss, (u32)parseNum(value));
            } else {
                mem.debugger.convertNumeric(ss, value);
            }
            
            retroShell << '\n' << ss << '\n';
        }
    });
}

}
