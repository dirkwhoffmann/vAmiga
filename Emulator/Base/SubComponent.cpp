// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "SubComponent.h"
#include "Amiga.h"  

SubComponent::SubComponent(Amiga& ref) :
agnus(ref.agnus),
amiga(ref),
blitter(ref.agnus.blitter),
ciaa(ref.ciaA),
ciab(ref.ciaB),
controlPort1(ref.controlPort1),
controlPort2(ref.controlPort2),
copper(ref.agnus.copper),
cpu(ref.cpu),
denise(ref.denise),
diagBoard(ref.diagBoard),
diskController(ref.paula.diskController),
dmaDebugger(ref.agnus.dmaDebugger),
df0(ref.df0),
df1(ref.df1),
df2(ref.df2),
df3(ref.df3),
hd0(ref.hd0),
hd1(ref.hd1),
hd2(ref.hd2),
hd3(ref.hd3),
hd0con(ref.hd0con),
hd1con(ref.hd1con),
hd2con(ref.hd2con),
hd3con(ref.hd3con),
keyboard(ref.keyboard),
mem(ref.mem),
msgQueue(ref.msgQueue),
osDebugger(ref.osDebugger),
paula(ref.paula),
pixelEngine(ref.denise.pixelEngine),
ramExpansion(ref.ramExpansion),
remoteManager(ref.remoteManager),
retroShell(ref.retroShell),
rtc(ref.rtc),
scheduler(ref.agnus.scheduler),
serialPort(ref.serialPort),
uart(ref.paula.uart),
zorro(ref.zorro)
{
};

bool
SubComponent::isPoweredOff() const
{
    return amiga.isPoweredOff();
}

bool
SubComponent::isPoweredOn() const
{
    return amiga.isPoweredOn();
}

bool
SubComponent::isPaused() const
{
    return amiga.isPaused();
}

bool
SubComponent::isRunning() const
{
    return amiga.isRunning();
}

bool
SubComponent::isSuspended() const
{
    return amiga.isSuspended();
}

bool
SubComponent::isHalted() const
{
    return amiga.isHalted();
}

void
SubComponent::suspend()
{
    amiga.suspend();
}

void
SubComponent::resume()
{
    amiga.resume();
}

void
SubComponent::prefix() const
{
    amiga.prefix();
}
