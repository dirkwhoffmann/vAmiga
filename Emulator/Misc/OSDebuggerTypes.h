// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"
#include <vector>

namespace os {

#define APTR(x) (u32)(x)
#define BPTR(x) (u32)(4*(x))

//
// Enumerations
//

typedef enum
{
    NT_UNKNOWN      = 0,
    NT_TASK         = 1,
    NT_INTERRUPT    = 2,
    NT_DEVICE       = 3,
    NT_MSGPORT      = 4,
    NT_MESSAGE      = 5,
    NT_FREEMSG      = 6,
    NT_REPLYMSG     = 7,
    NT_RESOURCE     = 8,
    NT_LIBRARY      = 9,
    NT_MEMORY       = 10,
    NT_SOFTINT      = 11,
    NT_FONT         = 12,
    NT_PROCESS      = 13,
    NT_SEMAPHORE    = 14,
    NT_SIGNALSEM    = 15,
    NT_BOOTNODE     = 16,
    NT_KICKMEM      = 17,
    NT_GRAPHICS     = 18,
    NT_DEATHMESSAGE = 19,
    
    NT_USER         = 254,
    NT_EXTENDED     = 255
}
LnType;

typedef enum
{
    TS_INVALID      = 0,
    TS_ADDED        = 1,
    TS_RUN          = 2,
    TS_READY        = 3,
    TS_WAIT         = 4,
    TS_EXCEPT       = 5,
    TS_REMOVED      = 6
}
TState;


//
// Flags (bit masks)
//

typedef enum
{
    SIGF_ABORT      = 1 << 0,
    SIGF_CHILD      = 1 << 1,
    SIGF_BLIT       = 1 << 4,
    SIGF_SINGLE     = 1 << 4, // same as SIGF_BLIT
    SIGF_INTUITION  = 1 << 5,
    SIGF_NET        = 1 << 7,
    SIGF_DOS        = 1 << 8
}
SigFlags;

typedef enum
{
    TF_PROCTIME     = 1 << 0,
    TF_ETASK        = 1 << 3,
    TF_STACKCHK     = 1 << 4,
    TF_EXCEPT       = 1 << 5,
    TF_SWITCH       = 1 << 6,
    TF_LAUNCH       = 1 << 7
}
TFlags;

typedef enum AttnFlags
{
    AFF_68010       = 1 << 0,
    AFF_68020       = 1 << 1,
    AFF_68030       = 1 << 2,
    AFF_68040       = 1 << 3,
    AFF_68881       = 1 << 4,
    AFF_68882       = 1 << 5,
    AFF_FPU40       = 1 << 6,
    AFF_PRIVATE     = 1 << 15
}
AttnFlags;

typedef enum LibFlags
{
    LIBF_SUMMING    = 1 << 0,
    LIBF_CHANGED    = 1 << 1,
    LIBF_SUMUSED    = 1 << 2,
    LIBF_DELEXP     = 1 << 3
}
LibFlags;

typedef enum PrFlags
{
    PRF_FREESEGLIST = 1 << 0,
    PRF_FREECURRDIR = 1 << 1,
    PRF_FREECLI     = 1 << 2,
    PRF_CLOSEINPUT  = 1 << 3,
    PRF_CLOSEOUTPUT = 1 << 4,
    PRF_FREEARGS    = 1 << 5

}
PrFlags;


//
// Structures
//

struct Node
{
    u32     addr;
    
    u32     ln_Succ;                // 0
    u32     ln_Pred;                // 4
    u8      ln_Type;                // 8
    i8      ln_Pri;                 // 9
    u32     ln_Name;                // 10
};

typedef struct Library
{
    u32     addr;

    struct  Node lib_Node;          // 0
    u8      lib_Flags;              // 14
    u8      lib_pad;                // 15
    u16     lib_NegSize;            // 16
    u16     lib_PosSize;            // 18
    u16     lib_Version;            // 20
    u16     lib_Revision;           // 22
    u32     lib_IdString;           // 24
    u32     lib_Sum;                // 28
    u16     lib_OpenCnt;            // 32
}
Library;

typedef struct Interrupt
{
    u32     addr;

    struct  Node is_Node;           // 0
    u32     is_Data;                // 14
    u32     is_Code;                // 18
}
Interrupt;

typedef struct IntVector
{
    u32     addr;

    u32     iv_Data;                // 0
    u32     iv_Code;                // 4
    u32     iv_Node;                // 8
}
IntVector;

typedef struct List
{
    u32     addr;

    u32     lh_Head;                // 0
    u32     lh_Tail;                // 4
    u32     lh_TailPred;            // 8
    u8      lh_Type;                // 12
    u8      lh_pad;                 // 13
}
List;

typedef struct MinList
{
    u32     addr;
    
    u32      mlh_Head;               // 0
    u32      mlh_Tail;               // 4
    u32      mlh_TailPred;           // 8
}
MinList;

typedef struct SoftIntList
{
    u32     addr;

    struct  List sh_List;           // 0
    u16     sh_Pad;                 // 4
}
SoftIntList;

typedef struct MsgPort
{
    u32     addr;

    struct  Node mp_Node;           // 0
    u8      mp_Flags;               // 14
    u8      mp_SigBit;              // 15
    u32     mp_SigTask;             // 16
    struct  List mp_MsgList;        // 20
}
MsgPort;

typedef struct CommandLineInterface
{
    u32     addr;
    
    i32     cli_Result2;            // 0
    u32     cli_SetName;            // 4
    u32     cli_CommandDir;         // 8
    i32     cli_ReturnCode;         // 12
    u32     cli_CommandName;        // 16
    i32     cli_FailLevel;          // 20
    u32     cli_Prompt;             // 24
    u32     cli_StandardInput;      // 28
    u32     cli_CurrentInput;       // 32
    u32     cli_CommandFile;        // 36
    i32     cli_Interactive;        // 40
    i32     cli_Background;         // 44
    u32     cli_CurrentOutput;      // 48
    i32     cli_DefaultStack;       // 52
    u32     cli_StandardOutput;     // 56
    u32     cli_Module;             // 60
}
CommandLineInterface;

typedef struct Task
{
    u32     addr;

    struct  Node tc_Node;           // 0
    u8      tc_Flags;               // 14
    u8      tc_State;               // 15
    i8      tc_IDNestCnt;           // 16
    i8      tc_TDNestCnt;           // 17
    u32     tc_SigAlloc;            // 18
    u32     tc_SigWait;             // 22
    u32     tc_SigRecvd;            // 26
    u32     tc_SigExcept;           // 30
    u16     tc_TrapAlloc;           // 34
    u16     tc_TrapAble;            // 36
    u32     tc_ExceptData;          // 38
    u32     tc_ExceptCode;          // 42
    u32     tc_TrapData;            // 46
    u32     tc_TrapCode;            // 50
    u32     tc_SPReg;               // 54
    u32     tc_SPLower;             // 58
    u32     tc_SPUpper;             // 62
    u32     tc_Switch;              // 66
    u32     tc_Launch;              // 70
    struct  List tc_MemEntry;       // 74
    u32     tc_UserData;            // 88
}
Task;

typedef struct Process
{
    u32     addr;

    struct  Task pr_Task;           // 0
    struct  MsgPort pr_MsgPort;     // 92
    i16     pr_Pad;                 // 126
    u32     pr_SegList;             // 128
    i32     pr_StackSize;           // 132
    u32     pr_GlobVec;             // 136
    i32     pr_TaskNum;             // 140
    u32     pr_StackBase;           // 144
    i32     pr_Result2;             // 148
    u32     pr_CurrentDir;          // 152
    u32     pr_CIS;                 // 156
    u32     pr_COS;                 // 160
    u32     pr_ConsoleTask;         // 164
    u32     pr_FileSystemTask;      // 168
    u32     pr_CLI;                 // 172
    u32     pr_ReturnAddr;          // 176
    u32     pr_PktWait;             // 180
    u32     pr_WindowPtr;           // 184

    // The following definitions are new with 2.0
    u32    pr_HomeDir;              // 188
    i32    pr_Flags;                // 192
    u32    pr_ExitCode;             // 196
    i32    pr_ExitData;             // 200
    u32    pr_Arguments;            // 204
    struct MinList pr_LocalVars;    // 208
    u32    pr_ShellPrivate;         // 220
    u32    pr_CES;                  // 224
}
Process;

typedef std::vector<std::pair<u32, u32>> SegList;

typedef struct ExecBase
{
    u32     addr;

    struct  Library LibNode;        // 0
    u16     SoftVer;                // 34
    i16     LowMemChkSum;           // 36
    u32     ChkBase;                // 38
    u32     ColdCapture;            // 42
    u32     CoolCapture;            // 46
    u32     WarmCapture;            // 50
    u32     SysStkUpper;            // 54
    u32     SysStkLower;            // 58
    u32     MaxLocMem;              // 62
    u32     DebugEntry;             // 66
    u32     DebugData;              // 70
    u32     AlertData;              // 74
    u32     MaxExtMem;              // 78
    u16     ChkSum;                 // 82
    struct  IntVector IntVects[16]; // 84
    u32     ThisTask;               // 276
    u32     IdleCount;              // 280
    u32     DispCount;              // 284
    u16     Quantum;                // 288
    u16     Elapsed;                // 290
    u16     SysFlags;               // 292
    i8      IDNestCnt;              // 294
    i8      TDNestCnt;              // 295
    u16     AttnFlags;              // 296
    u16     AttnResched;            // 298
    u32     ResModules;             // 300
    u32     TaskTrapCode;           // 304
    u32     TaskExceptCode;         // 308
    u32     TaskExitCode;           // 312
    u32     TaskSigAlloc;           // 316
    u16     TaskTrapAlloc;          // 320
    struct  List MemList;           // 322
    struct  List ResourceList;      // 336
    struct  List DeviceList;        // 350
    struct  List IntrList;          // 364
    struct  List LibList;           // 378
    struct  List PortList;          // 392
    struct  List TaskReady;         // 406
    struct  List TaskWait;          // 420
    struct  SoftIntList SoftInts[5];// 434
    i32     LastAlert[4];           // 514
    u8      VBlankFrequency;        // 530
    u8      PowerSupplyFrequency;   // 531
    struct  List SemaphoreList;     // 532
    u32     KickMemPtr;             // 546
    u32     KickTagPtr;             // 550
    u32     KickCheckSum;           // 554

    // V36 Exec additions
    u16     ex_Pad0;                // 558
    u32     ex_LaunchPoint;         // 560
    u32     ex_RamLibPrivate;       // 564
    u32     ex_EClockFrequency;     // 568
    u32     ex_CacheControl;        // 572
    u32     ex_TaskID;              // 576
    u32     ex_PuddleSize;          // 580
    u32     ex_PoolThreshold;       // 584
    struct  MinList ex_PublicPool;  // 588
    u32     ex_MMULock;             // 600
    u8      ex_Reserved[12];        // 604
}
ExecBase;

}
