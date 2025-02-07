// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Constants
//

// Constants from exec/errors.h
constexpr i8    IOERR_OPENFAIL      = -1;
constexpr i8    IOERR_ABORTED       = -2;
constexpr i8    IOERR_NOCMD         = -3;
constexpr i8    IOERR_BADLENGTH     = -4;
constexpr i8    IOERR_BADADDRESS    = -5;
constexpr i8    IOERR_UNITBUSY      = -6;
constexpr i8    IOERR_SELFTEST      = -7;

// Offsets into the IOStdReq struct
constexpr u32   IO_COMMAND          = 0x1C;
constexpr u32   IO_ERROR            = 0x1F;
constexpr u32   IO_ACTUAL           = 0x20;
constexpr u32   IO_LENGTH           = 0x24;
constexpr u32   IO_DATA             = 0x28;
constexpr u32   IO_OFFSET           = 0x2C;


//
// Enumerations
//

enum class IoCommand
{
    // Standard commands
    INVALID,            // 0
    RESET,              // 1
    READ,               // 2
    WRITE,              // 3
    UPDATE,             // 4
    CLEAR,              // 5
    STOP,               // 6
    START,              // 7
    FLUSH,              // 8
    
    // Nonstandard commands
    TD_MOTOR,           // 9
    TD_SEEK,            // 10
    TD_FORMAT,          // 11
    TD_REMOVE,          // 12
    TD_CHANGENUM,       // 13
    TD_CHANGESTATE,     // 14
    TD_PROTSTATUS,      // 15
    TD_RAWREAD,         // 16
    TD_RAWWRITE,        // 17
    TD_GETDRIVETYPE,    // 18
    TD_GETNUMTRACKS,    // 19
    TD_ADDCHANGEINT,    // 20
    TD_REMCHANGEINT,    // 21
    TD_GETGEOMETRY,     // 22
    TD_EJECT,           // 23
    TD_LASTCOMM         // 24
};

struct IoCommandEnum : Reflection<IoCommandEnum, IoCommand>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(IoCommand::TD_LASTCOMM);
    
    static const char *_key(IoCommand value)
    {
        switch (value) {
                
            case IoCommand::INVALID:           return "INVALID";
            case IoCommand::RESET:             return "RESET";
            case IoCommand::READ:              return "READ";
            case IoCommand::WRITE:             return "WRITE";
            case IoCommand::UPDATE:            return "UPDATE";
            case IoCommand::CLEAR:             return "CLEAR";
            case IoCommand::STOP:              return "STOP";
            case IoCommand::START:             return "START";
            case IoCommand::FLUSH:             return "FLUSH";
                
            case IoCommand::TD_MOTOR:          return "TD_MOTOR";
            case IoCommand::TD_SEEK:           return "TD_SEEK";
            case IoCommand::TD_FORMAT:         return "TD_FORMAT";
            case IoCommand::TD_REMOVE:         return "TD_REMOVE";
            case IoCommand::TD_CHANGENUM:      return "TD_CHANGENUM";
            case IoCommand::TD_CHANGESTATE:    return "TD_CHANGESTATE";
            case IoCommand::TD_PROTSTATUS:     return "TD_PROTSTATUS";
            case IoCommand::TD_RAWREAD:        return "TD_RAWREAD";
            case IoCommand::TD_RAWWRITE:       return "TD_RAWWRITE";
            case IoCommand::TD_GETDRIVETYPE:   return "TD_GETDRIVETYPE";
            case IoCommand::TD_GETNUMTRACKS:   return "TD_GETNUMTRACKS";
            case IoCommand::TD_ADDCHANGEINT:   return "TD_ADDCHANGEINT";
            case IoCommand::TD_REMCHANGEINT:   return "TD_REMCHANGEINT";
            case IoCommand::TD_GETGEOMETRY:    return "TD_GETGEOMETRY";
            case IoCommand::TD_EJECT:          return "TD_EJECT";
            case IoCommand::TD_LASTCOMM:       return "TD_LASTCOMM";
        }
        return "???";
    }
    static const char *help(IoCommand value)
    {
        return "";
    }
};

enum class HdcState
{
    UNDETECTED,     // The controller waits to be detected by the OS
    INITIALIZING,   // The controller has been detected
    READY           // The controller is fully operational
};

struct HdcStateEnum : Reflection<HdcStateEnum, HdcState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(HdcState::READY);
    
    static const char *_key(HdcState value)
    {
        switch (value) {
                
            case HdcState::UNDETECTED:    return "UNDETECTED";
            case HdcState::INITIALIZING:  return "INITIALIZING";
            case HdcState::READY:         return "READY";
        }
        return "???";
    }
    static const char *help(HdcState value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    bool connected;
}
HdcConfig;

typedef struct
{
    isize nr;
    bool pluggedIn;
    HdcState state;
}
HdcInfo;

typedef struct
{
    // Tracks the number of executed commands
    isize cmdCount[25];
}
HdcStats;

}
