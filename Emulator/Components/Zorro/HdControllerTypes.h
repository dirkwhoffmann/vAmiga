// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

//
// Constants
//

#ifdef __cplusplus

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

#endif

//
// Enumerations
//

enum_long(IO_CMD)
{
    // Standard commands
    CMD_INVALID,            // 0
    CMD_RESET,              // 1
    CMD_READ,               // 2
    CMD_WRITE,              // 3
    CMD_UPDATE,             // 4
    CMD_CLEAR,              // 5
    CMD_STOP,               // 6
    CMD_START,              // 7
    CMD_FLUSH,              // 8
    
    // Nonstandard commands
    CMD_TD_MOTOR,           // 9
    CMD_TD_SEEK,            // 10
    CMD_TD_FORMAT,          // 11
    CMD_TD_REMOVE,          // 12
    CMD_TD_CHANGENUM,       // 13
    CMD_TD_CHANGESTATE,     // 14
    CMD_TD_PROTSTATUS,      // 15
    CMD_TD_RAWREAD,         // 16
    CMD_TD_RAWWRITE,        // 17
    CMD_TD_GETDRIVETYPE,    // 18
    CMD_TD_GETNUMTRACKS,    // 19
    CMD_TD_ADDCHANGEINT,    // 20
    CMD_TD_REMCHANGEINT,    // 21
    CMD_TD_GETGEOMETRY,     // 22
    CMD_TD_EJECT,           // 23
    CMD_TD_LASTCOMM         // 24
};
typedef IO_CMD IoCommand;

#ifdef __cplusplus
struct IoCommandEnum : vamiga::util::Reflection<IoCommandEnum, IoCommand>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CMD_TD_LASTCOMM;

    static const char *prefix() { return "CMD"; }
    static const char *_key(long value)
    {
        switch (value) {

            case CMD_INVALID:           return "INVALID";
            case CMD_RESET:             return "RESET";
            case CMD_READ:              return "READ";
            case CMD_WRITE:             return "WRITE";
            case CMD_UPDATE:            return "UPDATE";
            case CMD_CLEAR:             return "CLEAR";
            case CMD_STOP:              return "STOP";
            case CMD_START:             return "START";
            case CMD_FLUSH:             return "FLUSH";
                
            case CMD_TD_MOTOR:          return "TD_MOTOR";
            case CMD_TD_SEEK:           return "TD_SEEK";
            case CMD_TD_FORMAT:         return "TD_FORMAT";
            case CMD_TD_REMOVE:         return "TD_REMOVE";
            case CMD_TD_CHANGENUM:      return "TD_CHANGENUM";
            case CMD_TD_CHANGESTATE:    return "TD_CHANGESTATE";
            case CMD_TD_PROTSTATUS:     return "TD_PROTSTATUS";
            case CMD_TD_RAWREAD:        return "TD_RAWREAD";
            case CMD_TD_RAWWRITE:       return "TD_RAWWRITE";
            case CMD_TD_GETDRIVETYPE:   return "TD_GETDRIVETYPE";
            case CMD_TD_GETNUMTRACKS:   return "TD_GETNUMTRACKS";
            case CMD_TD_ADDCHANGEINT:   return "TD_ADDCHANGEINT";
            case CMD_TD_REMCHANGEINT:   return "TD_REMCHANGEINT";
            case CMD_TD_GETGEOMETRY:    return "TD_GETGEOMETRY";
            case CMD_TD_EJECT:          return "TD_EJECT";
            case CMD_TD_LASTCOMM:       return "TD_LASTCOMM";
        }
        return "???";
    }
};
#endif

enum_long(HDC_STATE)
{
    HDC_UNDETECTED,     // The controller waits to be detected by the OS
    HDC_INITIALIZING,   // The controller has been detected
    HDC_READY           // The controller is fully operational
};
typedef HDC_STATE HdcState;

#ifdef __cplusplus
struct HdcStateEnum : vamiga::util::Reflection<HdcStateEnum, HdcState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDC_READY;
        
    static const char *prefix() { return "HDC"; }
    static const char *_key(long value)
    {
        switch (value) {

            case HDC_UNDETECTED:    return "UNDETECTED";
            case HDC_INITIALIZING:  return "INITIALIZING";
            case HDC_READY:         return "READY";
        }
        return "???";
    }
};
#endif


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
