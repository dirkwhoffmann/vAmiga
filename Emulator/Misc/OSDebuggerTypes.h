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

namespace os {

struct Node
{
    u32     ln_Succ;                // 0
    u32     ln_Pred;                // 4
    u8      ln_Type;                // 8
    i8      ln_Pri;                 // 9
    u32     ln_Name;                // 10
};

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

typedef struct Library
{
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

typedef struct IntVector
{
    u32     iv_Data;                // 0
    u32     iv_Code;                // 4
    u32     iv_Node;                // 8
}
IntVector;

typedef struct List
{
    u32     lh_Head;                // 0
    u32     lh_Tail;                // 4
    u32     lh_TailPred;            // 8
    u8      lh_Type;                // 12
    u8      l_pad;                  // 13
}
List;

typedef struct MinList
{
   u32      mlh_Head;               // 0
   u32      mlh_Tail;               // 4
   u32      mlh_TailPred;           // 8
}
MinList;

typedef struct SoftIntList
{
    struct  List sh_List;           // 0
    u16     sh_Pad;                 // 4
}
SoftIntList;

typedef struct ExecBase
{
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
