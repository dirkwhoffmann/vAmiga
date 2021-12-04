// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "GdbServer.h"
#include "IOUtils.h"
#include "MemUtils.h"

void
GdbServer::process(string packet)
{
    // Check if the previous package has been rejected
    if (packet[0] == '-') throw VAError(ERROR_GDB_NO_ACK);

    // Strip off the acknowledgment symbol if present
    if (packet[0] == '+') packet.erase(0,1);
        
    if (auto len = packet.length()) {
        
        // Check for Ctrl+C
        if (packet[0] == 0x03) {
            printf("Ctrl+C\n");
            return;
        }
        
        // Check for '$x[...]#xx'
        if (packet[0] == '$' && len >= 5 && packet[len - 3] == '#') {
                                    
            auto cmd = packet[1];
            auto arg = packet.substr(2, len - 5);
            auto chk = packet.substr(len - 2, 2);

            /*
            cout << "cmd = " << cmd << std::endl;
            cout << "arg = " << arg << std::endl;
            cout << "chk = " << chk << std::endl;
            */
            
            if (chk == checksum(packet.substr(1, len - 4))) {

                if (ackMode) connection.send("+");
                process(cmd, arg);
            
            } else {

                if (ackMode) connection.send("-");
                throw VAError(ERROR_GDB_INVALID_CHECKSUM);
            }
                            
            return;
        }
        
        throw VAError(ERROR_GDB_INVALID_FORMAT);
    }
}

template <> void
GdbServer::process <'v'> (string arg)
{
    if (arg == "MustReplyEmpty") {
        
        send("");
    }

    /*
    if (arg == "Cont?") {
        
        send("vCont;c;C;s;S;t;r");
    }
    
    if (arg.rfind("Cont;c", 0) == 0) {
        
        std::cout << "Continuing..." << std::endl;
        
        t = std::thread(&GdbServer::emulateStop, this);
        connection.send("OK");
    }
    */
}

template <> void
GdbServer::process <'q'> (string cmd)
{
    auto command = cmd.substr(0, cmd.find(":"));
        
    if (command == "Supported") {

        // send("qSupported:+;multiprocess+;vContSupported+;QNonStop+");
        send("qSupported:+;multiprocess+;vContSupported+");
        return;
    }
    
    if (cmd == "TStatus") {
        
        send("T0");
        return;
    }

    if (cmd == "TfV") {
        
        send("");
        return;
    }

    if (cmd == "fThreadInfo") {
        
        send("m01,02");
        return;
    }

    if (cmd == "sThreadInfo") {
        
        send("");
        return;
    }

    if (command == "Attached") {
        
        send("0");
        return;
    }
    
    if (command == "C") {
        
        send("");
        return;
    }
    
    /*
    if (command == "Xfer") {
    
        send("OK");
        return;
    }
    
    if (cmd == "sThreadInfo") {
        
        send("l");
        return;
    }

    // send("0:0:0:0");
    send("0");
    */
    
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "q");
}

template <> void
GdbServer::process <'Q'> (string cmd)
{
    auto tokens = split(cmd, ':');
    
    for (auto &it: tokens) {
        printf("> %s <\n", it.c_str());
    }
    
    /*
    if (tokens[0] == "NonStop") {
        
        if (tokens.size() == 2) {
            std::cout << "QNonStop: " << tokens[1] << std::endl;
        }
        send("OK");
    }
    */
    
    if (cmd == "StartNoAckMode") {

        ackMode = false;
        send("OK");
    }
}

template <> void
GdbServer::process <'g'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "g");
    /*
    send("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004091f46cfe7f0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000081125d907f000000020000330000002b00000000000000000000000000000000000000");
    */
}

template <> void
GdbServer::process <'s'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "s");
}

template <> void
GdbServer::process <'n'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "n");
}

template <> void
GdbServer::process <'H'> (string cmd)
{
    send("OK");
}

template <> void
GdbServer::process <'G'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "G");
}

template <> void
GdbServer::process <'?'> (string cmd)
{
    send("S05");
}

template <> void
GdbServer::process <'!'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "!");
}

template <> void
GdbServer::process <'k'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "k");
}

template <> void
GdbServer::process <'m'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "m");

    /*
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 2) {

        // auto t0 = std::stoul(tokens[0], nullptr, 16);
        auto t1 = std::stoul(tokens[1], nullptr, 16);
        
        string result;
        for (int i = 0; i < t1; i++) result += "42";
        send(result);
    }
    */
}

template <> void
GdbServer::process <'M'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "M");
}

template <> void
GdbServer::process <'p'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "p");
}

template <> void
GdbServer::process <'P'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "P");
}

template <> void
GdbServer::process <'c'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "c");
}

template <> void
GdbServer::process <'D'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "D");
}

template <> void
GdbServer::process <'Z'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "Z");

    /*
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 3) {
        
        std::cout << "Break/Watchpoint type " << tokens[0] << std::endl;
        send("OK");

    } else {
    
        std::cout << "Format error " << std::endl;
        send("");
    }
    */
}

template <> void
GdbServer::process <'z'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "z");
}

void
GdbServer::process(char cmd, string arg)
{
    switch (cmd) {
 
        case 'v' : process <'v'> (arg); break;
        case 'q' : process <'q'> (arg); break;
        case 'Q' : process <'Q'> (arg); break;
        case 'g' : process <'g'> (arg); break;
        case 's' : process <'s'> (arg); break;
        case 'n' : process <'n'> (arg); break;
        case 'H' : process <'H'> (arg); break;
        case 'G' : process <'G'> (arg); break;
        case '?' : process <'?'> (arg); break;
        case '!' : process <'!'> (arg); break;
        case 'k' : process <'k'> (arg); break;
        case 'm' : process <'m'> (arg); break;
        case 'M' : process <'M'> (arg); break;
        case 'p' : process <'p'> (arg); break;
        case 'P' : process <'P'> (arg); break;
        case 'c' : process <'c'> (arg); break;
        case 'D' : process <'D'> (arg); break;
        case 'Z' : process <'Z'> (arg); break;
        case 'z' : process <'z'> (arg); break;
            
        default:
            throw VAError(ERROR_GDB_UNRECOGNIZED_CMD, string(1, cmd));
    }
}
