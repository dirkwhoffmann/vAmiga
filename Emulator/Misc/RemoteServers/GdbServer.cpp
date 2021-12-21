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
GdbServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Verbose");
        os << bol(config.verbose) << std::endl;
    }
}

void
GdbServer::start(isize port)
{
    RemoteServer::start(port);
    ackMode = true;
}

string
GdbServer::receive()
{
    if (connected) {

        latestCmd = connection.recv();
        debug(SRV_DEBUG, "R: '%s'\n", util::makePrintable(latestCmd).c_str());
        
        if (config.verbose) {
            retroShell << "R: " << latestCmd << '\n';
        }
        
        execute(latestCmd);
        
        msgQueue.put(MSG_SRV_RECEIVE);
        return latestCmd;
    }
    
    return "";
}

void
GdbServer::send(const string &payload)
{
    if (connected) {
        
        if (config.verbose) {
            retroShell << "T: " << payload << '\n';
        }
        
        debug(SRV_DEBUG, "T: '%s'\n", util::makePrintable(payload).c_str());
        connection.send(payload);
    }
}

void
GdbServer::sendPacket(const string &payload)
{
    string packet = "$";
    
    packet += payload;
    packet += "#";
    packet += computeChecksum(payload);
    
    send(packet);
}

void
GdbServer::execute(const string &packet)
{
    try {
        
        process(packet);
        
    } catch (VAError &err) {
        
        auto message = "GDB server error: " + string(err.what());
        printf("%s\n", message.c_str());
        
        // Disconnect the client
        disconnect();
    }
}


GdbServerConfig
GdbServer::getDefaultConfig()
{
    GdbServerConfig defaults;

    defaults.verbose = true;

    return defaults;
}

void
GdbServer::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_GDB_VERBOSE, defaults.verbose);
}

i64
GdbServer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_GDB_VERBOSE:    return config.verbose;
            
        default:
            fatalError;
    }
}

void
GdbServer::setConfigItem(Option option, i64 value)
{
    switch (option) {
                        
        case OPT_GDB_VERBOSE:
            
            config.verbose = (bool)value;
            return;

        default:
            fatalError;
    }
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
