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
RemoteServer::start(const string name)
{
    debug(GDB_DEBUG, "start\n");
        
    // Only proceed if the server is not running
    if (listening) throw VAError(ERROR_GDB_SERVER_RUNNING);
    
    debugProcess = name;
    
    // Make sure that we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();
    
    // Spawn a new thread
    serverThread = std::thread(&RemoteServer::main, this);
}

void
RemoteServer::stop()
{
    debug(GDB_DEBUG, "stop\n");
 
    // Cancel pending events
    scheduler.cancel<SLOT_GDB>();

    // Only proceed if an open connection exists
    if (!listening) throw VAError(ERROR_GDB_SERVER_NOT_RUNNING);

    listening = false;
        
    // Trigger an exception inside the server thread
    connection.close();
    listener.close();

    // Wait until the server thread has terminated
    serverThread.join();

    debug(GDB_DEBUG, "stopped\n");
}

string
RemoteServer::receive()
{
    auto packet = connection.recv();

    if (config.verbose) {
        
        retroShell << "R: " << packet << '\n';
    }

    debug(GDB_DEBUG, "R: %s\n", packet.c_str());
    msgQueue.put(MSG_GDB_RECEIVE);
    
    return packet;
}

void
RemoteServer::send(const string &cmd)
{
    string packet = "$";
                
    packet += cmd;
    packet += "#";
    packet += checksum(cmd);
    
    connection.send(packet);

    if (config.verbose) {
        
        retroShell << "T: " << packet << '\n';
    }

    debug(GDB_DEBUG, "T: %s\n", packet.c_str());
    msgQueue.put(MSG_GDB_SEND);
}

void
RemoteServer::main()
{
    debug(GDB_DEBUG, "main\n");

    msgQueue.put(MSG_GDB_START);
    
    listening = true;
    ackMode = true;
    amiga.pause();
    amiga.debugOn(1);
    
    try {
        
        // Create a port listener
        listener = PortListener((u16)config.port);
        
        // Wait for a client to connect
        connection = listener.accept();
        
        debug(GDB_DEBUG, "Entering main loop\n");

        while (1) {
            
            auto cmd = receive();
            process(cmd);
        }
             
    } catch (VAError &err) {
        
        warn("VAError: %s\n", err.what());
        if (listening) msgQueue.put(MSG_GDB_ERROR);

    } catch (std::exception &err) {

        warn("Error: %s\n", err.what());
        if (listening) msgQueue.put(MSG_GDB_ERROR);
    }
    
    listening = false;
    amiga.debugOff(1);
    connection.close();
    listener.close();
    
    msgQueue.put(MSG_GDB_STOP);
    
    debug(GDB_DEBUG, "Leaving main\n");
}

string
RemoteServer::checksum(const string &s)
{
    uint8_t chk = 0;
    for(auto &c : s) chk += (uint8_t)c;

    return util::hexstr <2> (chk);
}

std::vector<string>
RemoteServer::split(const string &s, char delimiter)
{
    std::stringstream ss(s);
    std::vector<std::string> result;
    string substr;
    
    while(std::getline(ss, substr, delimiter)) {
        result.push_back(substr);
    }
    
    return result;
}

string
RemoteServer::readRegister(isize nr)
{
    if (nr >= 0 && nr <= 7) {
        return util::hexstr <8> ((u32)cpu.getD((int)(nr)));
    }
    if (nr >= 8 && nr <= 15) {
        return util::hexstr <8> ((u32)cpu.getA((int)(nr - 8)));
    }
    if (nr == 16) {
        return util::hexstr <8> ((u32)cpu.getSR());
    }
    if (nr == 17) {
        return util::hexstr <8> ((u32)cpu.getPC());
    }

    return "xxxxxxxx";
}

string
RemoteServer::readMemory(isize addr)
{
    auto byte = mem.spypeek8 <ACCESSOR_CPU> ((u32)addr);
    return util::hexstr <2> (byte);
}

void
RemoteServer::breakpointReached()
{
    send("S01");
}

void
RemoteServer::serviceGdbEvent()
{
    printf("serviceGdbEvent (%d)\n", scheduler.id[SLOT_GDB]);
    
    auto id = scheduler.id[SLOT_GDB];
    scheduler.cancel<SLOT_GDB>();

    switch (id) {
            
        case GDB_PENDING:
                        
            printf("Trying again command %s\n", latestCmd.c_str());
            process(latestCmd);
            break;
            
        default:
            break;
    }
    
}
