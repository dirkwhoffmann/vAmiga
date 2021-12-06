// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "OSDebugger.h"
#include "IOUtils.h"
#include "Memory.h"

void
OSDebugger::read(u32 addr, u8 *result) const
{
    *result = mem.spypeek8 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, u16 *result) const
{
    *result = mem.spypeek16 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, u32 *result) const
{
    *result = mem.spypeek32 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, os::Node *result) const
{
    read(addr +  0, &result->ln_Succ);
    read(addr +  4, &result->ln_Pred);
    read(addr +  8, &result->ln_Type);
    read(addr +  9, &result->ln_Pri);
    read(addr + 10, &result->ln_Name);
}

void
OSDebugger::read(u32 addr, os::Library *result) const
{
    read(addr + 0,  &result->lib_Node);
    read(addr + 14, &result->lib_Flags);
    read(addr + 15, &result->lib_pad);
    read(addr + 16, &result->lib_NegSize);
    read(addr + 18, &result->lib_PosSize);
    read(addr + 20, &result->lib_Version);
    read(addr + 22, &result->lib_Revision);
    read(addr + 24, &result->lib_IdString);
    read(addr + 28, &result->lib_Sum);
    read(addr + 32, &result->lib_OpenCnt);
}

void
OSDebugger::read(u32 addr, os::IntVector *result) const
{
    read(addr + 0,  &result->iv_Data);
    read(addr + 4,  &result->iv_Code);
    read(addr + 8,  &result->iv_Node);
}

void
OSDebugger::read(u32 addr, os::List *result) const
{
    read(addr + 0,  &result->lh_Head);
    read(addr + 4,  &result->lh_Tail);
    read(addr + 8,  &result->lh_TailPred);
    read(addr + 12, &result->lh_Type);
    read(addr + 13, &result->l_pad);
}

void
OSDebugger::read(u32 addr, os::MinList *result) const
{
    read(addr + 0,  &result->mlh_Head);
    read(addr + 4,  &result->mlh_Tail);
    read(addr + 8,  &result->mlh_TailPred);
}

void
OSDebugger::read(u32 addr, os::SoftIntList *result) const
{
    read(addr + 0,  &result->sh_List);
    read(addr + 4,  &result->sh_Pad);
}

void
OSDebugger::read(u32 addr, os::ExecBase *result) const
{
    read(addr + 0,   &result->LibNode);
    read(addr + 34,  &result->SoftVer);
    read(addr + 36,  &result->LowMemChkSum);
    read(addr + 38,  &result->ChkBase);
    read(addr + 42,  &result->ColdCapture);
    read(addr + 46,  &result->CoolCapture);
    read(addr + 50,  &result->WarmCapture);
    read(addr + 54,  &result->SysStkUpper);
    read(addr + 58,  &result->SysStkLower);
    read(addr + 62,  &result->MaxLocMem);
    read(addr + 66,  &result->DebugEntry);
    read(addr + 70,  &result->DebugData);
    read(addr + 74,  &result->AlertData);
    read(addr + 78,  &result->MaxExtMem);
    read(addr + 82,  &result->ChkSum);
    
    for (u32 i = 0; i < 16; i++) {
        read(addr + 84 + i*12,  &result->IntVects[i]);
    }
    
    read(addr + 276,  &result->ThisTask);
    read(addr + 280,  &result->IdleCount);
    read(addr + 284,  &result->DispCount);
    read(addr + 288,  &result->Quantum);
    read(addr + 290,  &result->Elapsed);
    read(addr + 292,  &result->SysFlags);
    read(addr + 294,  &result->IDNestCnt);
    read(addr + 295,  &result->TDNestCnt);
    read(addr + 296,  &result->AttnFlags);
    read(addr + 298,  &result->AttnResched);
    read(addr + 300,  &result->ResModules);
    read(addr + 304,  &result->TaskTrapCode);
    read(addr + 308,  &result->TaskExceptCode);
    read(addr + 312,  &result->TaskExitCode);
    read(addr + 316,  &result->TaskSigAlloc);
    read(addr + 320,  &result->TaskTrapAlloc);
    read(addr + 322,  &result->MemList);
    read(addr + 336,  &result->ResourceList);
    read(addr + 350,  &result->DeviceList);
    read(addr + 364,  &result->IntrList);
    read(addr + 378,  &result->LibList);
    read(addr + 392,  &result->PortList);
    read(addr + 406,  &result->TaskReady);
    read(addr + 420,  &result->TaskWait);
    
    for (u32 i = 0; i < 5; i++) {
        read(addr + 434 + i*16, &result->SoftInts[i]);
    }
    for (u32 i = 0; i < 5; i++) {
        read(addr + 514 + i*4, &result->LastAlert[i]);
    }

    read(addr + 530,  &result->VBlankFrequency);
    read(addr + 531,  &result->PowerSupplyFrequency);
    read(addr + 532,  &result->SemaphoreList);
    read(addr + 546,  &result->KickMemPtr);
    read(addr + 550,  &result->KickTagPtr);
    read(addr + 554,  &result->KickCheckSum);

    read(addr + 558,  &result->ex_Pad0);
    read(addr + 560,  &result->ex_LaunchPoint);
    read(addr + 564,  &result->ex_RamLibPrivate);
    read(addr + 568,  &result->ex_EClockFrequency);
    read(addr + 572,  &result->ex_CacheControl);
    read(addr + 576,  &result->ex_TaskID);
    read(addr + 580,  &result->ex_PuddleSize);
    read(addr + 584,  &result->ex_PoolThreshold);
    read(addr + 588,  &result->ex_PublicPool);
    read(addr + 600,  &result->ex_MMULock);
    
    for (u32 i = 0; i < 12; i++) {
        read(addr + 604 + i, &result->ex_Reserved[i]);
    }
}

void
OSDebugger::dumpExecBase(std::ostream& s) const
{
    using namespace util;

    // Read the ExecBase struct from Amiga memory
    u32 addr = mem.spypeek32 <ACCESSOR_CPU> (4);
    os::ExecBase execBase;
    read(addr, &execBase);
    
    s << tab("Base address");
    s << hex(addr) << std::endl;
    s << tab("VBlankFrequency");
    s << hex(execBase.VBlankFrequency) << std::endl;
    s << tab("PowerSupplyFrequency");
    s << hex(execBase.PowerSupplyFrequency) << std::endl;
}
