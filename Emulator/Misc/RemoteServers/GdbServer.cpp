// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "GdbServer.h"
#include "Amiga.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "OSDebugger.h"
#include "RetroShell.h"

namespace vamiga {

void
GdbServer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    RemoteServer::_dump(category, os);

    if (category == Category::Segments) {
        
        os << tab("Code segment") << hex(codeSeg()) << std::endl;
        os << tab("Data segment") << hex(dataSeg()) << std::endl;
        os << tab("BSS segment") << hex(bssSeg()) << std::endl;
    }
}

void
GdbServer::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_SRV_PORT,
        OPT_SRV_PROTOCOL,
        OPT_SRV_AUTORUN,
        OPT_SRV_VERBOSE
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option, SERVER_GDB));
    }
}

bool
GdbServer::shouldRun()
{
    // Don't run if no process is specified
    if (processName == "") return false;
    
    // Try to locate the segment list if it hasn't been located yet
    if (segList.empty()) readSegList();

    return !segList.empty();
}

string
GdbServer::doReceive()
{
    auto cmd = connection.recv();
    
    // Remove LF and CR (if present)
    cmd = util::rtrim(cmd, "\n\r");

    if (config.verbose) {
        retroShell << "R: " << util::makePrintable(cmd) << "\n";
    }

    latestCmd = cmd;
    return latestCmd;
}

void
GdbServer::doSend(const string &payload)
{
    connection.send(payload);
    
    if (config.verbose) {
        retroShell << "T: " << util::makePrintable(payload) << "\n";
    }
}

void
GdbServer::doProcess(const string &payload)
{
    try {
        
        process(latestCmd);
        
    } catch (VAError &err) {
        
        auto msg = "GDB server error: " + string(err.what());
        debug(SRV_DEBUG, "%s\n", msg.c_str());

        // Display the error message in RetroShell
        retroShell << msg << '\n';

        // Disconnect the client
        disconnect();
    }
}

void
GdbServer::didStart()
{
    amiga.pause();
}

void
GdbServer::didStop()
{
    detach();
}

void
GdbServer::didConnect()
{
    ackMode = true;
}

void
GdbServer::reply(const string &payload)
{
    string packet = "$";
    
    packet += payload;
    packet += "#";
    packet += computeChecksum(payload);
    
    send(packet);
}

bool
GdbServer::attach(const string &name)
{
    SUSPENDED
    
    this->processName = name;
    this->segList = { };
    
    readSegList();
    
    if (segList.empty()) {

        retroShell << "Waiting for process '" << processName << "' to launch.\n";
        return false;
    }
    return true;
}

void
GdbServer::detach()
{
    processName = "";
    segList = { };
}

bool
GdbServer::readSegList()
{
    // Quick-exit if no process is supposed to be attached
    if (processName == "") return false;
    
    // Quick-exit if the segment list is already present
    if (!segList.empty()) return true;

    // Try to find the segment list in memory
    osDebugger.read(processName, segList);
    if (segList.empty()) return false;
    
    retroShell << "Successfully attached to process '" << processName << "'\n\n";
    retroShell << "    Data segment: " << util::hexstr <8> (dataSeg()) << "\n";
    retroShell << "    Code segment: " << util::hexstr <8> (codeSeg()) << "\n";
    retroShell << "     BSS segment: " << util::hexstr <8> (bssSeg()) << "\n\n";
    return true;
}

u32
GdbServer::codeSeg() const
{
    return segList.size() > 0 ? segList[0].first : 0;
}

u32
GdbServer::dataSeg() const
{
    return segList.size() > 1 ? segList[1].first : 0;
}

u32
GdbServer::bssSeg() const
{
    return segList.size() > 2 ? segList[2].first : dataSeg();
}

string
GdbServer::computeChecksum(const string &s)
{
    u8 chk = 0;
    for(auto &c : s) U8_INC(chk, c);

    return util::hexstr <2> (chk);
}

bool
GdbServer::verifyChecksum(const string &s, const string &chk)
{
    return chk == computeChecksum(s);
}

string
GdbServer::readRegister(isize nr)
{
    if (nr >= 0 && nr <= 7) {
        return util::hexstr <8> ((u32)cpu.getD((int)(nr)));
    }
    if (nr >= 8 && nr <= 15) {
        return util::hexstr <8> ((u32)cpu.getA((int)(nr - 8)));
    }
    if (nr == 16) {
        return util::hexstr <8> ((u32)cpu.getSR());
    }
    if (nr == 17) {
        return util::hexstr <8> ((u32)cpu.getPC());
    }

    return "xxxxxxxx";
}

string
GdbServer::readMemory(isize addr)
{
    auto byte = mem.spypeek8 <ACCESSOR_CPU> ((u32)addr);
    return util::hexstr <2> (byte);
}

void
GdbServer::breakpointReached()
{
    debug(GDB_DEBUG, "breakpointReached()\n");
    process <'?'> ("");
}

}
