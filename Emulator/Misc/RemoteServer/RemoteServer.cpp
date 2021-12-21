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
        
        os << tab("Mode");
        os << ServerModeEnum::key(config.mode) << std::endl;
    }
    
    if (category & dump::State) {
        
        os << tab("Listening") << bol(listening) << std::endl;
        os << tab("Connected") << bol(connected) << std::endl;
        os << tab("Port") << dec(port) << std::endl;
    }
}

RemoteServerConfig
RemoteServer::getDefaultConfig()
{
    RemoteServerConfig defaults;

    defaults.mode = SRVMODE_TERMINAL;
    // defaults.port = 8080;

    return defaults;
}

void
RemoteServer::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_SRV_MODE, defaults.mode);
}

i64
RemoteServer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_SRV_MODE:      return config.mode;
            
        default:
            fatalError;
    }
}

void
RemoteServer::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_SRV_MODE:
            
            if (!ServerModeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, ServerModeEnum::keyList());
            }
            config.mode = (ServerMode)value;
            return;
                        
        default:
            fatalError;
    }
}

void
RemoteServer::start(isize port)
{
    debug(SRV_DEBUG, "Starting remote server at port %ld...\n", port);

    // Only proceed if the server is not running
    if (listening) throw VAError(ERROR_SERVER_RUNNING);

    // Make sure that we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();
    
    // Spawn a new thread
    this->port = port;
    listening = true;
    serverThread = std::thread(&RemoteServer::main, this);
}

void
RemoteServer::stop()
{
    debug(SRV_DEBUG, "Stopping remote server...\n");
 
    // Only proceed if an open connection exists
    if (!listening) throw VAError(ERROR_SERVER_NOT_RUNNING);
        
    // Interrupt the server thread
    listening = false;
    disconnect();

    // Wait until the server thread has terminated
    serverThread.join();
}

void
RemoteServer::disconnect()
{
    debug(SRV_DEBUG, "Disconnecting client...\n");
    
    // Trigger an exception inside the server thread
    connection.close();
    listener.close();
}

string
RemoteServer::receive()
{
    string packet;
    
    if (isConnected()) {
        
        packet = _receive();
        debug(SRV_DEBUG, "R: %s\n", util::makePrintable(packet).c_str());
    }
    
    return packet;
}

void
RemoteServer::send(ServerMode mode, const string &packet)
{
    if (isConnected()) {
        
        _send(packet);
        debug(SRV_DEBUG, "T: '%s'\n", util::makePrintable(packet).c_str());
    }
}

void
RemoteServer::send(ServerMode mode, char payload)
{
    if (!isConnected() || mode != config.mode) return;
    
    switch (payload) {
            
        case '\n':
            
            send("\n");
            break;
            
        case '\r':
            
            send("\33[2K\r");
            break;
            
        default:
            
            if (isprint(payload)) send(string(1, payload));
            break;
    }
}

void
RemoteServer::send(ServerMode mode, int payload)
{
    send(std::to_string(payload));
}

void
RemoteServer::send(ServerMode mode, long payload)
{
    send(std::to_string(payload));
}


void
RemoteServer::send(ServerMode mode, std::stringstream &payload)
{
    string line;
    while(std::getline(payload, line)) {
        send(line + "\n");
    }
}

void
RemoteServer::main()
{
    debug(SRV_DEBUG, "Remote server started\n");
    msgQueue.put(MSG_SRV_START);
    
    while (listening) {
        
        try {
            
            // Create a port listener
            listener = PortListener((u16)port);
            
            // Wait for a client
            connection = listener.accept();
            connected = true;
            debug(SRV_DEBUG, "Connection established\n");
            msgQueue.put(MSG_SRV_CONNECT);

            // Print the startup message and the input prompt
            welcome();
            send(SRVMODE_TERMINAL, retroShell.prompt);
            
            // Receive and process packages
            mainLoop();
            
        } catch (VAError &err) {
            
            warn("VAError: %s\n", err.what());
            if (listening) msgQueue.put(MSG_SRV_ERROR);
            
        } catch (std::exception &err) {
            
            warn("Error: %s\n", err.what());
            if (listening) msgQueue.put(MSG_SRV_ERROR);
        }
        
        connected = false;
        connection.close();
        listener.close();
        
        debug(SRV_DEBUG, "Client disconnected\n");
        msgQueue.put(MSG_SRV_DISCONNECT);
    }
    
    debug(SRV_DEBUG, "Remote server stopped\n");
    msgQueue.put(MSG_SRV_STOP);
}

void
RemoteServer::mainLoop()
{
    while (1) {
        
        auto packet = receive();
        
        switch (config.mode) {
                
            case SRVMODE_TERMINAL:
                
                retroShell.press(packet);
                retroShell.press('\n');
                break;
                
            case SRVMODE_GDB:
                
                gdbServer.execute(packet);
                break;
                
            default:
                fatalError;
        }
    }
}
