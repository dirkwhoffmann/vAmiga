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
// Top-level commands
//

template <> void
RetroShell::exec <Token::debug> (Arguments &argv, long param)
{
    clear();
    interpreter.switchInterpreter();
    welcome();
}

template <> void
RetroShell::exec <Token::clear> (Arguments &argv, long param)
{
    clear();
}

template <> void
RetroShell::exec <Token::close> (Arguments &argv, long param)
{
    msgQueue.put(MSG_CLOSE_CONSOLE);
}

template <> void
RetroShell::exec <Token::help> (Arguments &argv, long param)
{
    retroShell.help(argv.empty() ? "" : argv.front());
}

template <> void
RetroShell::exec <Token::easteregg> (Arguments& argv, long param)
{
    retroShell << "GREETINGS PROFESSOR HOFFMANN." << '\n' << '\n';
    retroShell << "THE ONLY WINNING MOVE IS NOT TO PLAY." << '\n' << '\n';
    retroShell << "HOW ABOUT A NICE GAME OF CHESS?" << '\n';
}

template <> void
RetroShell::exec <Token::source> (Arguments &argv, long param)
{
    auto stream = std::ifstream(argv.front());
    if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND, argv.front());
    
    execScript(stream);
}

template <> void
RetroShell::exec <Token::wait> (Arguments &argv, long param)
{
    auto seconds = util::parseNum(argv.front());
    
    Cycle limit = agnus.clock + SEC(seconds);
    amiga.retroShell.wakeUp = limit;
    
    throw ScriptInterruption("");
}


//
// Rgression testing
//

template <> void
RetroShell::exec <Token::regression, Token::setup> (Arguments &argv, long param)
{
    auto scheme = util::parseEnum <ConfigSchemeEnum> (argv[0]);
    auto rom = argv[1];
    auto ext = argv.size() > 2 ? argv[2] : "";
    
    amiga.regressionTester.prepare(scheme, rom, ext);
    
}

template <> void
RetroShell::exec <Token::regression, Token::run> (Arguments &argv, long param)
{
    amiga.regressionTester.run(argv.front());
}

template <> void
RetroShell::exec <Token::screenshot, Token::set, Token::filename> (Arguments &argv, long param)
{
    amiga.regressionTester.dumpTexturePath = argv.front();
}

template <> void
RetroShell::exec <Token::screenshot, Token::set, Token::cutout> (Arguments &argv, long param)
{
    std::vector<string> vec(argv.begin(), argv.end());
    
    isize x1 = util::parseNum(vec[0]);
    isize y1 = util::parseNum(vec[1]);
    isize x2 = util::parseNum(vec[2]);
    isize y2 = util::parseNum(vec[3]);

    amiga.regressionTester.x1 = x1;
    amiga.regressionTester.y1 = y1;
    amiga.regressionTester.x2 = x2;
    amiga.regressionTester.y2 = y2;
}

template <> void
RetroShell::exec <Token::screenshot, Token::save> (Arguments &argv, long param)
{
    amiga.regressionTester.dumpTexture(amiga, argv.front());
}


//
// Amiga
//

template <> void
RetroShell::exec <Token::amiga, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga);
}

template <> void
RetroShell::exec <Token::amiga, Token::set, Token::type> (Arguments& argv, long param)
{
    amiga.configure(OPT_VIDEO_FORMAT, util::parseEnum <VideoFormatEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::amiga, Token::vsync> (Arguments &argv, long param)
{
    amiga.configure(OPT_VSYNC, util::parseOnOff(argv.front()));
}

template <> void
RetroShell::exec <Token::amiga, Token::init> (Arguments &argv, long param)
{
    auto scheme = util::parseEnum <ConfigSchemeEnum> (argv.front());

    amiga.revertToFactorySettings();
    amiga.configure(scheme);
}

template <> void
RetroShell::exec <Token::amiga, Token::power> (Arguments &argv, long param)
{
    if (util::parseOnOff(argv.front())) {

        amiga.powerOn();
        amiga.run();

    } else {

        amiga.powerOff();
    }
}

template <> void
RetroShell::exec <Token::amiga, Token::reset> (Arguments &argv, long param)
{
    amiga.reset(true);
}

template <> void
RetroShell::exec <Token::amiga, Token::defaults> (Arguments &argv, long param)
{
    dump(amiga, Category::Defaults);
}


//
// Memory
//

template <> void
RetroShell::exec <Token::memory, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.mem);
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::chip> (Arguments& argv, long param)
{
    amiga.configure(OPT_CHIP_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::slow> (Arguments& argv, long param)
{
    amiga.configure(OPT_SLOW_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::fast> (Arguments& argv, long param)
{
    amiga.configure(OPT_FAST_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::extstart> (Arguments& argv, long param)
{
    amiga.configure(OPT_EXT_START, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::saveroms> (Arguments& argv, long param)
{
    amiga.configure(OPT_SAVE_ROMS, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::slowramdelay> (Arguments& argv, long param)
{
    amiga.configure(OPT_SLOW_RAM_DELAY, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::bankmap> (Arguments& argv, long param)
{
    amiga.configure(OPT_BANKMAP, util::parseEnum <BankMapEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::unmappingtype> (Arguments& argv, long param)
{
    amiga.configure(OPT_UNMAPPING_TYPE, util::parseEnum <UnmappedMemoryEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::raminitpattern> (Arguments& argv, long param)
{
    amiga.configure(OPT_RAM_INIT_PATTERN, util::parseEnum <RamInitPatternEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::load, Token::rom> (Arguments& argv, long param)
{
    auto path = argv.front();
    amiga.mem.loadRom(path.c_str());
}

template <> void
RetroShell::exec <Token::memory, Token::load, Token::extrom> (Arguments& argv, long param)
{
    auto path = argv.front();
    amiga.mem.loadExt(path.c_str());
}


//
// CPU
//

template <> void
RetroShell::exec <Token::cpu, Token::config> (Arguments &argv, long param)
{
    dumpConfig(amiga.cpu);
}

template <> void
RetroShell::exec <Token::cpu, Token::set, Token::revision> (Arguments &argv, long param)
{
    auto value = util::parseEnum <CPURevisionEnum> (argv.front());
    amiga.configure(OPT_CPU_REVISION, value);
}

template <> void
RetroShell::exec <Token::cpu, Token::set, Token::dasm, Token::revision> (Arguments &argv, long param)
{
    auto value = util::parseEnum <DasmRevisionEnum> (argv.front());
    amiga.configure(OPT_CPU_DASM_REVISION, value);
}

template <> void
RetroShell::exec <Token::cpu, Token::set, Token::dasm, Token::syntax> (Arguments &argv, long param)
{
    auto value = util::parseEnum <DasmSyntaxEnum> (argv.front());
    amiga.configure(OPT_CPU_DASM_SYNTAX, value);
}

template <> void
RetroShell::exec <Token::cpu, Token::set, Token::overclocking> (Arguments &argv, long param)
{
    auto value = util::parseNum(argv.front());
    amiga.configure(OPT_CPU_OVERCLOCKING, value);
}

template <> void
RetroShell::exec <Token::cpu, Token::set, Token::regreset> (Arguments &argv, long param)
{
    auto value = util::parseNum(argv.front());
    amiga.configure(OPT_CPU_RESET_VAL, value);
}


//
// CIA
//

template <> void
RetroShell::exec <Token::cia, Token::config> (Arguments &argv, long param)
{
    param == 0 ? dumpConfig(amiga.ciaA) : dumpConfig(amiga.ciaB);
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::revision> (Arguments &argv, long param)
{
    auto value = util::parseEnum <CIARevisionEnum> (argv.front());
    amiga.configure(OPT_CIA_REVISION, param, value);
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::todbug> (Arguments &argv, long param)
{
    auto value = util::parseBool(argv.front());
    amiga.configure(OPT_TODBUG, param, value);
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::esync> (Arguments &argv, long param)
{
    auto value = util::parseBool(argv.front());
    amiga.configure(OPT_ECLOCK_SYNCING, param, value);
}


//
// Agnus
//

template <> void
RetroShell::exec <Token::agnus, Token::config> (Arguments &argv, long param)
{
    dumpConfig(amiga.agnus);
}

template <> void
RetroShell::exec <Token::agnus, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_AGNUS_REVISION, util::parseEnum <AgnusRevisionEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror> (Arguments &argv, long param)
{
    amiga.configure(OPT_SLOW_RAM_MIRROR, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::agnus, Token::set, Token::ptrdrops> (Arguments &argv, long param)
{
    auto value = util::parseBool(argv.front());
    amiga.configure(OPT_PTR_DROPS, value);
}

/*
template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::state> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Debug);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::beam> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Beam);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::registers> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Registers);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::dma> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Dma);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::events> (Arguments &argv, long param)
{
    dump(amiga.agnus, Category::Events);
}
*/


//
// Blitter
//

template <> void
RetroShell::exec <Token::blitter, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.agnus.blitter);
}

template <> void
RetroShell::exec <Token::blitter, Token::set, Token::accuracy> (Arguments &argv, long param)
{
    amiga.configure(OPT_BLITTER_ACCURACY, util::parseNum(argv.front()));
}

/*
template <> void
RetroShell::exec <Token::blitter, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.agnus.blitter, Category::Debug);
}

template <> void
RetroShell::exec <Token::blitter, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.agnus.blitter, Category::Registers);
}
*/


//
// Copper
//

/*
template <> void
RetroShell::exec <Token::copper, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.agnus.copper, Category::Debug);
}

template <> void
RetroShell::exec <Token::copper, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.agnus.copper, Category::Registers);
}

template <> void
RetroShell::exec <Token::copper, Token::list> (Arguments& argv, long param)
{
    auto value = util::parseNum(argv.front());
    
    switch (value) {
        case 1: dump(amiga.agnus.copper, Category::List1); break;
        case 2: dump(amiga.agnus.copper, Category::List2); break;
        default: throw VAError(ERROR_OPT_INVARG, "1 or 2");
    }
}
*/


//
// Denise
//

template <> void
RetroShell::exec <Token::denise, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.denise);
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_DENISE_REVISION, util::parseEnum <DeniseRevisionEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::tracking> (Arguments &argv, long param)
{
    amiga.configure(OPT_VIEWPORT_TRACKING, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxsprspr> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_SPR_SPR, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxsprplf> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_SPR_PLF, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxplfplf> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_PLF_PLF, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::hide, Token::bitplanes> (Arguments &argv, long param)
{
    amiga.configure(OPT_HIDDEN_BITPLANES, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::hide, Token::sprites> (Arguments &argv, long param)
{
    amiga.configure(OPT_HIDDEN_SPRITES, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::hide, Token::layers> (Arguments &argv, long param)
{
    amiga.configure(OPT_HIDDEN_LAYERS, util::parseNum(argv.front()));
}


//
// DMA Debugger
//

template <> void
RetroShell::exec <Token::dmadebugger, Token::open> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_ENABLE, true);
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::close> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_ENABLE, false);
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::copper> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_COPPER, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::blitter> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BLITTER, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::disk> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_DISK, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::audio> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_AUDIO, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::sprites> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_SPRITE, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::bitplanes> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BITPLANE, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::cpu> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_CPU, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dmadebugger, Token::refresh> (Arguments& argv, long param)
{
    amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_REFRESH, util::parseBool(argv.front()));
}


//
// Monitor
//

template <> void
RetroShell::exec <Token::monitor, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.denise.pixelEngine);
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::palette> (Arguments& argv, long param)
{
    amiga.configure(OPT_PALETTE, util::parseEnum <PaletteEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::brightness> (Arguments& argv, long param)
{
    amiga.configure(OPT_BRIGHTNESS, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::contrast> (Arguments& argv, long param)
{
    amiga.configure(OPT_CONTRAST, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::saturation> (Arguments& argv, long param)
{
    amiga.configure(OPT_SATURATION, util::parseNum(argv.front()));
}


//
// Audio
//

template <> void
RetroShell::exec <Token::audio, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.paula.muxer);
}

template <> void
RetroShell::exec <Token::audio, Token::filter, Token::config> (Arguments& argv, long param)
{
    *this << '\n';

    *this << "Left channel:" << '\n';
    dumpConfig(paula.muxer.filterL);

    *this << "Right channel:" << '\n';
    dumpConfig(paula.muxer.filterR);
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::sampling> (Arguments& argv, long param)
{
    amiga.configure(OPT_SAMPLING_METHOD, util::parseEnum <SamplingMethodEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::filter, Token::set, Token::type> (Arguments& argv, long param)
{
    amiga.configure(OPT_FILTER_TYPE, util::parseEnum <FilterTypeEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::filter, Token::set, Token::activation> (Arguments& argv, long param)
{
    amiga.configure(OPT_FILTER_ACTIVATION, util::parseEnum <FilterActivationEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::volume> (Arguments& argv, long param)
{
    switch (param) {
            
        case 0: amiga.configure(OPT_AUDVOL, 0, util::parseNum(argv.front())); break;
        case 1: amiga.configure(OPT_AUDVOL, 1, util::parseNum(argv.front())); break;
        case 2: amiga.configure(OPT_AUDVOL, 2, util::parseNum(argv.front())); break;
        case 3: amiga.configure(OPT_AUDVOL, 3, util::parseNum(argv.front())); break;
        case 4: amiga.configure(OPT_AUDVOLL, util::parseNum(argv.front())); break;
        case 5: amiga.configure(OPT_AUDVOLR, util::parseNum(argv.front())); break;
            
        default:
            fatalError;
    }
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::pan> (Arguments& argv, long param)
{
    amiga.configure(OPT_AUDPAN, param, util::parseNum(argv.front()));
}


//
// Paula
//

/*
template <> void
RetroShell::exec <Token::paula, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dumpDetails(amiga.paula);
}

template <> void
RetroShell::exec <Token::paula, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.paula, Category::Registers);
}
*/


//
// RTC
//

template <> void
RetroShell::exec <Token::rtc, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.rtc);
}

template <> void
RetroShell::exec <Token::rtc, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_RTC_MODEL, util::parseEnum <RTCRevisionEnum> (argv.front()));
}


//
// Keyboard
//

template <> void
RetroShell::exec <Token::keyboard, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.keyboard);
}

template <> void
RetroShell::exec <Token::keyboard, Token::set, Token::accuracy> (Arguments &argv, long param)
{
    amiga.configure(OPT_ACCURATE_KEYBOARD, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::keyboard, Token::press> (Arguments& argv, long param)
{
    auto keycode = util::parseNum(argv.front());
    keyboard.autoType(KeyCode(keycode));
}


//
// Mouse
//

template <> void
RetroShell::exec <Token::mouse, Token::config> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    dumpConfig(port.mouse);
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::pullup> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_PULLUP_RESISTORS, port, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::shakedetector> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_SHAKE_DETECTION, port, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::velocity> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_MOUSE_VELOCITY, port, util::parseNum(argv.front()));
}

/*
template <> void
RetroShell::exec <Token::mouse, Token::inspect> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    dumpDetails(port.mouse);
}
*/

template <> void
RetroShell::exec <Token::mouse, Token::press, Token::left> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.mouse.pressAndReleaseLeft();
}

template <> void
RetroShell::exec <Token::mouse, Token::press, Token::right> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.mouse.pressAndReleaseRight();
}


//
// Joystick
//

template <> void
RetroShell::exec <Token::joystick, Token::config> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    dumpConfig(port.joystick);
}

template <> void
RetroShell::exec <Token::joystick, Token::set, Token::autofire> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_AUTOFIRE, port, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::joystick, Token::set, Token::bullets> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_AUTOFIRE_BULLETS, port, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::joystick, Token::set, Token::delay> (Arguments &argv, long param)
{
    auto port = (param == 0) ? ControlPort::PORT1 : ControlPort::PORT2;
    amiga.configure(OPT_AUTOFIRE_DELAY, port, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::joystick, Token::press> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    auto nr = util::parseNum(argv.front());

    switch (nr) {

        case 1: port.joystick.trigger(PRESS_FIRE); break;
        case 2: port.joystick.trigger(PRESS_FIRE2); break;
        case 3: port.joystick.trigger(PRESS_FIRE3); break;

        default:
            throw VAError(ERROR_OPT_INVARG, "1...3");
    }
}

template <> void
RetroShell::exec <Token::joystick, Token::unpress> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    auto nr = util::parseNum(argv.front());

    switch (nr) {

        case 1: port.joystick.trigger(RELEASE_FIRE); break;
        case 2: port.joystick.trigger(RELEASE_FIRE2); break;
        case 3: port.joystick.trigger(RELEASE_FIRE3); break;

        default:
            throw VAError(ERROR_OPT_INVARG, "1...3");
    }
}

template <> void
RetroShell::exec <Token::joystick, Token::pull, Token::left> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(PULL_LEFT);
}

template <> void
RetroShell::exec <Token::joystick, Token::pull, Token::right> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(PULL_RIGHT);
}

template <> void
RetroShell::exec <Token::joystick, Token::pull, Token::up> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(PULL_UP);
}

template <> void
RetroShell::exec <Token::joystick, Token::pull, Token::down> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(PULL_DOWN);
}

template <> void
RetroShell::exec <Token::joystick, Token::release, Token::xaxis> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(RELEASE_X);
}

template <> void
RetroShell::exec <Token::joystick, Token::release, Token::yaxis> (Arguments& argv, long param)
{
    auto &port = (param == 0) ? amiga.controlPort1 : amiga.controlPort2;
    port.joystick.trigger(RELEASE_Y);
}


//
// Serial port
//

template <> void
RetroShell::exec <Token::serial, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.serialPort);
}

template <> void
RetroShell::exec <Token::serial, Token::set, Token::device> (Arguments &argv, long param)
{
    amiga.configure(OPT_SERIAL_DEVICE, util::parseEnum <SerialPortDeviceEnum> (argv.front()));
}


//
// Disk controller
//

template <> void
RetroShell::exec <Token::dc, Token::config> (Arguments& argv, long param)
{
    dumpConfig(amiga.paula.diskController);
}

template <> void
RetroShell::exec <Token::dc, Token::speed> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_SPEED, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::dc, Token::dsksync, Token::autosync> (Arguments& argv, long param)
{
    amiga.configure(OPT_AUTO_DSKSYNC, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dc, Token::dsksync, Token::lock> (Arguments& argv, long param)
{
    amiga.configure(OPT_LOCK_DSKSYNC, util::parseBool(argv.front()));
}


//
// Df0, Df1, Df2, Df3
//

template <> void
RetroShell::exec <Token::dfn, Token::config> (Arguments& argv, long param)
{
    dumpConfig(*amiga.df[param]);
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::insert> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());

    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_INSERT_VOLUME, param, num);
    } else {
        amiga.configure(OPT_INSERT_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::eject> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_EJECT_VOLUME, param, num);
    } else {
        amiga.configure(OPT_EJECT_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::step> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_STEP_VOLUME, param, num);
    } else {
        amiga.configure(OPT_STEP_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::poll> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_POLL_VOLUME, param, num);
    } else {
        amiga.configure(OPT_POLL_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::eject> (Arguments& argv, long param)
{
    amiga.df[param]->ejectDisk();
}

template <> void
RetroShell::exec <Token::dfn, Token::connect> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_CONNECT, param, true);
}

template <> void
RetroShell::exec <Token::dfn, Token::disconnect> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_CONNECT, param, false);
}

template <> void
RetroShell::exec <Token::dfn, Token::insert> (Arguments& argv, long param)
{
    assert(param >= 0 && param <= 3);

    auto path = argv.front();
    df[param]->swapDisk(path);
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::model> (Arguments& argv, long param)
{
    long num = util::parseEnum <FloppyDriveTypeEnum> (argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_TYPE, param, num);
    } else {
        amiga.configure(OPT_DRIVE_TYPE, num);
    }
}

template <> void
RetroShell::exec <Token::dc, Token::set, Token::rpm> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());

    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_RPM, param, num);
    } else {
        amiga.configure(OPT_DRIVE_RPM, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::pan> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_PAN, param, num);
    } else {
        amiga.configure(OPT_DRIVE_PAN, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::mechanics> (Arguments& argv, long param)
{
    auto scheme = util::parseEnum<DriveMechanicsEnum>(argv[0]);

    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_MECHANICS, param, scheme);
    } else {
        amiga.configure(OPT_DRIVE_MECHANICS, scheme);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::searchpath> (Arguments& argv, long param)
{
    string path = argv.front();

    if (param == 0 || param > 3) df0.setSearchPath(path);
    if (param == 1 || param > 3) df1.setSearchPath(path);
    if (param == 2 || param > 3) df2.setSearchPath(path);
    if (param == 3 || param > 3) df3.setSearchPath(path);
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::swapdelay> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DISK_SWAP_DELAY, param, num);
    } else {
        amiga.configure(OPT_DISK_SWAP_DELAY, num);
    }
}


//
// Hd0, Hd1, Hd2, Hd3
//

template <> void
RetroShell::exec <Token::hdn, Token::config> (Arguments& argv, long param)
{
    dumpConfig(*amiga.hd[param]);
}

template <> void
RetroShell::exec <Token::hdn, Token::set, Token::pan> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_HDR_PAN, param, num);
    } else {
        amiga.configure(OPT_HDR_PAN, num);
    }
}

template <> void
RetroShell::exec <Token::hdn, Token::audiate, Token::step> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_HDR_STEP_VOLUME, param, num);
    } else {
        amiga.configure(OPT_HDR_STEP_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::hdn, Token::geometry> (Arguments& argv, long param)
{
    auto c = util::parseNum(argv[0]);
    auto h = util::parseNum(argv[1]);
    auto s = util::parseNum(argv[2]);

    amiga.hd[param]->changeGeometry(c, h, s);
}

//
// Zorro boards
//

template <> void
RetroShell::exec <Token::zorro, Token::list> (Arguments& argv, long param)
{
    dumpDetails(zorro);
}

template <> void
RetroShell::exec <Token::zorro, Token::inspect> (Arguments& argv, long param)
{
    auto value = util::parseNum(argv.front());
    
    if (auto board = zorro.getBoard(value); board != nullptr) {

        dump(*board, Category::Properties);
        dumpDetails(*board);
        dump(*board, Category::Stats);
    }
}


//
// Remote servers
//

template <> void
RetroShell::exec <Token::server> (Arguments& argv, long param)
{
    dump(remoteManager, Category::Status);
}

template <> void
RetroShell::exec <Token::server, Token::serial, Token::set, Token::port> (Arguments& argv, long param)
{
    remoteManager.serServer.setConfigItem(OPT_SRV_PORT, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::serial, Token::set, Token::verbose> (Arguments& argv, long param)
{
    remoteManager.serServer.setConfigItem(OPT_SRV_PORT, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::serial, Token::config> (Arguments& argv, long param)
{
    dumpConfig(remoteManager.serServer);
}

template <> void
RetroShell::exec <Token::server, Token::serial, Token::inspect> (Arguments& argv, long param)
{
    dumpDetails(remoteManager.serServer);
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::start> (Arguments& argv, long param)
{
    remoteManager.rshServer.start();
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::stop> (Arguments& argv, long param)
{
    remoteManager.rshServer.stop();
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::disconnect> (Arguments& argv, long param)
{
    remoteManager.rshServer.disconnect();
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::set, Token::port> (Arguments& argv, long param)
{
    remoteManager.rshServer.setConfigItem(OPT_SRV_PORT, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::set, Token::verbose> (Arguments& argv, long param)
{
    remoteManager.rshServer.setConfigItem(OPT_SRV_PORT, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::config> (Arguments& argv, long param)
{
    dumpConfig(remoteManager.rshServer);
}

template <> void
RetroShell::exec <Token::server, Token::rshell, Token::inspect> (Arguments& argv, long param)
{
    dumpDetails(remoteManager.rshServer);
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::attach> (Arguments& argv, long param)
{
    remoteManager.gdbServer.attach(argv.front());
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::detach> (Arguments& argv, long param)
{
    remoteManager.gdbServer.detach();
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::set, Token::port> (Arguments& argv, long param)
{
    remoteManager.gdbServer.setConfigItem(OPT_SRV_PORT, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::set, Token::verbose> (Arguments& argv, long param)
{
    remoteManager.gdbServer.setConfigItem(OPT_SRV_VERBOSE, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::config> (Arguments& argv, long param)
{
    dumpConfig(remoteManager.gdbServer);
}

template <> void
RetroShell::exec <Token::server, Token::gdb, Token::inspect> (Arguments& argv, long param)
{
    dumpDetails(remoteManager.gdbServer);
}

}
