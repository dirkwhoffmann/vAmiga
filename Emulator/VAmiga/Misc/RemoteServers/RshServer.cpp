// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "RshServer.h"
#include "Amiga.h"
#include "RetroShell.h"
#include "StringUtils.h"

namespace vamiga {

void
RshServer::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    RemoteServer::_dump(category, os);
}

void
RshServer::didStart()
{
    if (config.verbose) {

        *this << "Remote server is listening at port " << config.port << "\n";
    }
}

void
RshServer::didConnect()
{
    if (config.verbose) {
        
        try {

            retroShell.asyncExec("welcome");

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

}
