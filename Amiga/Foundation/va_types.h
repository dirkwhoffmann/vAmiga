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

enum RunLoopControlFlag
{
    RL_SNAPSHOT           = 0b00001,
    RL_INSPECT            = 0b00010,
    RL_ENABLE_TRACING     = 0b00100,
    RL_ENABLE_BREAKPOINTS = 0b01000,
    RL_STOP               = 0b10000,
    
    RL_DEBUG              = 0b01100
};


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

typedef enum : uint32_t
{
    DRIVE_ID_NONE  = 0x00000000,
    DRIVE_ID_35DD  = 0xFFFFFFFF,
    DRIVE_ID_35HD  = 0xAAAAAAAA,
    DRIVE_ID_525SD = 0x55555555
}
DriveIdCode;

typedef enum
{
    DRIVE_DMA_OFF = 0,   // Drive is idle
    DRIVE_DMA_SYNC_WAIT, // Drive is waiting for the sync word
    DRIVE_DMA_READ,      // Drive is reading (via DMA)
    DRIVE_DMA_WRITE      // Drive is writing (via DMA)
}
DriveState;


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
// Event handler
//

typedef enum : long
{
    //
    // Primary slot table
    //
    
    CIAA_SLOT = 0,    // CIA A execution
    CIAB_SLOT,        // CIA B execution
    DMA_SLOT,         // Disk, Audio, Sprite, and Bitplane DMA
    COP_SLOT,         // Copper DMA
    BLT_SLOT,         // Blitter DMA
    RAS_SLOT,         // Raster line events
    SEC_SLOT,         // Secondary events
    PRIM_SLOT_COUNT,
    
    //
    // Secondary slot table
    //
    
    DSK_SLOT = 0,     // Disk controller
    IRQ_TBE_SLOT,     // Source 0 IRQ (Serial port transmit buffer empty)
    IRQ_DSKBLK_SLOT,  // Source 1 IRQ (Disk block finished)
    IRQ_SOFT_SLOT,    // Source 2 IRQ (Software-initiated)
    IRQ_PORTS_SLOT,   // Source 3 IRQ (I/O ports and CIA A)
    IRQ_COPR_SLOT,    // Source 4 IRQ (Copper)
    IRQ_VERTB_SLOT,   // Source 5 IRQ (Start of vertical blank)
    IRQ_BLIT_SLOT,    // Source 6 IRQ (Blitter finished)
    IRQ_AUD0_SLOT,    // Source 7 IRQ (Audio channel 0 block finished)
    IRQ_AUD1_SLOT,    // Source 8 IRQ (Audio channel 1 block finished)
    IRQ_AUD2_SLOT,    // Source 9 IRQ (Audio channel 2 block finished)
    IRQ_AUD3_SLOT,    // Source 10 IRQ (Audio channel 3 block finished)
    IRQ_RBF_SLOT,     // Source 11 IRQ (Serial port receive buffer full)
    IRQ_DSKSYN_SLOT,  // Source 12 IRQ (Disk sync register matches disk data)
    IRQ_EXTER_SLOT,   // Source 13 IRQ (I/O ports and CIA B)
    INSPECTOR_SLOT,   // Handles periodic calls to inspect()
    SEC_SLOT_COUNT,
} EventSlot;

static inline bool isPrimarySlot(int32_t s) { return s <= PRIM_SLOT_COUNT; }
static inline bool isSecondarySlot(int32_t s) { return s <= SEC_SLOT_COUNT; }

typedef enum : long
{
    EVENT_NONE = 0,
    
    //
    // Events in the primary event table
    //
    
    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // DMA slot
    DMA_DISK = 1,
    DMA_A0,
    DMA_A1,
    DMA_A2,
    DMA_A3,
    DMA_S0,
    DMA_S1,
    DMA_S2,
    DMA_S3,
    DMA_S4,
    DMA_S5,
    DMA_S6,
    DMA_S7,
    DMA_L1,
    DMA_L2,
    DMA_L3,
    DMA_L4,
    DMA_L5,
    DMA_L6,
    DMA_H1,
    DMA_H2,
    DMA_H3,
    DMA_H4,
    DMA_EVENT_COUNT,
    
    // Copper slot
    COP_REQUEST_DMA = 1,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT,
    COP_SKIP,
    COP_JMP1,
    COP_JMP2,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_INIT = 1,
    BLT_EXECUTE,
    BLT_FAST_BLIT,
    BLT_EVENT_COUNT,
    
    // Raster slot
    RAS_HSYNC = 1,
    RAS_DIWSTRT,
    RAS_DIWDRAW,
    RAS_EVENT_COUNT,
    
    // SEC slot
    SEC_TRIGGER = 1,
    SEC_EVENT_COUNT,
    
    //
    // Events in secondary event table
    //
    
    // Disk controller slot
    DSK_ROTATE = 1,
    
    // IRQ slots
    IRQ_SET = 1,
    IRQ_CLEAR,
    IRQ_EVENT_COUNT,
    
    // Inspector slot
    INS_NONE = 1,
    INS_AMIGA,
    INS_CPU,
    INS_MEM,
    INS_CIA,
    INS_AGNUS,
    INS_PAULA,
    INS_DENISE,
    INS_EVENTS
} EventID;

static inline bool isCiaEvent(EventID id) { return id <= CIA_EVENT_COUNT; }
static inline bool isDmaEvent(EventID id) { return id <= DMA_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id <= COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id <= BLT_EVENT_COUNT; }
static inline bool isRasEvent(EventID id) { return id <= RAS_EVENT_COUNT; }

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;


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
    MSG_DRIVE_MOTOR_ON,
    MSG_DRIVE_MOTOR_OFF,
    MSG_DRIVE_DMA_ON,
    MSG_DRIVE_DMA_OFF,
    MSG_DRIVE_HEAD,
    MSG_DRIVE_HEAD_POLL,
    
    
    
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
 * This is a full description of the emulated computer model.
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
    bool realTimeClock;
    long layout;
    DriveConfiguration df0;
    DriveConfiguration df1;
    DriveConfiguration df2;
    DriveConfiguration df3;
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
    uint32_t sprpt[8];
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
    uint16_t adkcon;
}
PaulaInfo;

typedef struct
{
    int8_t selectedDrive;
    DriveState state;
    int32_t fifo[6];
    uint8_t fifoCount;
    
    uint16_t dsklen;
    uint16_t dskbytr;
    uint16_t dsksync;
    uint8_t prb;
}
DiskControllerInfo;

#endif
