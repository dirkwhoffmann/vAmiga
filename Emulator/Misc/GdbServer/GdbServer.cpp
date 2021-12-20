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

GdbServer::GdbServer(Amiga& ref) : SubComponent(ref)
{
}

GdbServer::~GdbServer()
{
}

void
GdbServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Verbose");
        os << bol(config.verbose) << std::endl;
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

void
GdbServer::startSession()
{
    ackMode = true;
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
GdbServer::send(const string &cmd)
{
    string packet = "$";
                
    packet += cmd;
    packet += "#";
    packet += computeChecksum(cmd);
    
    if (config.verbose) {
        retroShell << "T: " << packet << '\n';
    }

    remoteServer.send(SRVMODE_GDB, packet);
}

void
GdbServer::execute(const string &packet)
{
    if (config.verbose) {
        retroShell << "R: " << packet << '\n';
    }

    try {
        
        process(packet);
        
    } catch (VAError &err) {
        
        auto message = "GDB server error: " + string(err.what());
        printf("%s\n", message.c_str());
        
        // Disconnect the client
        remoteServer.disconnect();
    }
}
