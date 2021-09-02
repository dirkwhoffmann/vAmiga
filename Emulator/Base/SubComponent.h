// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

class Agnus;
class Amiga;
class Blitter;
class CPU;
class CIA;
class CIAA;
class CIAB;
class ControlPort;
class Copper;
class Denise;
class DiskController;
class DmaDebugger;
class Drive;
class Joystick;
class Keyboard;
class Memory;
class Mouse;
class MsgQueue;
class Paula;
class PixelEngine;
class RetroShell;
class RTC;
class Scheduler;
class SerialPort;
class UART;
class ZorroManager;

/* This class extends the AmigaComponent class with references to all components
 * that are part of the Amiga class.
 */
class SubComponent : public AmigaComponent {

protected:

    Agnus &agnus;
    Amiga &amiga;
    Blitter &blitter;
    CIAA &ciaa;
    CIAB &ciab;
    ControlPort &controlPort1;
    ControlPort &controlPort2;
    Copper &copper;
    CPU &cpu;
    Denise &denise;
    DiskController &diskController;
    DmaDebugger &dmaDebugger;
    Drive &df0;
    Drive &df1;
    Drive &df2;
    Drive &df3;
    Keyboard &keyboard;
    Memory &mem;
    MsgQueue &msgQueue;
    Paula &paula;
    PixelEngine &pixelEngine;
    RetroShell &retroShell;
    RTC &rtc;
    Scheduler &scheduler;
    SerialPort &serialPort;
    UART &uart;
    ZorroManager &zorro;

    Drive *df[4] = { &df0, &df1, &df2, &df3 };

public:

    SubComponent(Amiga& ref);

    virtual bool isPoweredOff() const override;
    virtual bool isPoweredOn() const override;
    virtual bool isPaused() const override;
    virtual bool isRunning() const override; 

    void suspend() override;
    void resume() override;
    
    void prefix() const override;
};
