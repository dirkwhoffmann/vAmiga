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

void
TerminalServer::welcome()
{
    if (config.mode != SRVMODE_TERMINAL) return;
    
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
