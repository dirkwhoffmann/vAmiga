// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "GdbServer.h"
#include "Emulator.h"
#include "CPU.h"
#include "Memory.h"
#include "MsgQueue.h"
#include "OSDebugger.h"
#include "RetroShell.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

namespace vamiga {

void
GdbServer::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    RemoteServer::_dump(category, os);

    if (category == Category::Segments) {

        os << tab("Code segment") << hex(codeSeg()) << std::endl;
        os << tab("Data segment") << hex(dataSeg()) << std::endl;
        os << tab("BSS segment") << hex(bssSeg()) << std::endl;
    }
}

bool
GdbServer::canRun()
{
    // Don't run if no process is specified
    if (processName == "") return false;

    // Try to locate the segment list if it hasn't been located yet
    if (segList.empty()) readSegList();

    return !segList.empty();
}

Transport &
GdbServer::transport()
{
    switch (config.transport) {

        case TransportProtocol::STDIO: return stdio;
        case TransportProtocol::TCP:   return tcp;

        default:
            fatalError;
    }
}

const Transport &
GdbServer::transport() const
{
    return const_cast<GdbServer *>(this)->transport();
}

bool
GdbServer::isSupported(TransportProtocol protocol) const
{
    switch (protocol) {

        case TransportProtocol::STDIO:  return true;
        case TransportProtocol::TCP:    return true;

        default:
            return false;
    }
}

void
GdbServer::didReceive(const string &payload)
{
    // Remove LF and CR (if present)
    auto cmd = utl::rtrim(payload, "\n\r");

    if (config.verbose) {
        retroShell << "R: " << utl::makePrintable(cmd) << "\n";
    }

    latestCmd = cmd;

    try {

        process(latestCmd);

    } catch(Error &err) {

        auto msg = "GDB server error: " + string(err.what());
        logmsg(LOG_SRV, "%s\n", msg.c_str());

        // Display the error message in RetroShell
        retroShell << msg << '\n';

        // Disconnect the client
        disconnect();
    }
}

void
GdbServer::didStart()
{
    emulator.pause();
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

    if (config.verbose) {
        retroShell << "T: " << utl::makePrintable(packet) << "\n";
    }
}

bool
GdbServer::attach(const string &name)
{
    this->processName = name;
    this->segList = { };

    if (readSegList()) {

        retroShell << "Successfully attached to process '" << processName << "'\n\n";
        retroShell << "    Data segment: " << utl::hexstr <8> (dataSeg()) << "\n";
        retroShell << "    Code segment: " << utl::hexstr <8> (codeSeg()) << "\n";
        retroShell << "     BSS segment: " << utl::hexstr <8> (bssSeg()) << "\n\n";
    }

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

    return utl::hexstr <2> (chk);
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
        return utl::hexstr <8> ((u32)cpu.getD((int)(nr)));
    }
    if (nr >= 8 && nr <= 15) {
        return utl::hexstr <8> ((u32)cpu.getA((int)(nr - 8)));
    }
    if (nr == 16) {
        return utl::hexstr <8> ((u32)cpu.getSR());
    }
    if (nr == 17) {
        return utl::hexstr <8> ((u32)cpu.getPC());
    }

    return "xxxxxxxx";
}

string
GdbServer::readMemory(isize addr)
{
    auto byte = mem.spypeek8 <Accessor::CPU> ((u32)addr);
    return utl::hexstr <2> (byte);
}

void
GdbServer::breakpointReached()
{
    logmsg(LOG_GDB, "breakpointReached()\n");
    process <'?'> ("");
}

}
