// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Amiga.h"
#include "BootBlockImage.h"
#include "FSTypes.h"
#include "IOUtils.h"
#include "Parser.h"
#include <fstream>
#include <sstream>

namespace vamiga {

//
// Instruction Stream
//

template <> void
RetroShell::exec <Token::pause> (Arguments& argv, long param)
{
    amiga.pause();
}

template <> void
RetroShell::exec <Token::run> (Arguments& argv, long param)
{
    amiga.run();
}

template <> void
RetroShell::exec <Token::step> (Arguments& argv, long param)
{
    amiga.stepInto();
}

template <> void
RetroShell::exec <Token::next> (Arguments& argv, long param)
{
    amiga.stepOver();
}

template <> void
RetroShell::exec <Token::jump> (Arguments& argv, long param)
{
    auto value = util::parseNum(argv.front());
    amiga.cpu.jump((u32)value);
}

template <> void
RetroShell::exec <Token::disassemble> (Arguments& argv, long param)
{
    std::stringstream ss;

    auto addr = argv.empty() ? cpu.getPC0() : u32(util::parseNum(argv.front()));
    cpu.disassembleRange(ss, addr, 16);

    *this << '\n' << ss << '\n';
}


//
// Breakpoints
//

template <> void
RetroShell::exec <Token::bp> (Arguments& argv, long param)
{
    dump(amiga.cpu, Category::Breakpoints);
}

template <> void
RetroShell::exec <Token::bp, Token::at> (Arguments& argv, long param)
{
    amiga.cpu.setBreakpoint(u32(util::parseNum(argv.front())));
}

template <> void
RetroShell::exec <Token::bp, Token::del> (Arguments& argv, long param)
{
    amiga.cpu.deleteBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::bp, Token::enable> (Arguments& argv, long param)
{
    amiga.cpu.enableBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::bp, Token::disable> (Arguments& argv, long param)
{
    amiga.cpu.disableBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::bp, Token::ignore> (Arguments& argv, long param)
{
    amiga.cpu.ignoreBreakpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
}


//
// Watchpoints
//

template <> void
RetroShell::exec <Token::wp> (Arguments& argv, long param)
{
    dump(amiga.cpu, Category::Watchpoints);
}

template <> void
RetroShell::exec <Token::wp, Token::at> (Arguments& argv, long param)
{
    amiga.cpu.setWatchpoint(u32(util::parseNum(argv.front())));
}

template <> void
RetroShell::exec <Token::wp, Token::del> (Arguments& argv, long param)
{
    amiga.cpu.deleteWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::wp, Token::enable> (Arguments& argv, long param)
{
    amiga.cpu.enableWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::wp, Token::disable> (Arguments& argv, long param)
{
    amiga.cpu.disableWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::wp, Token::ignore> (Arguments& argv, long param)
{
    amiga.cpu.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
}


//
// Catchpoints
//

template <> void
RetroShell::exec <Token::cp> (Arguments& argv, long param)
{
    dump(amiga.cpu, Category::Catchpoints);
}

template <> void
RetroShell::exec <Token::cp, Token::vector> (Arguments& argv, long param)
{
    auto nr = util::parseNum(argv.front());
    if (nr < 0 || nr > 255) throw VAError(ERROR_OPT_INVARG, "0...255");
    amiga.cpu.setCatchpoint(u8(nr));
}

template <> void
RetroShell::exec <Token::cp, Token::interrupt> (Arguments& argv, long param)
{
    auto nr = util::parseNum(argv.front());
    if (nr < 1 || nr > 7) throw VAError(ERROR_OPT_INVARG, "1...7");
    amiga.cpu.setCatchpoint(u8(nr + 24));
}

template <> void
RetroShell::exec <Token::cp, Token::trap> (Arguments& argv, long param)
{
    auto nr = util::parseNum(argv.front());
    if (nr < 0 || nr > 15) throw VAError(ERROR_OPT_INVARG, "0...15");
    amiga.cpu.setCatchpoint(u8(nr + 32));
}

template <> void
RetroShell::exec <Token::cp, Token::del> (Arguments& argv, long param)
{
    amiga.cpu.deleteCatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cp, Token::enable> (Arguments& argv, long param)
{
    amiga.cpu.enableCatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cp, Token::disable> (Arguments& argv, long param)
{
    amiga.cpu.disableCatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cp, Token::ignore> (Arguments& argv, long param)
{
    amiga.cpu.ignoreCatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
}


//
// Copper breakpoints
//

template <> void
RetroShell::exec <Token::cbp> (Arguments& argv, long param)
{
    dump(copper.debugger, Category::Breakpoints);
}

template <> void
RetroShell::exec <Token::cbp, Token::at> (Arguments& argv, long param)
{
    copper.debugger.setBreakpoint(u32(util::parseNum(argv.front())));
}

template <> void
RetroShell::exec <Token::cbp, Token::del> (Arguments& argv, long param)
{
    copper.debugger.deleteBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cbp, Token::enable> (Arguments& argv, long param)
{
    copper.debugger.enableBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cbp, Token::disable> (Arguments& argv, long param)
{
    copper.debugger.disableBreakpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cbp, Token::ignore> (Arguments& argv, long param)
{
    copper.debugger.ignoreBreakpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
}


//
// Copper watchpoints
//

template <> void
RetroShell::exec <Token::cwp> (Arguments& argv, long param)
{
    dump(copper.debugger, Category::Watchpoints);
}

template <> void
RetroShell::exec <Token::cwp, Token::at> (Arguments& argv, long param)
{
    copper.debugger.setWatchpoint(u32(util::parseNum(argv.front())));
}

template <> void
RetroShell::exec <Token::cwp, Token::del> (Arguments& argv, long param)
{
    copper.debugger.deleteWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cwp, Token::enable> (Arguments& argv, long param)
{
    copper.debugger.enableWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cwp, Token::disable> (Arguments& argv, long param)
{
    copper.debugger.disableWatchpoint(util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::cwp, Token::ignore> (Arguments& argv, long param)
{
    copper.debugger.ignoreWatchpoint(util::parseNum(argv[0]), util::parseNum(argv[1]));
}


//
// Amiga
//

template <> void
RetroShell::exec <Token::amiga> (Arguments &argv, long param)
{
    dump(amiga, Category::Inspection);
}

template <> void
RetroShell::exec <Token::amiga, Token::host> (Arguments &argv, long param)
{
    dump(host, Category::Inspection);
}

template <> void
RetroShell::exec <Token::amiga, Token::debug> (Arguments &argv, long param)
{
    dump(amiga, Category::Debug);
}


//
// Memory
//

template <> void
RetroShell::exec <Token::memory> (Arguments &argv, long param)
{
    dump(mem, Category::BankMap);
}

template <> void
RetroShell::exec <Token::memory, Token::memdump> (Arguments& argv, long param)
{
    std::stringstream ss;

    mem.memDump<ACCESSOR_CPU>(ss, u32(util::parseNum(argv.front())));

    *this << '\n' << ss << '\n';
}

template <> void
RetroShell::exec <Token::memory, Token::bankmap> (Arguments& argv, long param)
{
    dump(amiga, Category::BankMap);
}

template <> void
RetroShell::exec <Token::memory, Token::checksums> (Arguments& argv, long param)
{
    dump(amiga.mem, Category::Checksums);
}


//
// CPU
//

template <> void
RetroShell::exec <Token::cpu> (Arguments &argv, long param)
{
    dumpSummary(cpu);
}

template <> void
RetroShell::exec <Token::cpu, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(cpu);
}

template <> void
RetroShell::exec <Token::cpu, Token::vectors> (Arguments& argv, long param)
{
    dump(cpu, Category::Vectors);
}


//
// CIA
//

template <> void
RetroShell::exec <Token::cia> (Arguments &argv, long param)
{
    param == 0 ? dumpSummary(ciaa) : dumpSummary(ciab);
}

template <> void
RetroShell::exec <Token::cia, Token::debug> (Arguments &argv, long param)
{
    param == 0 ? dumpDetails(ciaa) : dumpDetails(ciab);
}

template <> void
RetroShell::exec <Token::cia, Token::tod> (Arguments &argv, long param)
{
    param == 0 ? dump(ciaa, Category::Tod) : dump(ciab, Category::Tod);
}


//
// Agnus
//

template <> void
RetroShell::exec <Token::agnus> (Arguments &argv, long param)
{
    dumpSummary(agnus);
}

template <> void
RetroShell::exec <Token::agnus, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(agnus);
}

template <> void
RetroShell::exec <Token::agnus, Token::beam> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Beam);
}

template <> void
RetroShell::exec <Token::agnus, Token::dma> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Dma);
}

template <> void
RetroShell::exec <Token::agnus, Token::events> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Events);
}


//
// Blitter
//

template <> void
RetroShell::exec <Token::blitter> (Arguments &argv, long param)
{
    dumpSummary(amiga.agnus.blitter);
}

template <> void
RetroShell::exec <Token::blitter, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(amiga.agnus.blitter);
}


//
// Copper
//

template <> void
RetroShell::exec <Token::copper> (Arguments &argv, long param)
{
    dumpSummary(amiga.agnus.copper);
}

template <> void
RetroShell::exec <Token::copper, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(amiga.agnus.copper);
}

template <> void
RetroShell::exec <Token::copper, Token::list> (Arguments& argv, long param)
{
    auto value = util::parseNum(argv.front());

    switch (value) {

        case 1: dump(amiga.agnus.copper, Category::List1); break;
        case 2: dump(amiga.agnus.copper, Category::List2); break;

        default:
            throw VAError(ERROR_OPT_INVARG, "1 or 2");
    }
}


//
// Paula
//

template <> void
RetroShell::exec <Token::paula> (Arguments &argv, long param)
{
    dumpSummary(paula);
}

template <> void
RetroShell::exec <Token::paula, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(paula);
}

template <> void
RetroShell::exec <Token::paula, Token::audio> (Arguments& argv, long param)
{
    dumpSummary(amiga.paula.muxer);
}

template <> void
RetroShell::exec <Token::paula, Token::audio, Token::debug> (Arguments& argv, long param)
{
    dumpDetails(amiga.paula.muxer);
}

template <> void
RetroShell::exec <Token::paula, Token::dc> (Arguments& argv, long param)
{
    dumpSummary(amiga.paula.diskController);
}

template <> void
RetroShell::exec <Token::paula, Token::dc, Token::debug> (Arguments& argv, long param)
{
    dumpDetails(amiga.paula.diskController);
}

template <> void
RetroShell::exec <Token::paula, Token::uart> (Arguments& argv, long param)
{
    dumpSummary(amiga.paula.uart);
}


//
// Denise
//

template <> void
RetroShell::exec <Token::denise> (Arguments &argv, long param)
{
    dumpSummary(denise);
}

template <> void
RetroShell::exec <Token::denise, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(denise);
}


//
// RTC
//

template <> void
RetroShell::exec <Token::rtc> (Arguments &argv, long param)
{
    dumpSummary(rtc);
}

template <> void
RetroShell::exec <Token::rtc, Token::debug> (Arguments &argv, long param)
{
    dumpDetails(rtc);
}


//
// Control ports
//

template <> void
RetroShell::exec <Token::controlport> (Arguments& argv, long param)
{
    if (param == 1) dumpSummary(amiga.controlPort1);
    if (param == 2) dumpSummary(amiga.controlPort2);
}

template <> void
RetroShell::exec <Token::controlport, Token::debug> (Arguments& argv, long param)
{
    if (param == 1) dumpDetails(amiga.controlPort1);
    if (param == 2) dumpDetails(amiga.controlPort2);
}


//
// Serial port
//

template <> void
RetroShell::exec <Token::serial> (Arguments& argv, long param)
{
    dumpSummary(amiga.serialPort);
}


//
// Keyboard, Mice, Joysticks
//

template <> void
RetroShell::exec <Token::keyboard> (Arguments& argv, long param)
{
    dumpSummary(amiga.keyboard);
}

template <> void
RetroShell::exec <Token::mouse> (Arguments& argv, long param)
{
    if (param == 1) dumpSummary(amiga.controlPort1.mouse);
    if (param == 2) dumpSummary(amiga.controlPort2.mouse);
}

template <> void
RetroShell::exec <Token::mouse, Token::debug> (Arguments& argv, long param)
{
    if (param == 1) dumpDetails(amiga.controlPort1.mouse);
    if (param == 2) dumpDetails(amiga.controlPort2.mouse);
}

template <> void
RetroShell::exec <Token::joystick> (Arguments& argv, long param)
{
    if (param == 1) dumpSummary(amiga.controlPort1.joystick);
    if (param == 2) dumpSummary(amiga.controlPort2.joystick);
}


//
// Df0, Df1, Df2, Df3
//

template <> void
RetroShell::exec <Token::dfn> (Arguments& argv, long param)
{
    dumpSummary(*amiga.df[param]);
}

template <> void
RetroShell::exec <Token::dfn, Token::debug> (Arguments& argv, long param)
{
    dumpDetails(*amiga.df[param]);
}

template <> void
RetroShell::exec <Token::dfn, Token::disk> (Arguments& argv, long param)
{
    dump(*amiga.df[param], Category::Disk);
}


//
// Hd0, Hd1, Hd2, Hd3
//

template <> void
RetroShell::exec <Token::hdn, Token::drive> (Arguments& argv, long param)
{
    dump(*amiga.hd[param], Category::Drive);
}

template <> void
RetroShell::exec <Token::hdn, Token::volumes> (Arguments& argv, long param)
{
    dump(*amiga.hd[param], Category::Volumes);
}

template <> void
RetroShell::exec <Token::hdn, Token::partition> (Arguments& argv, long param)
{
    dump(*amiga.hd[param], Category::Partitions);
}

template <> void
RetroShell::exec <Token::hdn, Token::debug> (Arguments& argv, long param)
{
    dumpDetails(*amiga.hd[param]);
}


//
// OSDebugger
//

template <> void
RetroShell::exec <Token::os, Token::info> (Arguments& argv, long param)
{
    std::stringstream ss;
    osDebugger.dumpInfo(ss);

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::execbase> (Arguments& argv, long param)
{
    std::stringstream ss;
    osDebugger.dumpExecBase(ss);

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::interrupts> (Arguments& argv, long param)
{
    std::stringstream ss;
    osDebugger.dumpIntVectors(ss);

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::libraries> (Arguments& argv, long param)
{
    std::stringstream ss;
    isize num;

    if (argv.empty()) {
        osDebugger.dumpLibraries(ss);
    } else if (util::parseHex(argv.front(), &num)) {
        osDebugger.dumpLibrary(ss, (u32)num);
    } else {
        osDebugger.dumpLibrary(ss, argv.front());
    }

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::devices> (Arguments& argv, long param)
{
    std::stringstream ss;
    isize num;

    if (argv.empty()) {
        osDebugger.dumpDevices(ss);
    } else if (util::parseHex(argv.front(), &num)) {
        osDebugger.dumpDevice(ss, (u32)num);
    } else {
        osDebugger.dumpDevice(ss, argv.front());
    }

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::resources> (Arguments& argv, long param)
{
    std::stringstream ss;
    isize num;

    if (argv.empty()) {
        osDebugger.dumpResources(ss);
    } else if (util::parseHex(argv.front(), &num)) {
        osDebugger.dumpResource(ss, (u32)num);
    } else {
        osDebugger.dumpResource(ss, argv.front());
    }

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::tasks> (Arguments& argv, long param)
{
    std::stringstream ss;
    isize num;

    if (argv.empty()) {
        osDebugger.dumpTasks(ss);
    } else if (util::parseHex(argv.front(), &num)) {
        osDebugger.dumpTask(ss, (u32)num);
    } else {
        osDebugger.dumpTask(ss, argv.front());
    }

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::processes> (Arguments& argv, long param)
{
    std::stringstream ss;
    isize num;

    if (argv.empty()) {
        osDebugger.dumpProcesses(ss);
    } else if (util::parseHex(argv.front(), &num)) {
        osDebugger.dumpProcess(ss, (u32)num);
    } else {
        osDebugger.dumpProcess(ss, argv.front());
    }

    *this << ss;
}

template <> void
RetroShell::exec <Token::os, Token::cp> (Arguments& argv, long param)
{
    diagBoard.catchTask(argv.back());
    *this << "Waiting for task '" << argv.back() << "' to start...\n";
}

template <> void
RetroShell::exec <Token::os, Token::set, Token::diagboard> (Arguments& argv, long param)
{
    diagBoard.setConfigItem(OPT_DIAG_BOARD, util::parseBool(argv.front()));
}

}
