// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "VAmiga.h"
#include "Emulator.h"

namespace vamiga {

void
API::suspend()
{
    emu->suspend();
}

void
API::resume()
{
    emu->resume();
}

bool
API::isUserThread() const
{
    return !emu->isEmulatorThread();
}

VAmiga::VAmiga() {

    emu = new Emulator();

    amiga.emu = emu;
    amiga.amiga = &emu->main;

    agnus.emu = emu;
    agnus.agnus = &emu->main.agnus;

    blitter.emu = emu;
    blitter.blitter = &emu->main.agnus.blitter;

    breakpoints.emu = emu;
    breakpoints.guards = &emu->main.cpu.debugger.breakpoints;

    ciaA.emu = emu;
    ciaA.cia = &emu->main.ciaA;

    ciaB.emu = emu;
    ciaB.cia = &emu->main.ciaB;

    controlPort1.emu = emu;
    controlPort1.controlPort = &emu->main.controlPort1;
    controlPort1.joystick.emu = emu;
    controlPort1.joystick.joystick = &emu->main.controlPort1.joystick;
    controlPort1.mouse.emu = emu;
    controlPort1.mouse.mouse = &emu->main.controlPort1.mouse;

    controlPort2.emu = emu;
    controlPort2.controlPort = &emu->main.controlPort2;
    controlPort2.joystick.emu = emu;
    controlPort2.joystick.joystick = &emu->main.controlPort2.joystick;
    controlPort2.mouse.emu = emu;
    controlPort2.mouse.mouse = &emu->main.controlPort2.mouse;

    copper.emu = emu;
    copper.copper = &emu->main.agnus.copper;

    copperBreakpoints.emu = emu;
    copperBreakpoints.guards = &emu->main.agnus.copper.debugger.breakpoints;

    cpu.emu = emu;
    cpu.cpu = &emu->main.cpu;

    debugger.emu = emu;
    debugger.debugger = &emu->main.debugger;

    denise.emu = emu;
    denise.denise = &emu->main.denise;

    diskController.emu = emu;
    diskController.diskController = &emu->main.paula.diskController;

    dmaDebugger.emu = emu;
    dmaDebugger.dmaDebugger = &emu->main.agnus.dmaDebugger;

    df0.emu = emu;
    df0.drive = &emu->main.df0;

    df1.emu = emu;
    df1.drive = &emu->main.df1;

    df2.emu = emu;
    df2.drive = &emu->main.df2;

    df3.emu = emu;
    df3.drive = &emu->main.df3;

    hd0.emu = emu;
    hd0.drive = &emu->main.hd0;

    hd1.emu = emu;
    hd1.drive = &emu->main.hd1;

    hd2.emu = emu;
    hd2.drive = &emu->main.hd2;

    hd3.emu = emu;
    hd3.drive = &emu->main.hd3;

    host.emu = emu;
    host.host = &emu->host;

    keyboard.emu = emu;
    keyboard.keyboard = &emu->main.keyboard;

    mem.emu = emu;
    mem.mem = &emu->main.mem;

    paula.emu = emu;
    paula.paula = &emu->main.paula;

    retroShell.emu = emu;
    retroShell.retroShell = &emu->main.retroShell;

    rtc.emu = emu;
    rtc.rtc = &emu->main.rtc;

    recorder.emu = emu;
    recorder.recorder = &emu->main.denise.screenRecorder;

    remoteManager.emu = emu;
    remoteManager.remoteManager = &emu->main.remoteManager;

    serialPort.emu = emu;
    serialPort.serialPort = &emu->main.serialPort;

    videoPort.emu = emu;
    videoPort.videoPort = &emu->main.videoPort;
    
    watchpoints.emu = emu;
    watchpoints.guards = &emu->main.cpu.debugger.watchpoints;
}

VAmiga::~VAmiga()
{
    emu->halt();
    delete emu;
}

string
VAmiga::version()
{
    return Amiga::version();
}

string
VAmiga::build()
{
    return Amiga::build();
}

bool
VAmiga::isPoweredOn()
{
    return emu->main.isPoweredOn();
}

bool
VAmiga::isPoweredOff()
{
    return emu->main.isPoweredOff();
}

bool
VAmiga::isPaused()
{
    return emu->main.isPaused();
}

bool
VAmiga::isRunning()
{
    return emu->main.isRunning();
}

bool
VAmiga::isSuspended()
{
    return emu->main.isSuspended();
}

bool
VAmiga::isHalted()
{
    return emu->main.isHalted();
}

bool
VAmiga::isWarping()
{
    return emu->isWarping();
}

bool
VAmiga::isTracking()
{
    return emu->isTracking();
}

void
VAmiga::isReady()
{
    return emu->isReady();
}

void
VAmiga::powerOn()
{
    emu->Thread::powerOn();
}

void
VAmiga::powerOff()
{
    emu->Thread::powerOff();
}

void
VAmiga::run()
{
    emu->run();
}

void
VAmiga::pause()
{
    emu->pause();
}

void
VAmiga::halt()
{
    emu->halt();
}

void
VAmiga::suspend()
{
    emu->suspend();
}

void
VAmiga::resume()
{
    emu->resume();
}

void
VAmiga::warpOn(isize source)
{
    emu->warpOn(source);
}

void
VAmiga::warpOff(isize source)
{
    emu->warpOff(source);
}

void
VAmiga::trackOn(isize source)
{
    emu->trackOn(source);
}

void
VAmiga::trackOff(isize source)
{
    emu->trackOff(source);
}

void
VAmiga::stepInto()
{
    assert(isUserThread());
    emu->main.debugger.stepInto();
}

void
VAmiga::stepOver()
{
    assert(isUserThread());
    emu->main.debugger.stepOver();
}

void
VAmiga::wakeUp()
{
    emu->wakeUp();
}

void
VAmiga::launch(const void *listener, Callback *func)
{
    assert(isUserThread());
    emu->launch(listener, func);
}

i64
VAmiga::get(Option option) const
{
    assert(isUserThread());
    return emu->get(option);
}

i64
VAmiga::get(Option option, long id) const
{
    assert(isUserThread());
    return emu->get(option, id);
}

void
VAmiga::set(ConfigScheme model)
{
    assert(isUserThread());
    emu->set(model);
    // emu->main.markAsDirty();
}

void
VAmiga::set(Option opt, i64 value) throws
{
    assert(isUserThread());

    emu->check(opt, value);
    put(CMD_CONFIG_ALL, ConfigCmd { .option = opt, .value = value });
    // emu->main.markAsDirty();
}

void
VAmiga::set(Option opt, i64 value, long id)
{
    assert(isUserThread());

    emu->check(opt, value, { id });
    put(CMD_CONFIG, ConfigCmd { .option = opt, .value = value, .id = id });
    // emu->main.markAsDirty();
}

void
VAmiga::exportConfig(const fs::path &path) const
{
    assert(isUserThread());
    emu->main.exportConfig(path);
}

void
VAmiga::exportConfig(std::ostream& stream) const
{
    assert(isUserThread());
    emu->main.exportConfig(stream);
}

void
VAmiga::put(const Cmd &cmd)
{
    assert(isUserThread());
    emu->put(cmd);
}


//
// AmigaAPI
//

Snapshot *
AmigaAPI::takeSnapshot()
{
    return amiga->takeSnapshot();
}

void
AmigaAPI::loadSnapshot(const Snapshot &snapshot)
{
    amiga->loadSnapshot(snapshot);
}


//
// RetroShellAPI
//

const char *
RetroShellAPI::text()
{
    return retroShell->text();
}

isize
RetroShellAPI::cursorRel()
{
    return retroShell->cursorRel();
}

void
RetroShellAPI::press(RetroShellKey key, bool shift)
{
    retroShell->press(key, shift);
}

void
RetroShellAPI::press(char c)
{
    retroShell->press(c);
}

void
RetroShellAPI::press(const string &s)
{
    retroShell->press(s);
}

void
RetroShellAPI::execScript(std::stringstream &ss)
{
    retroShell->execScript(ss);
}

void
RetroShellAPI::execScript(const std::ifstream &fs)
{
    retroShell->execScript(fs);
}

void
RetroShellAPI::execScript(const string &contents)
{
    retroShell->execScript(contents);
}

/*
void
RetroShellAPI::execScript(const MediaFile &file)
{
    retroShell->execScript(file);
}
*/

void
RetroShellAPI::setStream(std::ostream &os)
{
    retroShell->setStream(os);
}


//
// VideoPortAPI
//

const class FrameBuffer &
VideoPortAPI::getTexture() const
{
    return videoPort->getTexture();
}

}
