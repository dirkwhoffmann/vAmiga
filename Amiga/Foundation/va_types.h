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

// 2 bit binary value
typedef uint8_t uint2_t;
inline bool is_uint2_t(uint2_t value) { return value <= 0x03; }

// 24 bit binary value
typedef uint32_t uint24_t;
inline bool is_uint24_t(uint24_t value) { return value <= 0xFFFFFF; }


//
// Clocks
//

typedef int64_t Cycle;    // Cycle in master cycle units
typedef int64_t CPUCycle; // Cycle in CPU cycle units
typedef int64_t CIACycle; // Cycle in CIA cycle units
typedef int64_t DMACycle; // Cycle in DMA cycle units

#define CPU_CYCLES(cycles) ((cycles) << 2)
#define CIA_CYCLES(cycles) ((cycles) * 40)
#define DMA_CYCLES(cycles) ((cycles) << 3)

#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)


//
// Amiga
//

typedef enum : long
{
    A500,
    A1000,
    A2000
}
AmigaModel;

inline bool isAmigaModel(AmigaModel model)
{
    return model >= A500 && model <= A2000;
}

inline const char *modelName(AmigaModel model)
{
    return
    model == A500 ? "Amiga 500" :
    model == A1000 ? "Amiga 1000" :
    model == A2000 ? "Amiga 2000" : "???";
}


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
    MEM_BOOT,
    MEM_KICK
}
MemorySource;


//
// Floppy drive
//

typedef enum : long
{
    A1010_ORIG, // Amiga 3,5" drive, emulated with original speed
    A1010_2X,   // Amiga 3,5" drive, emulated 2x faster
    A1010_4X,   // Amiga 3,5" drive, emulated 4x faster
    A1010_8X,   // Amiga 3,5" drive, emulated 8x faster
    A1010_WARP  // Amiga 3,5" drive, emulated as fast as possible
}
DriveType;

inline bool isDriveType(DriveType model)
{
    return model >= A1010_ORIG && model <= A1010_WARP;
}

inline const char *driveTypeName(DriveType type)
{
    return
    type == A1010_ORIG ? "A1010 (original speed)" :
    type == A1010_2X   ? "A1010 (2x faster)" :
    type == A1010_4X   ? "A1010 (4x faster)" :
    type == A1010_8X   ? "A1010 (8x faster)" :
    type == A1010_WARP ? "A1010 (warp speed)" : "???";
}


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
    FILETYPE_KICK_ROM
}
AmigaFileType;

inline bool isVAFileType(AmigaFileType model) {
    return model >= FILETYPE_UKNOWN && model <= FILETYPE_KICK_ROM;
}


//
// Notification messages (GUI communication)
//

// List of all known message id's
typedef enum
{
    MSG_NONE = 0,
    
    // Emulator state
    MSG_CONFIG,
    MSG_READY_TO_POWER_ON,
    MSG_POWER_ON,
    MSG_POWER_OFF,
    MSG_RUN,
    MSG_PAUSE,
    MSG_INSPECT,
    MSG_RESET,
    MSG_ROM_MISSING,
    MSG_WARP_ON,
    MSG_WARP_OFF,
    MSG_POWER_LED_ON,
    MSG_POWER_LED_OFF,

    // CPU
    MSG_BREAKPOINT_CONFIG,
    MSG_BREAKPOINT_REACHED,
    
    // Memory
    MSG_MEM_LAYOUT,
    
    // Keyboard
    MSG_MAP_CMD_KEYS,
    MSG_UNMAP_CMD_KEYS,
    
    // Floppy drives
    MSG_DRIVE_CONNECT,
    MSG_DRIVE_DISCONNECT,
    MSG_DRIVE_LED_ON,
    MSG_DRIVE_LED_OFF,
    MSG_DRIVE_DISK_INSERT,
    MSG_DRIVE_DISK_EJECT,
    MSG_DRIVE_DISK_SAVED,
    MSG_DRIVE_DISK_UNSAVED,
    MSG_DRIVE_DISK_PROTECTED,
    MSG_DRIVE_DISK_UNPROTECTED,
    MSG_DRIVE_DMA_ON,
    MSG_DRIVE_DMA_OFF,
    MSG_DRIVE_HEAD_UP,
    MSG_DRIVE_HEAD_DOWN,
    
    
    
    //
    // DEPRECATED C64 related messages ...
    //
    
    // ROM and snapshot handling
    MSG_SNAPSHOT_TAKEN,
    
    // CPU related messages
    MSG_CPU_OK,
    MSG_CPU_SOFT_BREAKPOINT_REACHED,
    MSG_CPU_HARD_BREAKPOINT_REACHED,
    MSG_CPU_ILLEGAL_INSTRUCTION,
    
    // Keyboard
    MSG_KEYMATRIX,
    MSG_CHARSET,
    
    // Peripherals (Disk drive)
    MSG_VC1541_ATTACHED,
    MSG_VC1541_ATTACHED_SOUND,
    MSG_VC1541_DETACHED,
    MSG_VC1541_DETACHED_SOUND,
    MSG_VC1541_DISK,
    MSG_VC1541_DISK_SOUND,
    MSG_VC1541_NO_DISK,
    MSG_VC1541_NO_DISK_SOUND,
    MSG_VC1541_MOTOR_ON,
    MSG_VC1541_MOTOR_OFF,
    
    // Peripherals (Disk)
    MSG_DISK_SAVED,
    MSG_DISK_UNSAVED,
}
MessageType;

/* A single message
 * Only a very messages utilize the data file. E.g., the drive related message
 * use it to code the drive number (0 = df0 etc.).
 */
typedef struct
{
    MessageType type;
    long data;
}
Message;

// Callback function signature
typedef void Callback(const void *, int, long);


//
// Configurations
//

/* Amiga configuration
 * This is a full description of the computer we are going to emulate.
 */
typedef struct
{
    DriveType type;
    bool connected;
}
DriveConfiguration;

typedef struct
{
    AmigaModel model;
    long layout;
    bool realTimeClock;
    DriveConfiguration df0;
    DriveConfiguration df1;
}
AmigaConfiguration;

typedef struct
{
    long chipRamSize; // size in KB
    long slowRamSize; // size in KB
    long fastRamSize; // size in KB
}
AmigaMemConfiguration;


//
// Info structures (Filled by calling inspect() )
//

typedef struct
{
    Cycle masterClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
AmigaInfo;

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
    uint16_t dmacon;
    uint16_t diwstrt;
    uint16_t diwstop;
    uint16_t ddfstrt;
    uint16_t ddfstop;
    
    uint16_t bpl1mod;
    uint16_t bpl2mod;
    uint8_t  numBpls;
    
    uint32_t dskpt;
    uint32_t audlc[4];
    uint32_t bplpt[6];
    uint32_t sprptr[8];
}
DMAInfo;

typedef struct
{
    const char *slotName;
    const char *eventName;
    long eventId;
    Cycle trigger;
    Cycle triggerRel;
    long frame;
    long vpos;
    long hpos;
}
EventSlotInfo;

typedef struct
{
    Cycle dmaClock;
    EventSlotInfo primary[7];
    EventSlotInfo secondary[15];
}
EventHandlerInfo;

typedef struct
{
    bool active;
    bool cdang;
    uint32_t coppc;
    uint32_t coplc[2];
    uint16_t copins[2];
}
CopperInfo;

typedef struct
{
    bool active;
    uint16_t bltcon0;
    uint16_t bltcon1;
    uint16_t bltapt;
    uint16_t bltbpt;
    uint16_t bltcpt;
    uint16_t bltdpt;
    uint16_t bltafwm;
    uint16_t bltalwm;
    uint16_t bltsize;
    uint16_t bltamod;
    uint16_t bltbmod;
    uint16_t bltcmod;
    uint16_t bltdmod;
    uint16_t anew;
    uint16_t bnew;
    uint16_t ahold;
    uint16_t bhold;
    uint16_t chold;
    uint16_t dhold;
    bool bbusy;
    bool bzero;
}
BlitterInfo;

typedef struct
{
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;
    uint16_t bpldat[6];
    uint32_t color[32];
}
DeniseInfo;

typedef struct
{
    uint16_t intreq;
    uint16_t intena;
}
PaulaInfo;

#endif
