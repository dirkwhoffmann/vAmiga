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
#include "ServerError.h"
#include "SubComponent.h"
#include "Socket.h"
#include "Thread.h"
#include <thread>
#include "utl/wrappers.h"

namespace vamiga {

class RemoteServer : public SubComponent {

    friend class RemoteManager;

    Descriptions descriptions = {{

        .name           = "RshServer",
        .description    = "Remote Shell Server",
        .shell          = "server rsh"
    }, {
        .name           = "RpcServer",
        .description    = "RPC Remote Server",
        .shell          = "server rpc"
    }, {
        .name           = "GdbServer",
        .description    = "GDB Remote Server",
        .shell          = "server gdb"
    }, {
        .name           = "PromServer",
        .description    = "Prometheus Server",
        .shell          = "server prom"
    }, {
        .name           = "SerServer",
        .description    = "Serial Port Server",
        .shell          = "server ser"
    }};

    Options options = {

        Opt::SRV_ENABLE,
        Opt::SRV_PORT,
        Opt::SRV_PROTOCOL,
        Opt::SRV_VERBOSE
    };

public:

    // Result of the latest inspection
    utl::Backed<RemoteServerInfo> info;

protected:
    
    // Current configuration
    ServerConfig config = {};

    // The server thread
    std::thread serverThread;

    // The current server state
    SrvState state = SrvState::OFF;
    

    //
    // Initializing
    //
    
public:
    
    RemoteServer(Amiga& ref, isize id);
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

    void _dump(Category category, std::ostream &os) const override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from CoreComponent
    //
    
protected:
    
    void _powerOff() override;

    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) return;

        worker

        << config.enable
        << config.port
        << config.protocol
        << config.verbose;

    };
    virtual void operator << (SerChecker &worker) override { serialize(worker); }
    virtual void operator << (SerCounter &worker) override { serialize(worker); }
    virtual void operator << (SerResetter &worker) override { serialize(worker); }
    virtual void operator << (SerReader &worker) override { serialize(worker); }
    virtual void operator << (SerWriter &worker) override { serialize(worker); }
    
    void _didLoad() override;


    //
    // Methods from Configurable
    //

public:

    const ServerConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;


    //
    // Analyzing
    //

public:

    RemoteServerInfo cacheInfo() const;


    //
    // Examining state
    //
    
public:

    bool isOff() const { return state == SrvState::OFF; }
    bool isWaiting() const { return state == SrvState::WAITING; }
    bool isStarting() const { return state == SrvState::STARTING; }
    bool isListening() const { return state == SrvState::LISTENING; }
    bool isConnected() const { return state == SrvState::CONNECTED; }
    bool isStopping() const { return state == SrvState::STOPPING; }
    bool isErroneous() const { return state == SrvState::INVALID; }

    
    //
    // Starting and stopping the server
    //
    
private: // public:

    // Launch the remote server
    virtual void start();

    // Shuts down the remote server
    virtual void stop();

    // Disconnects the client
    virtual void disconnect() = 0;

protected:

    // Switches the internal state
    void switchState(SrvState newState);
    
private:
    
    // Indicates if the server is ready to launch
    virtual bool canRun() = 0; // { return true; }


    //
    // Running the server
    //

protected:

    // The main thread function
    virtual void main() = 0;

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
