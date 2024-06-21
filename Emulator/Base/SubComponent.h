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
class Debugger;
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

class References {

protected:

    class Agnus &agnus;
    class Amiga &amiga;
    class Blitter &blitter;
    class CIAA &ciaa;
    class CIAB &ciab;
    class ControlPort &controlPort1;
    class ControlPort &controlPort2;
    class Copper &copper;
    class CPU &cpu;
    class Debugger &debugger;
    class Denise &denise;
    class DiagBoard &diagBoard;
    class DiskController &diskController;
    class DmaDebugger &dmaDebugger;
    class FloppyDrive &df0;
    class FloppyDrive &df1;
    class FloppyDrive &df2;
    class FloppyDrive &df3;
    class HardDrive &hd0;
    class HardDrive &hd1;
    class HardDrive &hd2;
    class HardDrive &hd3;
    class HdController &hd0con;
    class HdController &hd1con;
    class HdController &hd2con;
    class HdController &hd3con;
    class Host &host;
    class Keyboard &keyboard;
    class Memory &mem;
    class MsgQueue &msgQueue;
    class OSDebugger &osDebugger;
    class Paula &paula;
    class PixelEngine &pixelEngine;
    class RamExpansion &ramExpansion;
    class RemoteManager &remoteManager;
    class RetroShell &retroShell;
    class RTC &rtc;
    class SerialPort &serialPort;
    class UART &uart;
    class ZorroManager &zorro;

    class FloppyDrive *df[4] = { &df0, &df1, &df2, &df3 };

public:

    References(Amiga& ref);
};


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
    Debugger &debugger;
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
    SubComponent(Amiga& ref, isize id);

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
