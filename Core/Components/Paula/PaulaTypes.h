// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Components/Paula/DiskController/DiskControllerTypes.h"
#include "Components/Paula/Audio/StateMachineTypes.h"
#include "Components/Paula/Audio/AudioFilterTypes.h"
#include "Components/Paula/UART/UARTTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class IrqSource : long
{
    TBE,
    DSKBLK,
    SOFT,
    PORTS,
    COPER,
    VERTB,
    BLIT,
    AUD0,
    AUD1,
    AUD2,
    AUD3,
    RBF,
    DSKSYN,
    EXTER
};

struct IrqSourceEnum : Reflection<IrqSourceEnum, IrqSource>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(IrqSource::EXTER);
    
    static const char *_key(IrqSource value)
    {
        switch (value) {
                
            case IrqSource::TBE:     return "TBE";
            case IrqSource::DSKBLK:  return "DSKBLK";
            case IrqSource::SOFT:    return "SOFT";
            case IrqSource::PORTS:   return "PORTS";
            case IrqSource::COPER:   return "COPER";
            case IrqSource::VERTB:   return "VERTB";
            case IrqSource::BLIT:    return "BLIT";
            case IrqSource::AUD0:    return "AUD0";
            case IrqSource::AUD1:    return "AUD1";
            case IrqSource::AUD2:    return "AUD2";
            case IrqSource::AUD3:    return "AUD3";
            case IrqSource::RBF:     return "RBF";
            case IrqSource::DSKSYN:  return "DSKSYN";
            case IrqSource::EXTER:   return "EXTER";
        }
        return "???";
    }
    static const char *help(IrqSource value)
    {
        switch (value) {
                
            case IrqSource::TBE:     return "Transmit buffer empty";
            case IrqSource::DSKBLK:  return "Floppy drive controller";
            case IrqSource::SOFT:    return "Software interrupt";
            case IrqSource::PORTS:   return "External ports";
            case IrqSource::COPER:   return "Copper interrupt";
            case IrqSource::VERTB:   return "Vertical blank";
            case IrqSource::BLIT:    return "Blitter interrupt";
            case IrqSource::AUD0:    return "Audio channel 0";
            case IrqSource::AUD1:    return "Audio channel 1";
            case IrqSource::AUD2:    return "Audio channel 2";
            case IrqSource::AUD3:    return "Audio channel 3";
            case IrqSource::RBF:     return "Receive buffer full";
            case IrqSource::DSKSYN:  return "Floppy drive controller";
            case IrqSource::EXTER:   return "External interrupt";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    u16 intreq;
    u16 intena;
    u16 adkcon;
}
PaulaInfo;

}
