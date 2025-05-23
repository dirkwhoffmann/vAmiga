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

class References {

public:

    class Agnus &agnus;
    class Amiga &amiga;
    class AudioPort &audioPort;
    class Blitter &blitter;
    class CIAA &ciaa;
    class CIAB &ciab;
    class ControlPort &controlPort1;
    class ControlPort &controlPort2;
    class Copper &copper;
    class CPU &cpu;
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
    class LogicAnalyzer &logicAnalyzer;
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
    class VideoPort &videoPort;
    class ZorroManager &zorro;

    class FloppyDrive *df[4] = { &df0, &df1, &df2, &df3 };
    class HardDrive *hd[4] = { &hd0, &hd1, &hd2, &hd3 };

public:

    References(Amiga& ref);
};

class SubComponent : public CoreComponent, public References {

protected:

public:

    SubComponent(Amiga& ref, isize id = 0);

    bool isRunAheadInstance() const;
    void prefix(isize level,  const char *component, isize line) const override;
};

}
