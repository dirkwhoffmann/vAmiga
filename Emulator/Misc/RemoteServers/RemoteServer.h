// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServerTypes.h"
#include "SubComponent.h"
#include "Socket.h"
#include "Thread.h"
#include <thread>

namespace vamiga {

class RemoteServer : public SubComponent {

    friend class RemoteManager;

    Descriptions descriptions = {{

        .name           = "SerServer",
        .description    = "Serial Port Server",
        .shell          = "serial"
    }, {
        .name           = "RshServer",
        .description    = "Remote Shell Server",
        .shell          = "rshell"
    }, {
        .name           = "PromServer",
        .description    = "Prometheus Server",
        .shell          = "prom"
    }, {
        .name           = "GdbServer",
        .description    = "GDB Remote Server",
        .shell          = "gdb"
    }};

    ConfigOptions options = {

        OPT_SRV_PORT,
        OPT_SRV_PROTOCOL,
        OPT_SRV_AUTORUN,
        OPT_SRV_VERBOSE
    };

protected:
    
    // Current configuration
    ServerConfig config = {};

    // The server thread
    std::thread serverThread;

    // The current server state
    SrvState state = SRV_STATE_OFF;
    

    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
    ~RemoteServer() { shutDownServer(); }
    void shutDownServer();
    
    RemoteServer& operator= (const RemoteServer& other) {

        CLONE(config)
        
        return *this;
    }


    //
    // Methods from CoreObject
    //
    
protected:

    void _dump(Category category, std::ostream& os) const override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from CoreComponent
    //
    
private:
    
    void _powerOff() override;

    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) return;

        worker

        << config.port
        << config.protocol
        << config.autoRun
        << config.verbose;

    } SERIALIZERS(serialize);

    void _didLoad() override;


    //
    // Methods from Configurable
    //

public:

    const ServerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;


    //
    // Examining state
    //
    
public:

    bool isOff() const { return state == SRV_STATE_OFF; }
    bool isStarting() const { return state == SRV_STATE_STARTING; }
    bool isListening() const { return state == SRV_STATE_LISTENING; }
    bool isConnected() const { return state == SRV_STATE_CONNECTED; }
    bool isStopping() const { return state == SRV_STATE_STOPPING; }
    bool isErroneous() const { return state == SRV_STATE_ERROR; }

    
    //
    // Starting and stopping the server
    //
    
public:

    // Launch the remote server
    virtual void start() throws;

    // Shuts down the remote server
    virtual void stop() throws;

    // Disconnects the client
    virtual void disconnect() throws = 0;

protected:

    // Switches the internal state
    void switchState(SrvState newState);
    
private:
    
    // Used by the launch daemon to determine if actions should be taken
    virtual bool shouldRun() { return true; }


    //
    // Running the server
    //

protected:

    // The main thread function
    virtual void main() throws = 0;

    // Reports an error to the GUI
    void handleError(const char *description);


    //
    // Delegation methods
    //

    void didSwitch(SrvState from, SrvState to);
    virtual void didStart() { };
    virtual void didStop() { };
    virtual void didConnect() { };
    virtual void didDisconnect() { };
};

}
