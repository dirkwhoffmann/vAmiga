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
    if (retroShell.inDebugShell()) {

        *this << '\n';
        exec("state");
        *this << getPrompt();
    }
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
    Console::welcome();
}

void
DebuggerConsole::summary()
{
    std::stringstream ss;

    // ss << "RetroShell Debugger" << std::endl << std::endl;
    amiga.dump(Category::Current, ss);

    *this << vspace{1};
    string line;
    while(std::getline(ss, line)) { *this << "    " << line << '\n'; }
    // *this << ss;
    *this << vspace{1};
}

void
DebuggerConsole::printHelp(isize tab)
{
    Console::printHelp(tab);
}

void
DebuggerConsole::pressReturn(bool shift)
{
    if (emulator.isPaused() && !shift && input.empty()) {

        emulator.stepInto();

    } else {
        
        Console::pressReturn(shift);
    }
}

void
DebuggerConsole::initCommands(RSCommand &root)
{
    Console::initCommands(root);

    //
    // Console management
    //

    root.add({

        .tokens = { "." },
        .chelp  = { "Switch to the next console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterNavigator();
        }
    });

    root.add({

        .tokens = { ".." },
        .chelp  = { "Switch to the previous console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterCommander();
        }
    });


    //
    // Program execution
    //
    
    RSCommand::currentGroup = "Program execution";

    root.add({ .tokens = { "p[ause]" }, .ghelp  = { "Pause emulation" }, .chelp  = { "p or pause" } });

    root.add({

        .tokens = { "pause" },
        .chelp  = { "Pause emulation" },
        .flags  = rs::shadowed,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (emulator.isRunning()) emulator.put(Cmd::PAUSE);
        }
    });

    root.clone({ "pause" }, "p");

    root.add({ .tokens = { "g[oto]" }, .ghelp  = { "Goto address" }, .chelp  = { "g or goto" } });

    root.add({
        
        .tokens = { "goto" },
        .chelp  = { "Goto address" },
        .flags  = rs::shadowed,
        .args   = { { .name = { "address", "Memory address" }, .flags = rs::opt } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            args.contains("address") ? cpu.jump(parseAddr(args.at("address"))) : emulator.run();
        }
    });
    
    root.clone({ "goto" }, "g");

    root.add({ .tokens = { "s[tep]" }, .ghelp  = { "Step into the next instruction" }, .chelp  = { "s or step" } });

    root.add({
        
        .tokens = { "step" },
        .chelp  = { "Step into the next instruction" },
        .flags  = rs::shadowed,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            emulator.stepInto();
        }
    });

    root.clone({ "step" }, "s");

    root.add({ .tokens = { "n[next]" }, .ghelp  = { "Step over the next instruction" }, .chelp  = { "n or next" } });

    root.add({

        .tokens = { "next" },
        .chelp  = { "Step over the next instruction" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            emulator.stepOver();
        }
    });
    
    root.clone({ "next" }, "n");
    
    root.add({
        
        .tokens = { "eol" },
        .chelp  = { "Complete the current line" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dmaDebugger.eolTrap = true;
            emulator.run();
        }
    });
    
    root.add({
        
        .tokens = { "eof" },
        .chelp  = { "Complete the current frame" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "List all breakpoints" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.cpu, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "break", "at" },
        .chelp  = { "Set a breakpoint" },
        .args   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args.at("address"));
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            cpu.breakpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "break", "delete" },
        .chelp  = { "Delete breakpoints" },
        .args   = { { .name = { "nr", "Breakpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.breakpoints.remove(parseNum(args.at("nr")));
        }
    });
    
    root.add({
        
        .tokens = { "break", "toggle" },
        .chelp  = { "Enable or disable breakpoints" },
        .args   = { { .name = { "nr", "Breakpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.breakpoints.toggle(parseNum(args.at("nr")));
        }
    });
    
    //
    // Watchpoints
    //
    
    root.add({
        
        .tokens = { "watch" },
        .ghelp  = { "Manage CPU watchpoints" },
        .chelp  = { "Lists all watchpoints" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.cpu, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "watch", "at" },
        .chelp  = { "Set a watchpoint at the specified address" },
        .args   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            cpu.watchpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "delete" },
        .chelp  = { "Delete a watchpoint" },
        .args   = { { .name = { "nr", "Watchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.watchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "watch", "toggle" },
        .chelp  = { "Enable or disable a watchpoint" },
        .args   = { { .name = { "nr", "Watchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.watchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    //
    // Catchpoints
    //
    
    root.add({
        
        .tokens = { "catch" },
        .ghelp  = { "Manage CPU catchpoints" },
        .chelp  = { "List all catchpoints" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.cpu, Category::Catchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "catch", "vector" },
        .chelp  = { "Catch an exception vector" },
        .args   = {
            { .name = { "vector", "Exception vector number" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "vector");
            if (nr < 0 || nr > 255) throw AppError(Fault::OPT_INV_ARG, "0...255");
            cpu.catchpoints.setAt(u32(nr), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "interrupt" },
        .chelp  = { "Catch an interrupt" },
        .args   = {
            { .name = { "interrupt", "Interrupt number" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "interrupt");
            if (nr < 1 || nr > 7) throw AppError(Fault::OPT_INV_ARG, "1...7");
            cpu.catchpoints.setAt(u32(nr + 24), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "trap" },
        .chelp  = { "Catch a trap instruction" },
        .args   = {
            { .name = { "trap", "Trap number" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "trap");
            if (nr < 0 || nr > 15) throw AppError(Fault::OPT_INV_ARG, "0...15");
            cpu.catchpoints.setAt(u32(nr + 32), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "delete" },
        .chelp  = { "Delete a catchpoint" },
        .args   = { { .name = { "nr", "Catchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.catchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "catch", "toggle" },
        .chelp  = { "Enable or disable a catchpoint" },
        .args   = { { .name = { "nr", "Catchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            cpu.catchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Copper Breakpoints
    //
    
    root.add({
        
        .tokens = { "cbreak" },
        .ghelp  = { "Manage Copper breakpoints" },
        .chelp  = { "List all breakpoints" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, copper.debugger, Category::Breakpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "at" },
        .chelp  = { "Set a breakpoint at the specified address" },
        .args   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            copper.debugger.breakpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "delete" },
        .chelp  = { "Delete a breakpoint" },
        .args   = { { .name = { "nr", "Copper breakpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "cbreak", "toggle" },
        .chelp  = { "Enable or disable a breakpoint" },
        .args   = { { .name = { "nr", "Copper breakpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            copper.debugger.breakpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Copper Watchpoints
    //
    
    root.add({
        
        .tokens = { "cwatch" },
        .ghelp  = { "Manage Copper watchpoints" },
        .chelp  = { "List all watchpoints" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, copper.debugger, Category::Watchpoints);
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "at" },
        .chelp  = { "Set a watchpoint at the specified address" },
        .args   = {
            { .name = { "address", "Memory address" } },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto addr = parseAddr(args, "address");
            if (IS_ODD(addr)) throw AppError(Fault::ADDR_UNALIGNED);
            copper.debugger.watchpoints.setAt(addr, parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "delete" },
        .chelp  = { "Delete a watchpoint" },
        .args   = { { .name = { "nr", "Copper watchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "cwatch", "toggle" },
        .chelp  = { "Enable or disable a watchpoint" },
        .args   = { { .name = { "nr", "Copper watchpoint number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            copper.debugger.watchpoints.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Beamtraps
    //
    
    root.add({
        
        .tokens = { "btrap" },
        .ghelp  = { "Manage beamtraps" },
        .chelp  = { "List all beamtraps" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, agnus.dmaDebugger, Category::Beamtraps);
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "at" },
        .chelp  = { "Set a beamtrap at the specified coordinate" },
        .args   = {
            { .name = { "x", "Vertical trigger position" }, .flags = rs::keyval },
            { .name = { "y", "Horizontal trigger position" }, .flags = rs::keyval },
            { .name = { "ignores", "Ignore count" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto h = parseNum(args, "x");
            auto v = parseNum(args, "y");
            agnus.dmaDebugger.beamtraps.setAt(HI_W_LO_W(v, h), parseNum(args, "ignores", 0));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "delete"},
        .chelp  = { "Delete a beamtrap" },
        .args   = { { .name = { "nr", "Beamtrap number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.remove(parseNum(args, "nr"));
        }
    });
    
    root.add({
        
        .tokens = { "btrap", "toggle" },
        .chelp  = { "Enable or disable a beamtrap" },
        .args   = { { .name = { "nr", "Beamtrap number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            agnus.dmaDebugger.beamtraps.toggle(parseNum(args, "nr"));
        }
    });
    
    
    //
    // Monitoring
    //
    
    RSCommand::currentGroup = "Monitoring";
    
    root.add({
        
        .tokens = { "d" },
        .chelp  = { "Disassemble instructions" },
        .args   = { { .name = { "address", "Memory address" }, .flags = rs::opt } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            cpu.disassembleRange(ss, parseAddr(args, "address", cpu.getPC0()), 16);
            retroShell << '\n' << ss << '\n';
        }
    });
    
    root.add({
        
        .tokens = { "a" },
        .chelp  = { "Dump memory in ASCII" },
        .args   = { { .name = { "address", "Memory address" }, .flags = rs::opt } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (args.contains("address")) { current = parseAddr(args, "address"); }

            std::stringstream ss;
            current += (u32)mem.debugger.ascDump<Accessor::CPU>(ss, current, 16);
            retroShell << '\n' << ss << '\n';
        }
    });

    root.add({

        .tokens = { "m[.b|.w|.l]" },
        .ghelp  = { "Dump memory" },
        .chelp  = { "Commands: m, m.b, m.w, m.l" }
    });

    root.add({
        
        .tokens = { "m" },
        .chelp  = { "Dump memory" },
        .flags  = rs::hidden,
        .args   = { { .name = { "address", "Memory address" }, .flags = rs::opt } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                        
            if (args.contains("address")) { current = parseAddr(args, "address"); }

            std::stringstream ss;
            current += (u32)mem.debugger.memDump<Accessor::CPU>(ss, current, 16, values[0]);
            retroShell << '\n' << ss << '\n';
        }, .payload = {2}
    });
    
    root.clone({"m"}, "m.b", { 1 });
    root.clone({"m"}, "m.w", { 2 });
    root.clone({"m"}, "m.l", { 4 });

    root.add({

        .tokens = { "w[.b|.w|.l]" },
        .ghelp  = { "Write into a register or memory" },
        .chelp  = { "Commands: w, w.b, w.w, w.l" }
    });

    root.add({
        
        .tokens = { "w" },
        .chelp  = { "Write into a register or memory" },
        .flags  = rs::hidden,
        .args   = {
            { .name = { "value", "Payload" } },
            { .name = { "target", "Memory address or custom register" }, .flags = rs::opt } },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

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
            
            mem.debugger.write(addr, u32(parseNum(args.at("value"))), values[0]);
            current = addr + u32(values[0]);
            
        }, .payload = {2}
    });
    
    root.clone({"w"}, "w.b", { 1 });
    root.clone({"w"}, "w.w", { 2 });
    root.clone({"w"}, "w.l", { 4 });

    root.add({

        .tokens = { "c[.b|.w|.l]" },
        .ghelp  = { "Copy a chunk of memory" },
        .chelp  = { "Commands: c, c.b, c.w, c.l" }
    });

    root.add({

        .tokens = { "c" },
        .chelp  = { "Copy a chunk of memory" },
        .flags  = rs::hidden,
        .args   = {
            { .name = { "src", "Source address" }, .flags = rs::keyval },
            { .name = { "dest", "Destination address" }, .flags = rs::keyval },
            { .name = { "count", "Number of bytes" }, .flags = rs::keyval } },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        }, .payload = {1}
    });
    
    root.clone({"c"}, "c.b", { 1 });
    root.clone({"c"}, "c.w", { 2 });
    root.clone({"c"}, "c.l", { 4 });

    root.add({

        .tokens = { "f[.b|.w|.l]" },
        .ghelp  = { "Find a sequence in memory" },
        .chelp  = { "Commands: f, f.b, f.w, f.l" }
    });

    root.add({
        
        .tokens = { "f" },
        .chelp  = { "Find a sequence in memory" },
        .flags  = rs::hidden,
        .args   = {
            { .name = { "sequence", "Search string" } },
            { .name = { "address", "Start address" }, .flags = rs::opt } },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        }, .payload = {1}
    });
    
    root.clone({"f"}, "f.b", { 1 });
    root.clone({"f"}, "f.w", { 2 });
    root.clone({"f"}, "f.l", { 4 });

    root.add({

        .tokens = { "e[.b|.w|.l]" },
        .ghelp  = { "Erase memory" },
        .chelp  = { "Commands: e, e.b, e.w, e.l" }
    });

    root.add({
        
        .tokens = { "e" },
        .chelp  = { "Erase memory" },
        .flags  = rs::hidden,
        .args   = {
            { .name = { "address", "Start address" } },
            { .name = { "count", "Number of bytes to erase" } },
            { .name = { "value", "Replacement value" }, .flags = rs::opt } },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto addr = parseAddr(args.at("address"));
            auto count = parseNum(args, "count");
            auto val = u32(parseNum(args, "value", 0));

            mem.debugger.write(addr, val, values[0], count);
        }, .payload = {1}
    });
    
    root.clone({"e"}, "e.b", { 1 });
    root.clone({"e"}, "e.w", { 2 });
    root.clone({"e"}, "e.l", { 4 });
    
    root.add({
        
        .tokens = { "?" },
        .ghelp  = { "Inspect a component" }
    });
    
    RSCommand::currentGroup = "Components";
    
    root.add({
        
        .tokens = { "?", "amiga" },
        .chelp  = { "Inspects the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory" },
        .ghelp  = { "RAM and ROM" },
        .chelp  = { "Inspects the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, mem, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "memory", "bankmap" },
        .chelp  = { "Dumps the memory bank map" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, mem, Category::BankMap);
        }
    });
    
    root.add({
        
        .tokens = { "?", "cpu" },
        .ghelp  = { "Motorola CPU" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, cpu, Category::State );
        }
    });
    
    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        root.add({
            
            .tokens = { "?", cia },
            .ghelp  = { "Complex Interface Adapter" },
            .chelp  = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 0) {
                    dump(os, ciaa, Category::State );
                } else {
                    dump(os, ciab, Category::State );
                }
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { "?", cia, "tod" },
            .chelp  = { "Display the state of the 24-bit counter" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 0) {
                    dump(os, ciaa.tod, Category::State );
                } else {
                    dump(os, ciab.tod, Category::State );
                }
            }, .payload = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "agnus" },
        .ghelp  = { "Custom Chipset" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, agnus, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "beam" },
        .chelp  = { "Display the current beam position" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.agnus, Category::Beam);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "dma" },
        .chelp  = { "Print all scheduled DMA events" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.agnus, Category::Dma);
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "sequencer" },
        .chelp  = { "Inspect the sequencer logic" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.agnus.sequencer, { Category::State, Category::Signals } );
        }
    });
    
    root.add({
        
        .tokens = { "?", "agnus", "events" },
        .chelp  = { "Inspect the event scheduler" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, amiga.agnus, Category::Events);
        }
    });
    
    root.add({
        
        .tokens = { "?", "blitter" },
        .ghelp  = { "Coprocessor" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, blitter, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper" },
        .ghelp  = { "Coprocessor" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, copper, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "copper", "list" },
        .chelp  = { "Print the Copper list" },
        .args   = { { .name = { "nr", "Copper list (1 or 2)" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "nr");

            switch (nr) {
                    
                case 1: copper.debugger.disassemble(os, 1, true); break;
                case 2: copper.debugger.disassemble(os, 2, true); break;

                default:
                    throw AppError(Fault::OPT_INV_ARG, "1 or 2");
            }
        }
    });
    
    root.add({
        
        .tokens = { "?", "paula" },
        .ghelp  = { "Ports, Audio, Interrupts" }
    });

    root.add({
        
        .tokens = { "?", "paula", "uart"},
        .ghelp  = { "Universal Asynchronous Receiver Transmitter" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, uart, Category::State);
        }
    });

    root.add({

        .tokens = { "?", "paula", "audio" },
        .ghelp  = { "Audio unit" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            dump(os, audioPort, Category::State );
        }
    });

    root.add({
        
        .tokens = { "?", "paula", "audio", "filter" },
        .chelp  = { "Inspect the internal filter state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, audioPort.filter, Category::State );
        }
    });

    root.add({

        .tokens = { "?", "paula", "dc" },
        .ghelp  = { "Disk controller" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, diskController, Category::State );
        }
    });

    root.add({
        
        .tokens = { "?", "denise"},
        .ghelp  = { "Graphics" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, denise, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "rtc" },
        .ghelp  = { "Real-time clock" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, rtc, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro" },
        .ghelp  = { "Expansion boards" },
        .chelp  = { "List all connected boards" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, zorro, Category::Slots);
        }
    });
    
    root.add({
        
        .tokens = { "?", "zorro", "board" },
        .chelp  = { "Inspect a specific Zorro board" },
        .args   = { { .name = { "nr", "Board number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto nr = parseNum(args, "nr");

            if (auto board = zorro.getBoard(nr); board != nullptr) {
                
                dump(os, *board, { Category::Properties, Category::State, Category::Stats } );
            }
        }
    });
    
    root.add({
        
        .tokens = { "?", "controlport" },
        .chelp  = { "Control ports" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "controlport", nr },
            .ghelp  = { "Control port " + nr },
            .chelp  = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(os, controlPort1, Category::State);
                if (values[0] == 2) dump(os, controlPort2, Category::State);
            }, .payload = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "serial" },
        .ghelp  = { "Serial port" },
        .chelp  = { "Display the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, serialPort, Category::State );
        }
    });
    
    RSCommand::currentGroup = "Peripherals";
    
    root.add({
        
        .tokens = { "?", "keyboard" },
        .ghelp  = { "Keyboard" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, keyboard, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "mouse" },
        .ghelp  = { "Mouse" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "mouse", nr },
            .ghelp  = { "Mouse in port " + nr },
            .chelp  = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(os, controlPort1.mouse, Category::State );
                if (values[0] == 2) dump(os, controlPort2.mouse, Category::State );
            }, .payload = {i}
        });
    }
    
    root.add({
        
        .tokens = { "?", "joystick" },
        .chelp  = { "Joystick" }
    });
    
    for (isize i = 1; i <= 2; i++) {
        
        string nr = (i == 1) ? "1" : "2";
        
        root.add({
            
            .tokens = { "?", "joystick", nr },
            .ghelp  = { "Joystick in port " + nr },
            .chelp  = { "Inspect the internal state" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                if (values[0] == 1) dump(os, controlPort1.joystick, Category::State);
                if (values[0] == 2) dump(os, controlPort2.joystick, Category::State);
            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "?", "df[n]" },
        .ghelp  = { "Floppy drive n" },
        .chelp  = { "? df0, ? df1, ? df1, or ? df2" }
    });

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({
            
            .tokens = { "?", df },
            .ghelp  = { "Floppy drive n" },
            .chelp  = { "Inspect the internal state" },
            .flags  = rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                dump(os, *amiga.df[values[0]], Category::State );
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { "?", df, "disk" },
            .chelp  = { "Inspect the inserted disk" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                dump(os, *amiga.df[values[0]], Category::Disk);
            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "?", "hd[n]" },
        .ghelp  = { "Hard drive n" },
        .chelp  = { "? hd0, ? hd1, ? hd2, or ? hd3" }
    });

    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);

        root.add({
            
            .tokens = { "?", hd },
            .ghelp  = "Hard drive n",
            .chelp  = { "Inspect the internal state" },
            .flags  = rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                dump(os, *amiga.hd[values[0]], Category::State );
            }, .payload = {i}
        });
                
        root.add({
            
            .tokens = { "?", hd, "volumes" },
            .chelp  = { "Display summarized volume information" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                dump(os, *amiga.df[values[0]], Category::Volumes);
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { "?", hd, "partitions" },
            .chelp  = { "Display information about all partitions" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                dump(os, *amiga.hd[values[0]], Category::Partitions);
            }, .payload = {i}
        });
    }

    RSCommand::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "?", "thread" },
        .ghelp  = { "Emulator thread" },
        .chelp  = { "Display information about the thread state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, emulator, Category::State);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server" },
        .ghelp  = { "Remote server" },
        .chelp  = { "Display a server status summary" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, remoteManager, Category::Status);
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "serial" },
        .ghelp  = { "Serial port server" },
        .chelp  = { "Inspect the internal state" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, remoteManager.serServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "rshell" },
        .ghelp  = { "Retro shell server" },
        .chelp  = { "Inspect the internal state" },
        
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, remoteManager.rshServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "?", "server", "gdb" },
        .ghelp  = { "GDB server" },
        .chelp  = { "Inspect the internal state" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, remoteManager.gdbServer, Category::State );
        }
    });
    
    root.add({
        
        .tokens = { "r" },
        .ghelp  = { "Show registers" }
    });
    
    root.add({
        
        .tokens = { "r", "cpu" },
        .chelp  = { "Motorola CPU" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, cpu, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciaa" },
        .chelp  = { "Complex Interface Adapter A" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, ciaa, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "ciab" },
        .chelp  = { "Complex Interface Adapter B" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, ciab, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "agnus" },
        .chelp  = { "Custom Chipset" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, agnus, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "blitter" },
        .chelp  = { "Coprocessor" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, blitter, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "copper" },
        .chelp  = { "Coprocessor" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, copper, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "paula" },
        .chelp  = { "Ports, Audio, Interrupts" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, paula, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "denise" },
        .chelp  = { "Graphics" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, denise, Category::Registers);
        }
    });
    
    root.add({
        
        .tokens = { "r", "rtc" },
        .chelp  = { "Real-time clock" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, rtc, Category::Registers);
        }
    });
    
    //
    // OSDebugger
    //
    
    root.add({
        
        .tokens = { "os" },
        .ghelp  = { "Run the OS debugger" }
    });
    
    root.add({
             
        .tokens = { "os", "info" },
        .chelp  = { "Display basic system information" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpInfo(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        .tokens = { "os", "execbase" },
        .chelp  = { "Display information about the ExecBase struct" },
        
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpExecBase(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "interrupts" },
        .chelp  = { "List all interrupt handlers" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            osDebugger.dumpIntVectors(ss);
            retroShell << ss;
        }
    });
    
    root.add({
        
        .tokens = { "os", "libraries" },
        .chelp  = { "List all libraries" },
        .args   = {
            { .name = { "nr", "Library number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "List all devices" },
        .args   = {
            { .name = { "nr", "Device number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "List all resources" },
        .args   = {
            { .name = { "nr", "Resource number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "List all tasks" },
        .args   = {
            { .name = { "nr", "Task number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "List all processes" },
        .args   = {
            { .name = { "nr", "Process number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "Pause emulation on task launch" },
        .args   = {
            { .name = { "task", "Task name" } }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            const auto &task = args.at("task");
            diagBoard.catchTask(args.at(task));
            retroShell << "Waiting for task '" << args.at(task) << "' to start...\n";
        }
    });
    
    root.add({
        
        .tokens = { "os", "set"},
        .ghelp  = { "Configure the component" }
    });
    
    root.add({
        
        .tokens = { "os", "set", "diagboard" },
        .chelp  = { "Attach or detach the debug expansion board" },
        .args   = {
            { .name = { "switch", "Is the board plugged in?" }, .key = "{ true | false }" }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            diagBoard.setOption(Opt::DIAG_BOARD, parseBool(args.at("switch")));
        }
    });
    
    
    //
    // Miscellaneous
    //
    
    RSCommand::currentGroup = "Miscellaneous";
    
    root.add({
        
        .tokens = { "debug" },
        .ghelp  = { "Debug variables" },
        .chelp  = { "Display all debug variables" },

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, emulator, Category::Debug);
        }
    });
    
    if (debugBuild) {
        
        for (auto i : DebugFlagEnum::elements()) {
            
            root.add({
                
                .tokens = { "debug", DebugFlagEnum::key(i) },
                .chelp  = { DebugFlagEnum::help(i) },
                .args   = {
                    { .name = { "level", "Debug level" } }
                },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                    
                    Emulator::setDebugVariable(DebugFlag(values[0]), int(parseNum(args, "level")));

                }, .payload = { isize(i) }
            });
        }
        
        root.add({
            
            .tokens = { "debug", "verbosity" },
            .chelp  = { "Set the verbosity level for generated debug output" },
            .args   = {
                { .name = { "level", "Verbosity level" } }
            },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                CoreObject::verbosity = isize(parseNum(args, "level"));
            }
        });
    }
    
    root.add({
        
        .tokens = {"%"},
        .chelp  = { "Convert a value into different formats" },
        .args   = {
            { .name = { "value", "Payload" } }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
