// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"  

AmigaComponent::AmigaComponent(Amiga& ref) :
amiga(ref),
cpu(ref.cpu),
ciaa(ref.ciaA),
ciab(ref.ciaB),
rtc(ref.rtc),
mem(ref.mem),
agnus(ref.agnus),
copper(ref.agnus.copper),
blitter(ref.agnus.blitter),
dmaDebugger(ref.agnus.dmaDebugger),
denise(ref.denise),
pixelEngine(ref.denise.pixelEngine),
paula(ref.paula),
audioUnit(ref.paula.audioUnit),
diskController(ref.paula.diskController),
uart(ref.paula.uart),
zorro(ref.zorro),
controlPort1(ref.controlPort1),
controlPort2(ref.controlPort2),
serialPort(ref.serialPort),
mouse(ref.mouse),
joystick1(ref.joystick1),
joystick2(ref.joystick2),
keyboard(ref.keyboard),
df0(ref.df0),
df1(ref.df1),
df2(ref.df2),
df3(ref.df3)
{

};

void AmigaComponent::prefix() { amiga.prefix(); }
