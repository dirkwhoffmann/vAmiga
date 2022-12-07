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
#include "Thread.h"
#include <sstream>

namespace vamiga {

void
OSDebugger::dumpInfo(std::ostream& s)
{
    {   SUSPENDED

        using namespace util;
        auto execBase = getExecBase();
        auto flags = execBase.AttnFlags;
        auto pal = execBase.VBlankFrequency == 50;
        auto ntsc = execBase.VBlankFrequency == 60;

        string cpu =
        (flags & os::AFF_68010) ? "68010" :
        (flags & os::AFF_68020) ? "68020" :
        (flags & os::AFF_68030) ? "68030" :
        (flags & os::AFF_68040) ? "68040" : "68000";
        
        string fpu =
        (flags & os::AFF_68882) ? "68882" :
        (flags & os::AFF_68881) ? "68881" : "none";
        
        if ((flags & os::AFF_68040) && (flags & os::AFF_FPU40)) {
            fpu = fpu == "none" ? "FPU40" : fpu + " + FPU40";
        }
        
        s << tab("CPU");
        s << cpu << std::endl;
        s << tab("FPU");
        s << fpu << std::endl;
        s << tab("EClock");
        s << dec(execBase.ex_EClockFrequency) << "Hz" << std::endl;
        s << tab("VFrequency");
        s << dec(execBase.VBlankFrequency) << "Hz ";
        s << (pal ? "(PAL)" : ntsc ? "(NTSC)" : "(?)") << std::endl;
        s << tab("Supply");
        s << dec(execBase.PowerSupplyFrequency) << "Hz" << std::endl;
    }
}

void
OSDebugger::dumpExecBase(std::ostream& s)
{
    {   SUSPENDED

        using namespace util;
        auto execBase = getExecBase();
        
        s << tab("SoftVer");
        s << hex(execBase.SoftVer) << std::endl;
        s << tab("LowMemChkSum");
        s << hex((u16)execBase.LowMemChkSum) << std::endl;
        s << tab("ChkBase");
        s << hex(execBase.ChkBase) << std::endl;
        s << tab("ColdCapture");
        s << hex(execBase.ColdCapture) << std::endl;
        s << tab("CoolCapture");
        s << hex(execBase.CoolCapture) << std::endl;
        s << tab("WarmCapture");
        s << hex(execBase.WarmCapture) << std::endl;
        s << tab("SysStkUpper");
        s << hex(execBase.SysStkUpper) << std::endl;
        s << tab("SysStkLower");
        s << hex(execBase.SysStkLower) << std::endl;
        s << tab("MaxLocMem");
        s << hex(execBase.MaxLocMem) << std::endl;
        s << tab("DebugEntry");
        s << hex(execBase.DebugEntry) << std::endl;
        s << tab("DebugData");
        s << hex(execBase.DebugData) << std::endl;
        s << tab("AlertData");
        s << hex(execBase.AlertData) << std::endl;
        s << tab("MaxExtMem");
        s << hex(execBase.MaxExtMem) << std::endl;
        s << tab("ChkSum");
        s << hex(execBase.ChkSum) << std::endl;
        
        s << tab("IdleCount");
        s << hex(execBase.IdleCount) << std::endl;
        s << tab("DispCount");
        s << hex(execBase.DispCount) << std::endl;
        s << tab("Quantum");
        s << hex(execBase.Quantum) << std::endl;
        s << tab("Elapsed");
        s << hex(execBase.Elapsed) << std::endl;
        s << tab("SysFlags");
        s << hex(execBase.SysFlags) << std::endl;
        s << tab("IDNestCnt");
        s << dec(execBase.IDNestCnt) << std::endl;
        s << tab("TDNestCnt");
        s << dec(execBase.TDNestCnt) << std::endl;
        s << tab("AttnFlags");
        s << toString((os::AttnFlags)execBase.AttnFlags) << std::endl;
        s << tab("AttnResched");
        s << hex(execBase.AttnResched) << std::endl;
        s << tab("ResModules");
        s << hex(execBase.ResModules) << std::endl;
        s << tab("TaskTrapCode");
        s << hex(execBase.TaskTrapCode) << std::endl;
        s << tab("TaskExceptCode");
        s << hex(execBase.TaskExceptCode) << std::endl;
        s << tab("TaskExitCode");
        s << hex(execBase.TaskExitCode) << std::endl;
        s << tab("TaskSigAlloc");
        s << hex(execBase.TaskSigAlloc) << std::endl;
        s << tab("TasTrapAlloc");
        s << hex(execBase.TaskTrapAlloc) << std::endl;
        
        s << tab("VBlankFrequency");
        s << dec(execBase.VBlankFrequency) << std::endl;
        s << tab("PowerSupplyFrequency");
        s << dec(execBase.PowerSupplyFrequency) << std::endl;
        
        s << tab("KickMemPtr");
        s << hex(execBase.KickMemPtr) << std::endl;
        s << tab("KickTagPtr");
        s << hex(execBase.KickTagPtr) << std::endl;
        s << tab("KickCheckSum");
        s << hex(execBase.KickCheckSum) << std::endl;
    }
}

void
OSDebugger::dumpIntVectors(std::ostream& s) 
{
    const char *name[16] = {
        
        "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",
        "8",  "9", "10", "11", "12", "13", "14", "15"
    };
    
    {   SUSPENDED

        using namespace util;
        auto execBase = getExecBase();
        
        for (isize i = 0; i < 16; i++) {
            
            s << tab("Table entry") << string(name[i]) << std::endl;
            dumpIntVector(s, execBase.IntVects[i]);
            if (i < 15) s << std::endl;
        }
    }
}

void
OSDebugger::dumpIntVector(std::ostream& s, const os::IntVector &intVec)
{
    {   SUSPENDED

        os::Interrupt irq;
        read(intVec.iv_Node, &irq);
        
        string name;
        read(irq.is_Node.ln_Name, name);
        
        using namespace util;
        
        if (!name.empty()) {
            
            s << tab("Name");
            s << name << std::endl;
        }
        s << tab("Data");
        s << hex(intVec.iv_Data) << " / " << hex(irq.is_Data) << std::endl;
        s << tab("Code");
        s << hex(intVec.iv_Code) << " / " << hex(irq.is_Code) << std::endl;
    }
}

void
OSDebugger::dumpLibraries(std::ostream& s)
{
    {   SUSPENDED
        
        std::vector <os::Library> libraries;
        read(getExecBase().LibList.lh_Head, libraries);
        
        for (auto &library : libraries) {
            
            dumpLibrary(s, library, false);
        }
    }
}

void
OSDebugger::dumpLibrary(std::ostream& s, u32 addr)
{
    {   SUSPENDED
        
        os::Library library;
        
        if (searchLibrary(addr, library)) {
            dumpLibrary(s, library, true);
        }
    }
}

void
OSDebugger::dumpLibrary(std::ostream& s, const string &name)
{
    {   SUSPENDED
        
        os::Library library;
        
        if (searchLibrary(name, library)) {
            dumpLibrary(s, library, true);
        }
    }
}

void
OSDebugger::dumpLibrary(std::ostream& s, const os::Library &lib, bool verbose)
{
    {   SUSPENDED
        
        using namespace util;
        
        string nodeName;
        read(lib.lib_Node.ln_Name, nodeName);
        
        string name;
        read(lib.lib_IdString, name);
        
        if (verbose) {
            
            s << tab("Name");
            s << nodeName << std::endl;
            
            if (!name.empty()) {
                
                s << tab("");
                s << name << std::endl;
            }
            
            s << tab("Version");
            s << dec(lib.lib_Version) << "." << dec(lib.lib_Revision) << std::endl;
            s << tab("NegSize");
            s << dec(lib.lib_NegSize) << std::endl;
            s << tab("PosSize");
            s << dec(lib.lib_PosSize) << std::endl;
            s << tab("Flags");
            s << toString((os::LibFlags)lib.lib_Flags) << std::endl;
            s << tab("Sum");
            s << dec(lib.lib_Sum) << std::endl;
            s << tab("Open count");
            s << dec(lib.lib_OpenCnt) << std::endl;
            
        } else {
            
            std::stringstream ss;
            ss << hex(lib.addr);
            s << tab(ss.str());
            s << nodeName << " (" << dec(lib.lib_OpenCnt) << ")" << std::endl;
        }
    }
}

void
OSDebugger::dumpDevices(std::ostream& s)
{
    {   SUSPENDED
        
        std::vector <os::Library> devices;
        read(getExecBase().DeviceList.lh_Head, devices);
        
        for (auto &device : devices) {
            dumpLibrary(s, device, false);
        }
    }
}

void
OSDebugger::dumpDevice(std::ostream& s, u32 addr)
{
    {   SUSPENDED
        
        os::Library device;
        
        if (searchLibrary(addr, device)) {
            dumpLibrary(s, device, true);
        }
    }
}

void
OSDebugger::dumpDevice(std::ostream& s, const string &name)
{
    {   SUSPENDED
        
        os::Library device;
        
        if (searchDevice(name, device)) {
            dumpDevice(s, device, true);
        }
    }
}

void
OSDebugger::dumpDevice(std::ostream& s, const os::Library &lib, bool verbose)
{
    {   SUSPENDED
        
        dumpLibrary(s, lib, verbose);
    }
}

void
OSDebugger::dumpResources(std::ostream& s)
{
    {   SUSPENDED
        
        std::vector <os::Library> resources;
        read(getExecBase().DeviceList.lh_Head, resources);
        
        for (auto &resource : resources) {
            dumpLibrary(s, resource, false);
        }
    }
}

void
OSDebugger::dumpResource(std::ostream& s, u32 addr)
{
    {   SUSPENDED
        
        os::Library resource;
        
        if (searchLibrary(addr, resource)) {
            dumpLibrary(s, resource, true);
        }
    }
}

void
OSDebugger::dumpResource(std::ostream& s, const string &name)
{
    {   SUSPENDED
        
        os::Library resource;
        
        if (searchDevice(name, resource)) {
            dumpDevice(s, resource, true);
        }
    }
}

void
OSDebugger::dumpResource(std::ostream& s, const os::Library &lib, bool verbose)
{
    {   SUSPENDED
        
        dumpLibrary(s, lib, verbose);
    }
}

void
OSDebugger::dumpTasks(std::ostream& s)
{
    {   SUSPENDED
        
        std::vector <os::Task> tasks;
        read(tasks);

        for (auto &t: tasks) {
            dumpTask(s, t, false);
        }
    }
}

void
OSDebugger::dumpTask(std::ostream& s, u32 addr)
{
    {   SUSPENDED
        
        os::Task task;
        
        if (searchTask(addr, task)) {
            dumpTask(s, task, true);
        }
    }
}

void
OSDebugger::dumpTask(std::ostream& s, const string &name)
{
    {   SUSPENDED
        
        os::Task task;
        
        if (searchTask(name, task)) {
            dumpTask(s, task, true);
        }
    }
}

void
OSDebugger::dumpTask(std::ostream& s, const os::Task &task, bool verbose)
{
    {   SUSPENDED
        
        using namespace util;
        
        string nodeName;
        read(task.tc_Node.ln_Name, nodeName);
        
        if (verbose) {
            
            // auto sp = task.tc_SPReg + 70; // + kickstart_version >= 37 ? 74 : 70;
            // auto pc = mem.spypeek32 <ACCESSOR_CPU> (sp);
            
            auto stackSize = task.tc_SPUpper - task.tc_SPLower;
            
            s << tab("Name");
            s << nodeName << std::endl;
            s << tab("Type");
            s << toString((os::LnType)task.tc_Node.ln_Type) << std::endl;
            s << tab("Priority");
            s << dec(task.tc_Node.ln_Pri) << std::endl;
            s << tab("Flags");
            s << toString((os::TFlags)task.tc_Flags) << std::endl;
            s << tab("State");
            s << toString((os::TState)task.tc_State) << std::endl;
            s << tab("IDNestCnt");
            s << dec(task.tc_IDNestCnt) << std::endl;
            s << tab("TDNestCnt");
            s << dec(task.tc_TDNestCnt) << std::endl;
            s << tab("SigAlloc");
            s << hex(task.tc_SigAlloc) << std::endl;
            s << tab("SigWait");
            s << hex(task.tc_SigWait) << std::endl;
            s << tab("SigRecvd");
            s << hex(task.tc_SigRecvd) << std::endl;
            s << tab("SigExcept");
            s << hex(task.tc_SigExcept) << std::endl;
            s << tab("TrapAlloc");
            s << hex(task.tc_TrapAlloc) << std::endl;
            s << tab("TrapAble");
            s << hex(task.tc_TrapAble) << std::endl;
            s << tab("ExceptData");
            s << hex(task.tc_ExceptData) << std::endl;
            s << tab("ExceptCode");
            s << hex(task.tc_ExceptCode) << std::endl;
            s << tab("TrapData");
            s << hex(task.tc_TrapData) << std::endl;
            s << tab("TrapCode");
            s << hex(task.tc_TrapCode) << std::endl;
            s << tab("SPReg");
            s << hex(task.tc_SPReg) << std::endl;
            s << tab("Stack");
            s << hex(task.tc_SPLower) << " (" << stackSize << " bytes)" << std::endl;
            s << tab("Switch");
            s << hex(task.tc_Switch) << std::endl;
            s << tab("Launch");
            s << hex(task.tc_Launch) << std::endl;
            s << tab("UserData");
            s << hex(task.tc_UserData) << std::endl;
            
            // s << tab("SP / PC");
            // s << hex(sp) << ", " << hex(pc) << std::endl;
            
        } else {
            
            std::stringstream ss;
            ss << hex(task.addr);
            s << tab(ss.str());
            s << nodeName;
            s << " (" << toString((os::TState)task.tc_State) << ")" << std::endl;
        }
    }
}

void
OSDebugger::dumpProcess(std::ostream& s, u32 addr)
{
    {   SUSPENDED
        
        os::Process process;
        
        if (searchProcess(addr, process)) {
            dumpProcess(s, process, true);
        }
    }
}

void
OSDebugger::dumpProcess(std::ostream& s, const string &name)
{
    {   SUSPENDED
        
        os::Process process;
        
        if (searchProcess(name, process)) {
            dumpProcess(s, process, true);
        }
    }
}

void
OSDebugger::dumpProcesses(std::ostream& s)
{
    {   SUSPENDED
        
        std::vector <os::Process> processes;
        read(processes);
        
        for (auto &process : processes) {
            dumpProcess(s, process, false);
        }
    }
}

void
OSDebugger::dumpProcess(std::ostream& s, const os::Process &process, bool verbose)
{
    {   SUSPENDED
        
        using namespace util;
        
        if (verbose) {

            s << tab("StackSize");
            s << dec(process.pr_StackSize) << std::endl;
            s << tab("StackBase");
            s << hex(process.pr_StackBase) << std::endl;
            s << tab("Flags");
            s << hex((u32)process.pr_Flags) << std::endl;
            // s << toString((os::PrFlags)process.pr_Flags) << std::endl;
            s << tab("TaskNum");
            s << dec(process.pr_TaskNum) << std::endl;
            s << tab("CLI");
            s << hex(process.pr_CLI) << std::endl;
            s << tab("ConsoleTask");
            s << hex(process.pr_ConsoleTask) << std::endl;
            s << tab("ConsoleTask");
            s << hex(process.pr_ConsoleTask) << std::endl;
            
            if (process.pr_CLI) {

                os::CommandLineInterface cli;
                read(BPTR(process.pr_CLI), &cli);

                string cmdDir;
                read(BPTR(cli.cli_CommandDir) + 1, cmdDir);

                string cmdName;
                read(BPTR(cli.cli_CommandName) + 1, cmdName);

                string cmdFile;
                read(BPTR(cli.cli_CommandFile) + 1, cmdFile);

                s << tab("Command Dir");
                s << cmdDir << std::endl;
                s << tab("Command Name");
                s << cmdName << std::endl;
                s << tab("Command File");
                s << cmdFile << std::endl;
            }
            
            os::SegList segList;
            read(process, segList);
            
            string title = "Segment list" ;
            
            for (auto &it: segList) {
                
                s << tab(title); title = "";
                s << hex(it.first) << " (" << dec(it.second) << " bytes)" << std::endl;
            }
            
        } else {
            
            dumpTask(s, process.pr_Task, verbose);
        }
    }
}

}
