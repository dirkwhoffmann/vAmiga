// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "SubComponent.h"
#include "Emulator.h"

namespace vamiga {

References::References(Amiga& ref) :

agnus(ref.agnus),
amiga(ref),
audioPort(ref.audioPort),
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
host(ref.host),
keyboard(ref.keyboard),
logicAnalyzer(ref.logicAnalyzer),
mem(ref.mem),
monitor(ref.monitor),
msgQueue(ref.msgQueue),
osDebugger(ref.osDebugger),
paula(ref.paula),
pixelEngine(ref.denise.pixelEngine),
ramExpansion(ref.ramExpansion),
remoteManager(ref.remoteManager),
retroShell(ref.retroShell),
rtc(ref.rtc),
serialPort(ref.serialPort),
uart(ref.paula.uart),
videoPort(ref.videoPort),
zorro(ref.zorro)
{
};

SubComponent::SubComponent(Amiga& ref, isize id) : CoreComponent(ref.emulator, id), References(ref) { };

bool
SubComponent::isRunAheadInstance() const
{
    return amiga.isRunAheadInstance();
}

void
SubComponent::prefix(isize level, const char *component, isize line) const
{
    amiga.prefix(level, component, line);
}

}
