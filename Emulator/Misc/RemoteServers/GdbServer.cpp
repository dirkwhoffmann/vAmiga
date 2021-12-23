// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "GdbServer.h"
#include "Amiga.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"

void
GdbServer::start(isize port)
{
    RemoteServer::start(port);
    ackMode = true;
}

string
GdbServer::_receive()
{
    latestCmd = connection.recv();
    retroShell << "R: " << latestCmd << "\n";
    return latestCmd;
}

void
GdbServer::_send(const string &payload)
{
    retroShell << "T: " << payload << "\n";
    connection.send(payload);
}

void
GdbServer::_process(const string &payload)
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
GdbServer::reply(const string &payload)
{
    string packet = "$";
    
    packet += payload;
    packet += "#";
    packet += computeChecksum(payload);
    
    send(packet);
}

string
GdbServer::computeChecksum(const string &s)
{
    uint8_t chk = 0;
    for(auto &c : s) chk += (uint8_t)c;

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
    debug(SRV_DEBUG, "Breakpoint reached\n");
    process <'?'> ("");
}
