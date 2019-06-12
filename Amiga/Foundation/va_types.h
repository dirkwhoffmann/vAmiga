// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VATYPES_H
#define _VATYPES_H

#include "AmigaTypes.h"
#include "MessageQueueTypes.h"
#include "EventHandlerTypes.h"
#include "AgnusTypes.h"
#include "DeniseTypes.h"
#include "PaulaTypes.h"
#include "ControlPortTypes.h"
#include "DriveTypes.h"
#include "DiskTypes.h"


// 2 bit binary value
typedef uint8_t uint2_t;
inline bool is_uint2_t(uint2_t value) { return value <= 0x03; }

// 24 bit binary value
typedef uint32_t uint24_t;
inline bool is_uint24_t(uint24_t value) { return value <= 0xFFFFFF; }


//
// CPU
//

/* Recorded instruction
 * This data structure is used inside the trace ringbuffer. In trace mode,
 * the program counter and the status register are recorded. The instruction
 * string is computed on-the-fly due to speed reasons.
 * DEPRECATED
 */
typedef struct
{
    Cycle cycle;
    uint16_t vhcount; 
    uint32_t pc;
    uint32_t sp;
    // char instr[63];
    // char flags[17];
}
RecordedInstruction;

// A disassembled instruction
typedef struct
{
    uint8_t bytes;  // Length of the disassembled command in bytes
    char addr[9];   // Textual representation of the instruction's address
    char data[33];  // Textual representation of the instruction's data bytes
    char flags[17]; // Textual representation of the status register (optional)
    char instr[65]; // Textual representation of the instruction
}
DisassembledInstruction;


//
// CIA
//

typedef union
{
    struct
    {
        uint8_t hi;
        uint8_t mid;
        uint8_t lo;
    };
    uint32_t value;
}
Counter24;


//
// Memory
//

/* Memory source identifiers
 * The identifiers are used in the mem source lookup table to specify the
 * source and target of a peek or poke operation, respectively.
 */
typedef enum
{
    MEM_UNMAPPED,
    MEM_CHIP,
    MEM_FAST,
    MEM_SLOW,
    MEM_CIA,
    MEM_RTC,
    MEM_OCS,
    MEM_AUTOCONF,
    MEM_BOOT,
    MEM_KICK
}
MemorySource;


//
// Game pads
//

typedef enum
{
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_FIRE
}
JoystickDirection;

typedef enum
{
    PULL_UP,
    PULL_DOWN,
    PULL_LEFT,
    PULL_RIGHT,
    PRESS_FIRE,
    RELEASE_X,
    RELEASE_Y,
    RELEASE_XY,
    RELEASE_FIRE
}
JoystickEvent;


//
// Video
//

typedef enum : long
{
    COLOR_PALETTE = 0,
    BLACK_WHITE_PALETTE,
    PAPER_WHITE_PALETTE,
    GREEN_PALETTE,
    AMBER_PALETTE,
    SEPIA_PALETTE
}
Palette;

inline bool isPalette(Palette model) {
    return model >= COLOR_PALETTE && model <= SEPIA_PALETTE;
}


//
// External files (snapshots, disk images, etc.)
//

typedef enum
{
    FILETYPE_UKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_BOOT_ROM,
    FILETYPE_KICK_ROM,
    FILETYPE_EXT_ROM
}
AmigaFileType;

inline bool isAmigaFileType(long value) {
    return value >= FILETYPE_UKNOWN && value <= FILETYPE_EXT_ROM;
}


//
// Configurations
//


//
// Info structures (Filled by calling inspect() )
//


#define CPUINFO_INSTR_COUNT 32

typedef struct
{
    // Registers
    uint32_t pc;
    uint32_t d[8];
    uint32_t a[8];
    uint32_t ssp;
    uint16_t flags;
    
    // Disassembled instructions starting at pc
    DisassembledInstruction instr[CPUINFO_INSTR_COUNT];
    
    // Disassembled instructions from the trace buffer
    DisassembledInstruction traceInstr[CPUINFO_INSTR_COUNT];
}
CPUInfo;

typedef struct
{
    Counter24 value;
    Counter24 latch;
    Counter24 alarm;
}
CounterInfo;

typedef struct
{
    struct {
        uint8_t port;
        uint8_t reg;
        uint8_t dir;
    } portA;
    
    struct {
        uint8_t port;
        uint8_t reg;
        uint8_t dir;
    } portB;
    
    struct {
        uint16_t count;
        uint16_t latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerA;
    
    struct {
        uint16_t count;
        uint16_t latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerB;
    
    uint8_t sdr;
    uint8_t icr;
    uint8_t imr;
    bool intLine;
    CounterInfo cnt;
    bool cntIntEnable;
    Cycle idleCycles;
    double idlePercentage;
}
CIAInfo;

typedef struct
{
    const char *slotName;
    const char *eventName;
    long eventId;
    Cycle trigger;
    Cycle triggerRel;

    // The trigger cycle translated to a beam position.
    long vpos;
    long hpos;

    // Indicates if (vpos, hpos) is a position inside the current frame.
    bool currentFrame;
}
EventSlotInfo;

typedef struct
{
    Cycle masterClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
    
    EventSlotInfo primary[PRIM_SLOT_COUNT];
    EventSlotInfo secondary[SEC_SLOT_COUNT];
}
EventHandlerInfo;

#endif
