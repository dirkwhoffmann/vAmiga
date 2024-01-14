// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreComponent.h"

namespace vamiga {

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
class DiagBoard;
class DiskController;
class DmaDebugger;
class FloppyDrive;
class HardDrive;
class HdController;
class Host;
class GdbServer;
class Joystick;
class Keyboard;
class Memory;
class Mouse;
class MsgQueue;
class OSDebugger;
class Paula;
class PixelEngine;
class RamExpansion;
class RemoteManager;
class RetroShell;
class RshServer;
class RTC;
class SerialPort;
class UART;
class ZorroManager;

/* This class extends the CoreComponent class with references to all components
 * that are part of the Amiga class.
 */
class SubComponent : public CoreComponent {

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
    DiagBoard &diagBoard;
    DiskController &diskController;
    DmaDebugger &dmaDebugger;
    FloppyDrive &df0;
    FloppyDrive &df1;
    FloppyDrive &df2;
    FloppyDrive &df3;
    HardDrive &hd0;
    HardDrive &hd1;
    HardDrive &hd2;
    HardDrive &hd3;
    HdController &hd0con;
    HdController &hd1con;
    HdController &hd2con;
    HdController &hd3con;
    Host &host;
    Keyboard &keyboard;
    Memory &mem;
    MsgQueue &msgQueue;
    OSDebugger &osDebugger;
    Paula &paula;
    PixelEngine &pixelEngine;
    RamExpansion &ramExpansion;
    RemoteManager &remoteManager;
    RetroShell &retroShell;
    RTC &rtc;
    SerialPort &serialPort;
    UART &uart;
    ZorroManager &zorro;

    FloppyDrive *df[4] = { &df0, &df1, &df2, &df3 };

public:

    SubComponent(Amiga& ref);

    virtual bool isPoweredOff() const override;
    virtual bool isPoweredOn() const override;
    virtual bool isPaused() const override;
    virtual bool isRunning() const override;
    virtual bool isSuspended() const override;
    virtual bool isHalted() const override;

    void suspend() override;
    void resume() override;
    
    void prefix() const override;
};

}
