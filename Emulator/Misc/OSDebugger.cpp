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
#include <sstream>

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
OSDebugger::searchLibrary(u32 addr, os::Library &result) const
{
    std::vector <os::Library> libraries;
    read(getExecBase().LibList.lh_Head, libraries);
    
    for (isize i = 0; i < (isize)libraries.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)libraries.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)devices.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)devices.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)resources.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)resources.size(); i++) {
        
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
    
    for (isize i = 0; i < (isize)tasks.size(); i++) {
        
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

    for (isize i = 0; i < (isize)tasks.size(); i++) {
        
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
    std::vector <os::Process> processes;
    read(processes);
    
    for (isize i = 0; i < (isize)processes.size(); i++) {
        
        if (processes[i].addr == addr || i + 1 == addr) {
            
            result = processes[i];
            return true;
        }
    }
    
    return false;
}

bool
OSDebugger::searchProcess(const string &name, os::Process &result) const
{
    std::vector <os::Process> processes;
    read(processes);

    for (isize i = 0; i < (isize)processes.size(); i++) {
        
        string nodeName;
        read(processes[i].pr_Task.tc_Node.ln_Name, nodeName);
        auto shortName = nodeName.substr(0, nodeName.find("."));
        
        if (name == nodeName || name == shortName) {
        
            result = processes[i];
            return true;
        }
    }
    
    return false;
}
