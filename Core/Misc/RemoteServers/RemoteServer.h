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
#include "RemoteManagerTypes.h"
#include "SubComponent.h"
#include "Transport.h"
#include "utl/wrappers.h"

namespace vamiga {

/* Every remote server is the transport delegate of its own transport layers.
 * The TransportDelegate base is inherited here (rather than by the concrete
 * subclasses) so that common functionality, such as traffic recording, can
 * be implemented in a single place.
 */
class RemoteServer : public SubComponent, public TransportDelegate {

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
        Opt::SRV_TRANSPORT,
        Opt::SRV_VERBOSE
    };

public:

    // Result of the latest inspection
    utl::Backed<RemoteServerInfo> info;

protected:

    // Current configuration
    ServerConfig config = {};

    /* Indicates that this server is enabled but waiting for its launch
     * condition (canRun()) to become true. Unlike the transport's own state
     * machine (owned by Transport, see transport().getState()), this flag is
     * tracked here because it describes a precondition on *starting* the
     * transport at all, not a state the transport itself passes through.
     */
    bool waiting = false;


    //
    // Initializing
    //

public:

    RemoteServer(Amiga& ref, isize id);

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
        << config.transport
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
    // Examining the server
    //

private:

    // Returns a reference to the currently selected transport layer
    virtual Transport &transport() = 0;
    virtual const Transport &transport() const = 0;

public:

    virtual bool isSupported(TransportProtocol protocol) const = 0;

    // Returns the combined state (WAITING is layered above the transport's
    // own state -- see 'waiting')
    virtual SrvState getState() const { return waiting ? SrvState::WAITING : transport().getState(); }

    bool isOff() const { return getState() == SrvState::OFF; }
    bool isWaiting() const { return getState() == SrvState::WAITING; }
    bool isStarting() const { return getState() == SrvState::STARTING; }
    bool isListening() const { return getState() == SrvState::LISTENING; }
    bool isConnected() const { return getState() == SrvState::CONNECTED; }
    bool isStopping() const { return getState() == SrvState::STOPPING; }
    bool isErroneous() const { return getState() == SrvState::INVALID; }


    //
    // Starting and stopping the server
    //

public:

    // Indicates if the server is ready to launch
    virtual bool canRun() = 0;

    // Launch the remote server
    virtual void start();

    // Shuts down the remote server
    virtual void stop();

    // Disconnects the client (if any)
    virtual void disconnect() { transport().disconnect(); }

    // Switches into the WAITING state (called by the RemoteManager's launch daemon)
    void waitForLaunch();


    //
    // Recording traffic
    //

public:

    /* Records a transmitted or received packet in the traffic log of the
     * RemoteManager. This function is called from the transport layer for
     * every packet that passes through. Note: It may be invoked from the
     * transport's session thread.
     */
    void recordTraffic(TrafficDirection direction, const string &payload);


    //
    // Sending packets
    //

public:

    // Sends a packet
    virtual void send(const string &payload) { transport().send(payload); }
    void send(char payload);
    void send(int payload);
    void send(long payload);
    void send(std::stringstream &payload);

    // Operator overloads
    RemoteServer &operator<<(char payload) { send(payload); return *this; }
    RemoteServer &operator<<(const string &payload) { send(payload); return *this; }
    RemoteServer &operator<<(int payload) { send(payload); return *this; }
    RemoteServer &operator<<(long payload) { send(payload); return *this; }
    RemoteServer &operator<<(std::stringstream &payload) { send(payload); return *this; }


    //
    // Methods from TransportDelegate
    //

public:

    void didSwitch(SrvState from, SrvState to) override;
    void didTerminate(const string &error) override;
};

}
