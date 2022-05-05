// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RshServer.h"
#include "Amiga.h"
#include "RetroShell.h"
#include "StringUtils.h"

RshServer::RshServer(Amiga& ref) : RemoteServer(ref)
{

}

void
RshServer::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    RemoteServer::_dump(category, os);
}

void
RshServer::resetConfig()
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
        setConfigItem(option, defaults.get(option, SERVER_RSH));
    }
}

void
RshServer::didStart()
{
    if (config.verbose) {
        retroShell << "Remote server is listening at port " << config.port << "\n";
    }
}

void
RshServer::didConnect()
{
    if (config.verbose) {
        
        try {
            
            send("vAmiga RetroShell Remote Server ");
            send(std::to_string(VER_MAJOR) + ".");
            send(std::to_string(VER_MINOR) + ".");
            send(std::to_string(VER_SUBMINOR));
            send(" (" __DATE__ " " __TIME__ ")\n\n");
            send("Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de\n");
            send("Licensed under the GNU General Public License v3\n\n");
            send("Type 'help' for help.\n");
            send("\n");
            send(retroShell.getPrompt());
            
        } catch (...) { };
    }
}

string
RshServer::doReceive()
{
    string payload = connection.recv();
    
    // Remove LF and CR (if present)
    payload = util::rtrim(payload, "\n\r");

    // Ask the client to delete the input (will be replicated by RetroShell)
    connection.send("\033[A\33[2K\r");
    
    return payload;
}

void
RshServer::doSend(const string &payload)
{
    string mapped;
    
    for (auto c : payload) {
        
        switch (c) {
                
            case '\r':

                mapped += "\33[2K\r";
                break;

            case '\n':

                mapped += "\n";
                break;

            default:
                
                if (isprint(c)) mapped += c;
                break;
        }
    }
    
    connection.send(mapped);
}

void
RshServer::doProcess(const string &payload)
{
    retroShell.press(payload);
    retroShell.press('\n');    
}
