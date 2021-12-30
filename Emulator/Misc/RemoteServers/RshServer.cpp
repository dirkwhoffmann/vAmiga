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
#include "MsgQueue.h"
#include "RetroShell.h"
#include "StringUtils.h"

RshServer::RshServer(Amiga& ref) : RemoteServer(ref)
{

}

void
RshServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    RemoteServer::_dump(category, os);
}

ServerConfig
RshServer::getDefaultConfig()
{
    ServerConfig defaults;
    
    defaults.port = 8081;
    defaults.autoRun = false;
    defaults.protocol = SRVPROT_DEFAULT;
    defaults.verbose = false;

    return defaults;
}

void
RshServer::didSwitch(SrvState from, SrvState to)
{
    if (to == SRV_STATE_CONNECTED) {

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
    
    // Ask the client to delete the input (will be replicated by RetroShell)
    send("\033[A\33[2K\r");
    
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
