// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "TerminalServer.h"
#include "MsgQueue.h"
#include "RetroShell.h"
#include "StringUtils.h"

void
TerminalServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    RemoteServer::_dump(category, os);
}

string
TerminalServer::receive()
{
    string packet;
    
    if (connected) {
        
        packet = connection.recv();
        debug(SRV_DEBUG, "R: '%s'\n", util::makePrintable(packet).c_str());

        // Ask the client to delete the input (will be replicated by RetroShell)
        send("\033[A\33[2K\r");
        
        retroShell.press(packet);
        retroShell.press('\n');
        msgQueue.put(MSG_SRV_RECEIVE);
    }
    
    return packet;
}

void
TerminalServer::send(const string &packet)
{
    if (connected) {
        
        debug(SRV_DEBUG, "T: '%s'\n", util::makePrintable(packet).c_str());
        connection.send(packet);
        
        msgQueue.put(MSG_SRV_SEND);
    }
}

void
TerminalServer::welcome()
{
    send("vAmiga Remote Server ");
    send(std::to_string(VER_MAJOR) + ".");
    send(std::to_string(VER_MINOR) + ".");
    send(std::to_string(VER_SUBMINOR));
    send(" (" __DATE__ " " __TIME__ ")\n\n");
    send("Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de\n");
    send("Licensed under the GNU General Public License v3\n\n");
    send("Type 'help' for help.\n");
    send("\n");
}
