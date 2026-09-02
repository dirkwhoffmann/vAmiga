// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RshServer.h"
#include "Amiga.h"
#include "RetroShell.h"
#include "utl/support.h"

namespace vamiga {

void
RshServer::_initialize()
{
    // The remote server drives its own shell, independent of the GUI's one
    rshShell.console.delegates.push_back(this);
}

void
RshServer::_dump(Category category, std::ostream &os) const
{
    RemoteServer::_dump(category, os);
}

Transport &
RshServer::transport()
{
    switch (config.transport) {

        case TransportProtocol::TCP: return tcp;

        default:
            return tcp;
            // fatalError;
    }
}

const Transport &
RshServer::transport() const
{
    return const_cast<RshServer *>(this)->transport();
}

bool
RshServer::isSupported(TransportProtocol protocol) const
{
    return protocol == TransportProtocol::TCP;
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
    // Hand the client a fresh shell
    rshShell.newSession();

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

            *this << rshShell.prompt();

        } catch (...) { };
    }
}

void
RshServer::didDisconnect()
{
    // Discard the client's session
    rshShell.newSession();
}

void
RshServer::didReceive(const string &payload)
{
    // Remove LF and CR (if present)
    auto trimmed = utl::rtrim(payload, "\n\r");

    rshShell.asyncExec(InputLine {

        .type = InputLine::Source::RSH,
        .input = trimmed
    });
}

void
RshServer::send(const string &payload)
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

    transport().send(mapped);
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
    *this << rshShell.prompt();
}

void
RshServer::didExecute(const InputLine &input, std::stringstream &ss, std::exception &e)
{
    // Echo the command if it came from somewhere else
    if (!input.isRpcCommand()) { *this << input.input << '\n'; }

    *this << '\n' << ss.str() << e.what() << '\n';
    *this << rshShell.prompt();
}

}
