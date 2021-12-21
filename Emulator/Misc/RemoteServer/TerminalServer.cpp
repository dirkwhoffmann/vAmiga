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

string
TerminalServer::_receive()
{
    auto packet = connection.recv();
            
    // Ask the client to delete the input (will be replicated by RetroShell)
    send("\033[A\33[2K\r");

    msgQueue.put(MSG_SRV_RECEIVE);
    return packet;
}

void
TerminalServer::_send(const string &packet)
{
    connection.send(packet);
    
    msgQueue.put(MSG_SRV_SEND);
}

void
TerminalServer::_process(const string &packet)
{
    retroShell.press(packet);
    retroShell.press('\n');
}
