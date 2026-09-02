// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RemoteServer.h"
#include "RemoteManager.h"
#include "Emulator.h"
#include "MsgQueue.h"
#include "RetroShell.h"
#include "utl/io.h"

namespace vamiga {

RemoteServer::RemoteServer(Amiga& ref, isize id) : SubComponent(ref, id)
{
    info.bind([this] { return cacheInfo(); } );
}

void
RemoteServer::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::Config) {

        dumpConfig(os);
    }

    if (category == Category::State) {

        os << tab("State");
        os << SrvStateEnum::key(getState()) << std::endl;
    }
}

void
RemoteServer::_powerOff()
{
    try { stop(); } catch(...) { }
}

void
RemoteServer::_didLoad()
{
    // Stop the server (will be restarted by the launch daemon in auto-run mode)
    stop();
}

i64
RemoteServer::getOption(Opt option) const
{
    switch (option) {

        case Opt::SRV_ENABLE:      return config.enable;
        case Opt::SRV_PORT:        return config.port;
        case Opt::SRV_TRANSPORT:   return (i64)config.transport;
        case Opt::SRV_VERBOSE:     return config.verbose;

        default:
            fatalError;
    }
}

void
RemoteServer::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::SRV_ENABLE:
        case Opt::SRV_PORT:
        case Opt::SRV_VERBOSE:

            return;

        case Opt::SRV_TRANSPORT:

            if (!isSupported(TransportProtocol(value))) {

                auto name = string(TransportProtocolEnum::key(TransportProtocol(value)));
                throw CoreError(CoreError::OPT_UNSUPPORTED, "Unsupported transport: " + name);
            }
            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
RemoteServer::setOption(Opt option, i64 value)
{
    checkOption(option, value);

    switch (option) {

        case Opt::SRV_ENABLE:

            config.enable = (bool)value;
            return;

        case Opt::SRV_PORT:

            if (config.port != (u16)value) {

                if (isOff()) {

                    config.port = (u16)value;

                } else {

                    stop();
                    config.port = (u16)value;
                    start();
                }
            }
            return;

        case Opt::SRV_TRANSPORT:

            if (config.transport != (TransportProtocol)value) {

                if (isOff()) {

                    config.transport = (TransportProtocol)value;

                } else {

                    stop();
                    config.transport = (TransportProtocol)value;
                    start();
                }
            }
            return;

        case Opt::SRV_VERBOSE:

            config.verbose = (bool)value;
            return;

        default:
            fatalError;
    }
}

RemoteServerInfo
RemoteServer::cacheInfo() const
{
    RemoteServerInfo info;

    info.state = getState();

    return info;
}

void
RemoteServer::start()
{
    if (!(isOff() || isWaiting())) return;

    waiting = false;
    transport().start(config.port);
}

void
RemoteServer::stop()
{
    if (isOff() || isStopping()) return;

    waiting = false;
    transport().stop();
}

void
RemoteServer::waitForLaunch()
{
    if (isOff()) waiting = true;
}

void
RemoteServer::recordTraffic(TrafficDirection direction, const string &payload)
{
    remoteManager.recordTraffic(ServerType(objid), direction, payload);
}

void
RemoteServer::didSwitch(SrvState from, SrvState to)
{
    if (from != to) msgQueue.put(Msg::SRV_STATE, (i64)to);
}

void
RemoteServer::didTerminate(const string &error)
{
    transport().switchState(SrvState::INVALID);
    retroShell << "Server Error: " << error << '\n';
}

void
RemoteServer::send(char payload)
{
    send(string(1, payload));
}

void
RemoteServer::send(int payload)
{
    send(std::to_string(payload));
}

void
RemoteServer::send(long payload)
{
    send(std::to_string(payload));
}

void
RemoteServer::send(std::stringstream &payload)
{
    string line;
    while(std::getline(payload, line)) {
        send(line + "\n");
    }
}

}
