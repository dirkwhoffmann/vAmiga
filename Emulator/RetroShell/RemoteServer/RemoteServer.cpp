// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RemoteServer.h"
#include "Amiga.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"

RemoteServer::RemoteServer(Amiga& ref) : SubComponent(ref)
{

}

RemoteServer::~RemoteServer()
{
    if (isListening()) stop();
}

void
RemoteServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Port");
        os << dec(config.port) << std::endl;
        os << tab("Verbose");
        os << bol(config.verbose) << std::endl;
    }
    
    if (category & dump::State) {
        
        os << tab("Running") << bol(listening) << std::endl;
    }
}

RemoteServerConfig
RemoteServer::getDefaultConfig()
{
    RemoteServerConfig defaults;

    defaults.port = 8080;
    defaults.verbose = true;

    return defaults;
}

void
RemoteServer::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_GDB_PORT, defaults.port);
    setConfigItem(OPT_GDB_VERBOSE, defaults.verbose);
}

i64
RemoteServer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_GDB_PORT:       return config.port;
        case OPT_GDB_VERBOSE:    return config.verbose;
            
        default:
            fatalError;
    }
}

void
RemoteServer::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_GDB_PORT:
            
            config.port = (isize)value;
            return;
                        
        case OPT_GDB_VERBOSE:
            
            config.verbose = (bool)value;
            return;

        default:
            fatalError;
    }
}

void
RemoteServer::start()
{
    debug(SRV_DEBUG, "Starting remote server\n");
        
    // Only proceed if the server is not running
    if (listening) throw VAError(ERROR_GDB_SERVER_RUNNING);
        
    // Make sure that we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();
    
    // Spawn a new thread
    serverThread = std::thread(&RemoteServer::main, this);
    
    // Send a welcome message
    send("vAmiga RemoteServer - Connection established");
}

void
RemoteServer::stop()
{
    debug(SRV_DEBUG, "Stopping remote server\n");
 
    // Only proceed if an open connection exists
    if (!listening) throw VAError(ERROR_GDB_SERVER_NOT_RUNNING);
        
    // Trigger an exception inside the server thread
    listening = false;
    connection.close();
    listener.close();

    // Wait until the server thread has terminated
    serverThread.join();

    debug(SRV_DEBUG, "stopped\n");
}

string
RemoteServer::receive()
{
    auto packet = connection.recv();

    if (config.verbose) {
        
        retroShell << packet; 
        retroShell.storage.append("");
    }
     
    try { retroShell.execUserCommand(packet); } catch (...) { }

    debug(SRV_DEBUG, "R: %s\n", packet.c_str());
    msgQueue.put(MSG_SRV_RECEIVE);
    
    return packet;
}

void
RemoteServer::send(const string &cmd)
{
    if (isListening()) {
 
        string packet = cmd; //  + "\n";
        connection.send(packet);

        debug(SRV_DEBUG, "T: %s\n", packet.c_str());
        msgQueue.put(MSG_SRV_SEND);
    }
}

RemoteServer&
RemoteServer::operator<<(char value)
{
    send(string(1, value));
    return *this;
}

RemoteServer&
RemoteServer::operator<<(const string& text)
{
    send(text);
    return *this;
}

RemoteServer&
RemoteServer::operator<<(int value)
{
    send(std::to_string(value));
    return *this;
}

RemoteServer&
RemoteServer::operator<<(long value)
{
    send(std::to_string(value));
    return *this;
}

RemoteServer&
RemoteServer::operator<<(std::stringstream &stream)
{
    string line;
    while(std::getline(stream, line)) {
        send(line + "\n");
    }
    return *this;
}

void
RemoteServer::main()
{
    debug(SRV_DEBUG, "Entering remote server thread\n");
    msgQueue.put(MSG_SRV_START);
    
    listening = true;
    
    try {
        
        // Create a port listener
        listener = PortListener((u16)config.port);
        
        // Wait for a client to connect
        connection = listener.accept();
        connected = true;
        
        // Update the server with the current text storage
        retroShell.dumpToServer();
        
        debug(SRV_DEBUG, "Entering main loop\n");

        while (1) {
            
            auto cmd = receive();
            // process(cmd);
        }
             
    } catch (VAError &err) {
        
        warn("VAError: %s\n", err.what());
        if (listening) msgQueue.put(MSG_SRV_ERROR);

    } catch (std::exception &err) {

        warn("Error: %s\n", err.what());
        if (listening) msgQueue.put(MSG_SRV_ERROR);
    }
    
    listening = false;
    connected = false;
    connection.close();
    listener.close();
    
    debug(SRV_DEBUG, "Exiting remote server thread\n");
    msgQueue.put(MSG_SRV_STOP);
}
