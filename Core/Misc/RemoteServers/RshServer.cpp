// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RshServer.h"
#include "Amiga.h"
#include "RetroShell.h"
#include "utl/support.h"

namespace vamiga {

void
RshServer::_initialize()
{
    retroShell.commander.delegates.push_back(this);
    retroShell.debugger.delegates.push_back(this);
    retroShell.navigator.delegates.push_back(this);
}

void
RshServer::_dump(Category category, std::ostream &os) const
{    
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

            *this << "vAmiga RetroShell Remote Server ";
            *this << Amiga::build() << '\n';
            *this << '\n';

            *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
            *this << "https://github.com/dirkwhoffmann/vamiga" << '\n';
            *this << '\n';

            *this << "Type 'help' for help.\n";
            *this << '\n';

            *this << retroShell.prompt();

        } catch (...) { };
    }
}

string
RshServer::doReceive()
{
    string payload = connection.recv();
    
    // Remove LF and CR (if present)
    payload = utl::rtrim(payload, "\n\r");

    // Ask the client to delete the input (will be replicated by RetroShell)
    // connection.send("\033[A\33[2K\r");

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
    retroShell.asyncExec(InputLine {

        .type = InputLine::Source::RSH,
        .input = payload
    });
}

void
RshServer::didActivate()
{

}

void
RshServer::didDeactivate()
{

}

void
RshServer::willExecute(const InputLine &input)
{
    // Echo the command if it came from somewhere else
    if (!input.isRshCommand()) { *this << input.input << '\n'; }
}

void
RshServer::didExecute(const InputLine &input, std::stringstream &ss)
{
    *this << '\n' << ss.str() << '\n';
    *this << retroShell.prompt();
}

void
RshServer::didExecute(const InputLine &input, std::stringstream &ss, std::exception &e)
{
    // Echo the command if it came from somewhere else
    if (!input.isRpcCommand()) { *this << input.input << '\n'; }

    *this << '\n' << ss.str() << e.what() << '\n';
    *this << retroShell.prompt();
}

}
