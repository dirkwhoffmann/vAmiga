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

using namespace os;

string
OSDebugger::toString(os::LnType value) const
{
    const char *result = "???";
    
    switch (value) {
            
        case NT_UNKNOWN:        result = "UNKNOWN"; break;
        case NT_TASK:           result = "TASK"; break;
        case NT_INTERRUPT:      result = "INTERRUPT"; break;
        case NT_DEVICE:         result = "DEVICE"; break;
        case NT_MSGPORT:        result = "MSGPORT"; break;
        case NT_MESSAGE:        result = "MESSAGE"; break;
        case NT_FREEMSG:        result = "FREEMSG"; break;
        case NT_REPLYMSG:       result = "REPLYMSG"; break;
        case NT_RESOURCE:       result = "RESOURCE"; break;
        case NT_LIBRARY:        result = "LIBRARY"; break;
        case NT_MEMORY:         result = "MEMORY"; break;
        case NT_SOFTINT:        result = "SOFTINT"; break;
        case NT_FONT:           result = "FONT"; break;
        case NT_PROCESS:        result = "PROCESS"; break;
        case NT_SEMAPHORE:      result = "SEMAPHORE"; break;
        case NT_SIGNALSEM:      result = "SIGNALSEM"; break;
        case NT_BOOTNODE:       result = "BOOTNODE"; break;
        case NT_KICKMEM:        result = "KICKMEM"; break;
        case NT_GRAPHICS:       result = "GRAPHICS"; break;
        case NT_DEATHMESSAGE:   result = "DEATHMESSAGE"; break;
        case NT_USER:           result = "USER"; break;
        case NT_EXTENDED:       result = "EXTENDED"; break;
    }
    
    return std::string(result);
}

string
OSDebugger::toString(os::TState value) const
{
    const char *result = "???";
    
    switch (value) {
            
        case TS_INVALID:        result = "INVALID"; break;
        case TS_ADDED:          result = "ADDED"; break;
        case TS_RUN:            result = "RUN"; break;
        case TS_READY:          result = "READY"; break;
        case TS_WAIT:           result = "WAIT"; break;
        case TS_EXCEPT:         result = "EXCEPT"; break;
        case TS_REMOVED:        result = "REMOVED"; break;
    }
    
    return std::string(result);
}

string
OSDebugger::toString(os::SigFlags value) const
{
    string result;
    
    if (value & SIGF_ABORT)     append(result, "ABORT");
    if (value & SIGF_CHILD)     append(result, "CHILD");
    if (value & SIGF_BLIT)      append(result, "BLIT");
    if (value & SIGF_INTUITION) append(result, "INTUITION");
    if (value & SIGF_NET)       append(result, "NET");
    if (value & SIGF_DOS)       append(result, "DOS");

    return result.empty() ? "-" : result;
}

string
OSDebugger::toString(os::TFlags value) const
{
    string result;
    
    if (value & TF_PROCTIME)    append(result, "PROCTIME");
    if (value & TF_ETASK)       append(result, "ETASK");
    if (value & TF_STACKCHK)    append(result, "STACKCHK");
    if (value & TF_EXCEPT)      append(result, "EXCEPT");
    if (value & TF_SWITCH)      append(result, "SWITCH");
    if (value & TF_LAUNCH)      append(result, "LAUNCH");
    
    return result.empty() ? "-" : result;
}

void
OSDebugger::append(string &str, const char *cstr) const
{
    if (!str.empty()) str += " | ";
    str += string(cstr);
}

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
OSDebugger::read(u32 addr, string &result) const
{
    read(addr, result, 256);
}

void
OSDebugger::read(u32 addr, string &result, isize limit) const
{
    for (isize i = 0; i < limit; i++, addr++) {

        auto c = (char)mem.spypeek8 <ACCESSOR_CPU> (addr);
        
        if (c != 0 && c != '\r' && c != '\n') {
            result += c;
        } else {
            break;
        }
    }
}

void
OSDebugger::read(u32 addr, os::Node *result) const
{
    result->addr = addr;
    
    read(addr +  0, &result->ln_Succ);
    read(addr +  4, &result->ln_Pred);
    read(addr +  8, &result->ln_Type);
    read(addr +  9, &result->ln_Pri);
    read(addr + 10, &result->ln_Name);
}

void
OSDebugger::read(u32 addr, os::Library *result) const
{
    result->addr = addr;

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
    result->addr = addr;

    read(addr + 0,  &result->iv_Data);
    read(addr + 4,  &result->iv_Code);
    read(addr + 8,  &result->iv_Node);
}

void
OSDebugger::read(u32 addr, os::List *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->lh_Head);
    read(addr + 4,  &result->lh_Tail);
    read(addr + 8,  &result->lh_TailPred);
    read(addr + 12, &result->lh_Type);
    read(addr + 13, &result->lh_pad);
}

void
OSDebugger::read(u32 addr, os::MinList *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->mlh_Head);
    read(addr + 4,  &result->mlh_Tail);
    read(addr + 8,  &result->mlh_TailPred);
}

void
OSDebugger::read(u32 addr, os::SoftIntList *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->sh_List);
    read(addr + 4,  &result->sh_Pad);
}

void
OSDebugger::read(u32 addr, os::Task *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->tc_Node);
    read(addr + 14, &result->tc_Flags);
    read(addr + 15, &result->tc_State);
    read(addr + 16, &result->tc_IDNestCnt);
    read(addr + 17, &result->tc_TDNestCnt);
    read(addr + 18, &result->tc_SigAlloc);
    read(addr + 22, &result->tc_SigWait);
    read(addr + 26, &result->tc_SigRecvd);
    read(addr + 30, &result->tc_SigExcept);
    read(addr + 34, &result->tc_TrapAlloc);
    read(addr + 36, &result->tc_TrapAble);
    read(addr + 38, &result->tc_ExceptData);
    read(addr + 42, &result->tc_ExceptCode);
    read(addr + 46, &result->tc_TrapData);
    read(addr + 50, &result->tc_TrapCode);
    read(addr + 54, &result->tc_SPReg);
    read(addr + 58, &result->tc_SPLower);
    read(addr + 62, &result->tc_SPUpper);
    read(addr + 66, &result->tc_Switch);
    read(addr + 70, &result->tc_Launch);
    read(addr + 74, &result->tc_MemEntry);
    read(addr + 88, &result->tc_UserData);
}

void
OSDebugger::read(u32 addr, os::MsgPort *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->mp_Node);
    read(addr + 14, &result->mp_Flags);
    read(addr + 15, &result->mp_SigBit);
    read(addr + 16, &result->mp_SigTask);
    read(addr + 20, &result->mp_MsgList);
}

void
OSDebugger::read(u32 addr, os::Process *result) const
{
    result->addr = addr;

    read(addr + 0,   &result->pr_Task);
    read(addr + 92,  &result->pr_MsgPort);
    read(addr + 126, &result->pr_Pad);
    read(addr + 128, &result->pr_SegList);
    read(addr + 132, &result->pr_StackSize);
    read(addr + 136, &result->pr_GlobVec);
    read(addr + 140, &result->pr_TaskNum);
    read(addr + 144, &result->pr_StackBase);
    read(addr + 148, &result->pr_Result2);
    read(addr + 152, &result->pr_CurrentDir);
    read(addr + 156, &result->pr_CIS);
    read(addr + 160, &result->pr_COS);
    read(addr + 164, &result->pr_ConsoleTask);
    read(addr + 168, &result->pr_FileSystemTask);
    read(addr + 172, &result->pr_CLI);
    read(addr + 176, &result->pr_ReturnAddr);
    read(addr + 180, &result->pr_PktWait);
    read(addr + 184, &result->pr_WindowPtr);
    read(addr + 188, &result->pr_HomeDir);
    read(addr + 192, &result->pr_Flags);
    read(addr + 196, &result->pr_ExitCode);
    read(addr + 200, &result->pr_ExitData);
    read(addr + 204, &result->pr_Arguments);
    read(addr + 208, &result->pr_LocalVars);
    read(addr + 220, &result->pr_ShellPrivate);
    read(addr + 224, &result->pr_CES);
}

void
OSDebugger::read(u32 addr, os::ExecBase *result) const
{
    result->addr = addr;

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
OSDebugger::read(os::ExecBase *result) const
{
    read(mem.spypeek32 <ACCESSOR_CPU> (4), result);
}

os::ExecBase
OSDebugger::getExecBase() const
{
    os::ExecBase result;
    read(mem.spypeek32 <ACCESSOR_CPU> (4), &result);
    return result;
}

void
OSDebugger::read(std::vector <os::Task> &result) const
{
    auto execBase = getExecBase();
        
    os::Task current;
    read(execBase.ThisTask, &current);
    
    result.push_back(current);
    read(execBase.TaskReady.lh_Head, result);
    read(execBase.TaskWait.lh_Head, result);
}

void
OSDebugger::read(std::vector <os::Process> &result) const
{
    std::vector <os::Task> tasks;
    read(tasks);
    
    for (auto &t : tasks) {
        
        if (t.tc_Node.ln_Type == NT_PROCESS) {
            
            Process process;
            read(t.addr, &process);
            result.push_back(process);
        }
    }
}

void
OSDebugger::read(u32 addr, std::vector <os::Task> &result) const
{
    for (isize i = 0; addr && i < 128; i++) {
        
        os::Task task;
        read(addr, &task);
        
        addr = task.tc_Node.ln_Succ;
        if (addr) result.push_back(task);
    }
}

/*
void
OSDebugger::read(u32 addr, std::vector <os::Process> &result) const
{
    auto isProcess = [this](u32 addr) -> bool {
        return mem.spypeek8 <ACCESSOR_CPU> (addr + 8) == NT_PROCESS;
    };
    
    for (isize i = 0; addr && i < 128; i++) {

        if (isProcess(addr)) {
            
            os::Process process;
            read(addr, &process);
        
            addr = process.pr_Task.tc_Node.ln_Succ;
            if (addr) result.push_back(process);
        }
    }
}
*/

void
OSDebugger::read(u32 addr, std::vector <os::Library> &result) const
{
    for (isize i = 0; addr && i < 128; i++) {
        
        os::Library library;
        read(addr, &library);
        
        addr = library.lib_Node.ln_Succ;
        if (addr) result.push_back(library);
    }
}

void
OSDebugger::read(u32 addr, std::vector <os::SegList> &result) const
{
    auto arraySize = mem.spypeek32 <ACCESSOR_CPU> (addr);

    for (u32 i = 1; i <= arraySize && i < 128; i++) {

        auto listAddr = BPTR(mem.spypeek32 <ACCESSOR_CPU> (addr + 4 * i));
        
        SegList list;
        read(listAddr, list);
        result.push_back(list);
    }
}

void
OSDebugger::read(u32 addr, os::SegList &result) const
{
    for (isize i = 0; addr && i < 128; i++) {
        
        auto size = mem.spypeek32 <ACCESSOR_CPU> (addr - 4);
        auto next = mem.spypeek32 <ACCESSOR_CPU> (addr);
        auto data = addr + 4;
        
        result.push_back(std::make_pair(size, data));
        addr = BPTR(next);
    }
}

void
OSDebugger::dumpExecBase(std::ostream& s) const
{
    using namespace util;

    auto execBase = getExecBase();
    
    s << tab("SoftVer");
    s << hex(execBase.SoftVer) << std::endl;
    s << tab("VBlankFrequency");
    s << hex(execBase.VBlankFrequency) << std::endl;
    s << tab("PowerSupplyFrequency");
    s << hex(execBase.PowerSupplyFrequency) << std::endl;
}

void
OSDebugger::dumpInterrupts(std::ostream& s) const
{
    using namespace util;
    
    auto execBase = getExecBase();
    
    for (isize i = 0; i < 16; i++) {
        
        s << tab("Interrupt " + std::to_string(i));
        s << hex(execBase.IntVects[i].iv_Code) << std::endl;
    }
}

void
OSDebugger::dumpLibraries(std::ostream& s) const
{
    auto execBase = getExecBase();

    std::vector <os::Library> libraries;
    read(execBase.LibList.lh_Head, libraries);

    for (auto &l: libraries) {
        
        dumpLibrary(s, l);
        s << std::endl;
    }
}

void
OSDebugger::dumpLibrary(std::ostream& s, const os::Library &lib) const
{
    using namespace util;

    string nodeName;
    read(lib.lib_Node.ln_Name, nodeName);

    string name;
    read(lib.lib_IdString, name);

    s << tab("Name");
    s << nodeName << std::endl;

    if (!name.empty()) {
        s << tab("");
        s << name << std::endl;
    }
    s << tab("Version");
    s << dec(lib.lib_Version) << "." << dec(lib.lib_Revision) << std::endl;
    s << tab("Open count");
    s << dec(lib.lib_OpenCnt) << std::endl;
}

void
OSDebugger::dumpDevices(std::ostream& s) const
{
    auto execBase = getExecBase();

    std::vector <os::Library> devices;
    read(execBase.DeviceList.lh_Head, devices);

    for (auto &l: devices) {
        
        dumpLibrary(s, l);
        s << std::endl;
    }
}

void
OSDebugger::dumpResources(std::ostream& s) const
{
    auto execBase = getExecBase();

    std::vector <os::Library> resources;
    read(execBase.ResourceList.lh_Head, resources);

    for (auto &l: resources) {
        
        dumpLibrary(s, l);
        s << std::endl;
    }
}

void
OSDebugger::dumpTasks(std::ostream& s) const
{
    std::vector <Task> tasks;
    read(tasks);
            
    for (auto &t: tasks) {
        
        dumpTask(s, t);
        s << std::endl;
    }
}

void
OSDebugger::dumpTask(std::ostream& s, const os::Task &task) const
{
    using namespace util;

    string name;
    read(task.tc_Node.ln_Name, name);
    
    auto sp = task.tc_SPReg + 70; // + kickstart_version >= 37 ? 74 : 70;
    auto pc = mem.spypeek32 <ACCESSOR_CPU> (sp);
    
    s << tab("Name");
    s << name << std::endl;
    s << tab("Flags");
    s << toString((TFlags)task.tc_Flags) << std::endl;
    s << tab("State");
    s << toString((TState)task.tc_State) << std::endl;
    s << tab("Type");
    s << toString((LnType)task.tc_Node.ln_Type) << std::endl;
    s << tab("Priority");
    s << dec(task.tc_Node.ln_Pri) << std::endl;
    s << tab("Sigwait");
    s << toString((SigFlags)task.tc_SigWait) << std::endl;
    s << tab("SP / PC");
    s << hex(sp) << ", " << hex(pc) << std::endl;

    if ((LnType)task.tc_Node.ln_Type == NT_PROCESS) {
                
        Process process;
        read(task.addr, &process);
        dumpProcess(s, process);
    }
}

void
OSDebugger::dumpProcesses(std::ostream& s) const
{
    std::vector <Process> processes;
    read(processes);
            
    for (auto &p : processes) {
        
        dumpTask(s, p.pr_Task);
        s << std::endl;
    }
}

void
OSDebugger::dumpProcess(std::ostream& s, const os::Process &process) const
{
    using namespace util;
    
    isize i = 0;
    
    // s << tab("SegList array");
    // s << hex(BPTR(process.pr_SegList)) << std::endl;
    
    std::vector<SegList> segListArray;
    if (process.pr_SegList) read(BPTR(process.pr_SegList), segListArray);
        
    for (auto &segList: segListArray) {

        string title = "Segment list " + std::to_string(++i);
                
        for (auto &it: segList) {
            
            s << tab(title); title = "";
            s << hex(it.second) << " (" << dec(it.first) << " bytes)" << std::endl;
        }
    }
}
