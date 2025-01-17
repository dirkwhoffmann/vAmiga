// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RemoteServer.h"
#include "Emulator.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"

namespace vamiga {

void
RemoteServer::shutDownServer()
{
    debug(SRV_DEBUG, "Shutting down\n");
    try { stop(); } catch(...) { }
}

void
RemoteServer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }
    
    if (category == Category::State) {
        
        os << tab("State");
        os << SrvStateEnum::key(state) << std::endl;
    }
}

void
RemoteServer::_powerOff()
{
    shutDownServer();
}

void
RemoteServer::_didLoad()
{
    // Stop the server (will be restarted by the launch daemon in auto-run mode)
    stop();
}

i64
RemoteServer::getOption(Option option) const
{
    switch (option) {
            
        case OPT_SRV_PORT: return config.port;
        case OPT_SRV_PROTOCOL: return config.protocol;
        case OPT_SRV_AUTORUN: return config.autoRun;
        case OPT_SRV_VERBOSE: return config.verbose;

        default:
            fatalError;
    }
}

void
RemoteServer::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_SRV_PORT:
        case OPT_SRV_PROTOCOL:
        case OPT_SRV_AUTORUN:
        case OPT_SRV_VERBOSE:

            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
RemoteServer::setOption(Option option, i64 value)
{
    switch (option) {

        case OPT_SRV_PORT:
            
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
            
        case OPT_SRV_PROTOCOL:
            
            config.protocol = (ServerProtocol)value;
            return;
            
        case OPT_SRV_AUTORUN:
            
            config.autoRun = (bool)value;
            return;

        case OPT_SRV_VERBOSE:
            
            config.verbose = (bool)value;
            return;

        default:
            fatalError;
    }
}

void
RemoteServer::start()
{
    if (isOff()) {

        debug(SRV_DEBUG, "Starting server...\n");
        switchState(SRV_STATE_STARTING);
        
        // Make sure we continue with a terminated server thread
        if (serverThread.joinable()) serverThread.join();
        
        // Spawn a new thread
        serverThread = std::thread(&RemoteServer::main, this);
    }
}

void
RemoteServer::stop()
{
    if (!isOff()) {

        debug(SRV_DEBUG, "Stopping server...\n");
        switchState(SRV_STATE_STOPPING);
        
        // Interrupt the server thread
        disconnect();
        
        // Wait until the server thread has terminated
        if (serverThread.joinable()) serverThread.join();
        
        switchState(SRV_STATE_OFF);
    }
}

void
RemoteServer::disconnect()
{

}

void
RemoteServer::switchState(SrvState newState)
{
    auto oldState = state;
    
    if (oldState != newState) {
        
        debug(SRV_DEBUG, "Switching state: %s -> %s\n",
              SrvStateEnum::key(state), SrvStateEnum::key(newState));
        
        // Switch state
        state = newState;
        
        // Call the delegation method
        didSwitch(oldState, newState);
        
        // Inform the GUI
        msgQueue.put(MSG_SRV_STATE, newState);
    }
}

void
RemoteServer::handleError(const char *description)
{
    switchState(SRV_STATE_ERROR);
    retroShell << "Server Error: " << string(description) << '\n';
}

void
RemoteServer::didSwitch(SrvState from, SrvState to)
{
    if (from == SRV_STATE_STARTING && to == SRV_STATE_LISTENING) {
        didStart();
    }
    if (to == SRV_STATE_OFF) {
        didStop();
    }
    if (to == SRV_STATE_CONNECTED) {
        didConnect();
    }
    if (from == SRV_STATE_CONNECTED) {
        didDisconnect();
    }
}

}
