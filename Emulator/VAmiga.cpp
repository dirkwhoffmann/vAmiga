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
    // emu->suspend();
}

void
API::resume()
{
    // emu->resume();
}

bool
API::isUserThread() const
{
    // return !emu->isEmulatorThread();
    return false;
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
    host.host = &emu->main.host;

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

    watchpoints.emu = emu;
    watchpoints.guards = &emu->main.cpu.debugger.watchpoints;
}

VAmiga::~VAmiga()
{
    // emu->halt();
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

}
