// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "OSDebugger.h"
#include "IOUtils.h"
#include "Memory.h"
#include <sstream>

namespace vamiga {

using namespace os;

string
OSDebugger::dosTypeStr(u32 type)
{
    char typeStr[] = {

        char(BYTE3(type)),
        char(BYTE2(type)),
        char(BYTE1(type)),
        char(BYTE0(type) + '0'),
        0
    };
    
    return string(typeStr);
}

string
OSDebugger::dosVersionStr(u32 version)
{
    auto major = std::to_string(HI_WORD(version));
    auto minor = std::to_string(LO_WORD(version));

    return major + "." + minor;
}

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

string
OSDebugger::toString(os::AttnFlags value) const
{
    string result;
    
    if (value & AFF_68010)      append(result, "68010");
    if (value & AFF_68020)      append(result, "68020");
    if (value & AFF_68030)      append(result, "68030");
    if (value & AFF_68040)      append(result, "68040");
    if (value & AFF_68881)      append(result, "68881");
    if (value & AFF_68882)      append(result, "68882");
    if (value & AFF_FPU40)      append(result, "FPU40");
    if (value & AFF_PRIVATE)    append(result, "PRIVATE");

    return result.empty() ? "-" : result;
}

string
OSDebugger::toString(os::LibFlags value) const
{
    string result;
    
    if (value & LIBF_SUMMING)   append(result, "SUMMING");
    if (value & LIBF_CHANGED)   append(result, "CHANGED");
    if (value & LIBF_SUMUSED)   append(result, "SUMUSED");
    if (value & LIBF_DELEXP)    append(result, "DELEXP");

    return result.empty() ? "-" : result;
}

string
OSDebugger::toString(os::PrFlags value) const
{
    string result;
    
    if (value & PRF_FREESEGLIST)    append(result, "FREESEGLIST");
    if (value & PRF_FREECURRDIR)    append(result, "FREECURRDIR");
    if (value & PRF_FREECLI)        append(result, "FREECLI");
    if (value & PRF_CLOSEINPUT)     append(result, "CLOSEINPUT");
    if (value & PRF_CLOSEOUTPUT)    append(result, "CLOSEOUTPUT");
    if (value & PRF_FREEARGS)       append(result, "FREEARGS");

    return result.empty() ? "-" : result;
}

void
OSDebugger::append(string &str, const char *cstr) const
{
    if (!str.empty()) str += " | ";
    str += string(cstr);
}

bool
OSDebugger::isRamPtr(u32 addr) const
{
    if (!mem.inRam(addr)) {
        warn("Pointer outside RAM: %x\n", addr);
    }

    return addr && mem.inRam(addr);
}

bool
OSDebugger::isRamOrRomPtr(u32 addr) const
{
    if (!mem.inRam(addr) && !mem.inRom(addr)) {
        warn("Pointer outside RAM and ROM: %x\n", addr);
    }
    
    return addr && (mem.inRam(addr) || mem.inRom(addr));
}

bool
OSDebugger::isValidPtr(u32 addr) const
{
    if (!IS_EVEN(addr)) {
        warn("Odd pointer: %x\n", addr);
    }
    if (!mem.inRam(addr) && !mem.inRom(addr)) {
        warn("Pointer outside RAM and ROM: %x\n", addr);
    }

    return addr && IS_EVEN(addr) && (mem.inRam(addr) || mem.inRom(addr));
}

bool
OSDebugger::searchLibrary(u32 addr, os::Library &result) const
{
    std::vector <os::Library> libraries;
    read(getExecBase().LibList.lh_Head, libraries);
    
    for (usize i = 0; i < libraries.size(); i++) {
        
        if (libraries[i].addr == addr || i + 1 == addr) {
            
            result = libraries[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchLibrary(const string &name, os::Library &result) const
{
    std::vector <os::Library> libraries;
    read(getExecBase().LibList.lh_Head, libraries);

    for (usize i = 0; i < libraries.size(); i++) {
        
        string nodeName;
        read(libraries[i].lib_Node.ln_Name, nodeName);
                
        if (name == nodeName || (name + ".library") == nodeName) {
        
            result = libraries[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchDevice(u32 addr, os::Library &result) const
{
    std::vector <os::Library> devices;
    read(getExecBase().DeviceList.lh_Head, devices);
    
    for (usize i = 0; i < devices.size(); i++) {
        
        if (devices[i].addr == addr || i + 1 == addr) {
            
            result = devices[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchDevice(const string &name, os::Library &result) const
{
    std::vector <os::Library> devices;
    read(getExecBase().DeviceList.lh_Head, devices);
    
    for (usize i = 0; i < devices.size(); i++) {
        
        string nodeName;
        read(devices[i].lib_Node.ln_Name, nodeName);
        
        if (name == nodeName || (name + ".device") == nodeName) {
        
            result = devices[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchResource(u32 addr, os::Library &result) const
{
    std::vector <os::Library> resources;
    read(getExecBase().ResourceList.lh_Head, resources);
    
    for (usize i = 0; i < resources.size(); i++) {
        
        if (resources[i].addr == addr || i + 1 == addr) {
            
            result = resources[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchResource(const string &name, os::Library &result) const
{
    std::vector <os::Library> resources;
    read(getExecBase().ResourceList.lh_Head, resources);
    
    for (usize i = 0; i < resources.size(); i++) {
        
        string nodeName;
        read(resources[i].lib_Node.ln_Name, nodeName);
        
        if (name == nodeName || (name + ".resources") == nodeName) {
        
            result = resources[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchTask(u32 addr, os::Task &result) const
{
    std::vector <os::Task> tasks;
    read(tasks);
    
    for (usize i = 0; i < tasks.size(); i++) {
        
        if (tasks[i].addr == addr || i + 1 == addr) {
            
            result = tasks[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchTask(const string &name, os::Task &result) const
{
    std::vector <os::Task> tasks;
    read(tasks);
    
    for (usize i = 0; i < tasks.size(); i++) {
        
        string nodeName;
        read(tasks[i].tc_Node.ln_Name, nodeName);
        auto shortName = nodeName.substr(0, nodeName.find("."));
        
        if (name == nodeName || name == shortName) {
            
            result = tasks[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchProcess(u32 addr, os::Process &result) const
{
    try {
        std::vector <os::Process> processes;
        read(processes);
        
        for (usize i = 0; i < processes.size(); i++) {
            
            if (processes[i].addr == addr || i + 1 == addr) {
                
                result = processes[i];
                return true;
            }
        }
    } catch (...) { }
    
    return false;
}

bool
OSDebugger::searchProcess(const string &name, os::Process &result) const
{
    try {
        std::vector <os::Process> processes;
        read(processes);
        
        for (usize i = 0; i < processes.size(); i++) {
            
            string nodeName;
            read(processes[i].pr_Task.tc_Node.ln_Name, nodeName);
            if (!nodeName.empty() && name == nodeName) {
                result = processes[i];
                return true;
            }
            
            string shortName = nodeName.substr(0, nodeName.find("."));
            if (!shortName.empty() && name == shortName) {
                result = processes[i];
                return true;
            }
            
            if (processes[i].pr_CLI) {
                
                os::CommandLineInterface cli;
                read(BPTR(processes[i].pr_CLI), &cli);
                
                string cmdName;
                read(BPTR(cli.cli_CommandName) + 1, cmdName);
                if (!cmdName.empty() && name == cmdName) {

                    result = processes[i];
                    return true;
                }
            }
        }
    } catch (...) { }
    
    return false;
}

void
OSDebugger::checkExecBase(const os::ExecBase &execBase) const
{
    // Check if the struct resides at an even location in RAM
    if (!(IS_EVEN(execBase.addr) && mem.inRam(execBase.addr))) {
        throw CoreError(Fault::OSDB, "ExecBase: Invalid address");
    }

    // Check if ChkBase is the bitwise complement of SysBase
    if (!(execBase.ChkBase == ~execBase.addr)) {
        throw CoreError(Fault::OSDB, "ExecBase: Invalid ChkSum");
    }
    
    // Check if words in the range [0x22 ; 0x52] sum up to 0xFFFF
    u16 checksum = 0;
    for (u32 offset = 0x22; offset <= 0x52; offset += 2) {
        checksum += mem.spypeek16 <Accessor::CPU> (execBase.addr + offset);
    }
    if (!(checksum == 0xFFFF)) {
        throw CoreError(Fault::OSDB, "ExecBase: Checksum mismatch");
    }
    
    // Check if MaxLocMem complies to the bank map
    if (execBase.MaxLocMem & 0xFF000000) {
        throw CoreError(Fault::OSDB, "ExecBase: MaxLocMem is too large");
    }
    if (execBase.MaxLocMem & 0x3FFFF) {
        throw CoreError(Fault::OSDB, "ExecBase: MaxLocMem is not aligned");
    }
    if (auto bank = execBase.MaxLocMem >> 16) {
        
        auto src1 =mem.cpuMemSrc[bank - 1];
        auto src2 =mem.cpuMemSrc[bank];
        
        if (!(src1 == MemSrc::CHIP && src2 != MemSrc::CHIP)) {
            throw CoreError(Fault::OSDB, "ExecBase: MaxLocMem doesn't match bank map");
        }
    }

    // Check if MaxExtMem complies to the bank map
    if (execBase.MaxExtMem & 0xFF000000) {
        throw CoreError(Fault::OSDB, "ExecBase: MaxExtMem is too large");
    }
    if (execBase.MaxExtMem & 0x3FFFF) {
        throw CoreError(Fault::OSDB, "ExecBase: MaxExtMem is not aligned");
    }
    if (auto bank = execBase.MaxExtMem >> 16) {
        
        auto src1 =mem.cpuMemSrc[bank - 1];
        auto src2 =mem.cpuMemSrc[bank];
        
        if (!(src1 == MemSrc::SLOW && src2 != MemSrc::SLOW)) {
            throw CoreError(Fault::OSDB, "ExecBase: MaxExtMem doesn't match bank map");
        }
    }
}

}
