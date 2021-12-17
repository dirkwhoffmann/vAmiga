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
#include "IOUtils.h"
#include "MemUtils.h"
#include "CPU.h"

void
RemoteServer::process(string packet)
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

                latestCmd = packet;
                
                if (ackMode) connection.send("+");
                process(cmd, arg);
            
            } else {

                if (ackMode) connection.send("-");
                throw VAError(ERROR_GDB_INVALID_CHECKSUM);
            }
                            
            return;
        }
       
        printf("ERROR_GDB_INVALID_FORMAT\n");
        // throw VAError(ERROR_GDB_INVALID_FORMAT);
    }
}

template <> void
RemoteServer::processCmd <GdbCmd::qSupported> (string arg)
{
    send("PacketSize=512;"
         "BreakpointCommands+;"
         "swbreak+;"
         "hwbreak+;"
         "QStartNoAckMode+;"
         "vContSupported+"
         // "QTFrame+"
         );
}

template <> void
RemoteServer::processCmd <GdbCmd::qSymbol> (string arg)
{
    send("OK");
}

template <> void
RemoteServer::processCmd <GdbCmd::qOffset> (string arg)
{
    string result;
    
    os::Process p;
    if (osDebugger.searchProcess(debugProcess, p)) {
                
        os::SegList segList;
        osDebugger.read(p, segList);
        
        for (usize i = 0; i < segList.size(); i++) {
            
            switch (i) {
                    
                case 0:
                    result += "TextSeg=";
                    result += util::hexstr <6> (segList[i].first);
                    break;
                case 1:
                    result += ";DataSeg=";
                    result += util::hexstr <6> (segList[i].first);
                    break;
                default:
                    // result += ";CustomSeg" + std::to_string(i) + "=";
                    break;
            }
        }
    }
    
    if (result.empty()) {
        
        printf("Postponing qOffset command\n");
        
        // Try this command a little later
        agnus.scheduleRel<SLOT_GDB>(SEC(0.5), GDB_PENDING);
        return;
    }
        
    send(result);
}

template <> void
RemoteServer::processCmd <GdbCmd::qTStatus> (string arg)
{
    send("T0");
}

template <> void
RemoteServer::processCmd <GdbCmd::qTfV> (string arg)
{
    send("l");
}

template <> void
RemoteServer::processCmd <GdbCmd::qTfP> (string arg)
{
    send("l");
}

template <> void
RemoteServer::processCmd <GdbCmd::qfThreadInfo> (string arg)
{
    send("m01");
}

template <> void
RemoteServer::processCmd <GdbCmd::qsThreadInfo> (string arg)
{
    send("l");
}

template <> void
RemoteServer::processCmd <GdbCmd::qAttached> (string arg)
{
    send("0");
}

template <> void
RemoteServer::processCmd <GdbCmd::qC> (string arg)
{
    send("QC1");
}


template <> void
RemoteServer::process <'v'> (string arg)
{
    printf("process <'v'>: %s\n", arg.c_str());
    
    if (arg == "MustReplyEmpty") {
        
        send("");
        return;
    }
    if (arg == "Cont?") {
        
        send("vCont;c;C;s;S;t;r");
        return;
    }
    if (arg == "Cont;c") {
        
        amiga.run();
        return;
    }
}

template <> void
RemoteServer::process <'q'> (string cmd)
{
    auto command = cmd.substr(0, cmd.find(":"));
        
    if (command == "Supported") {
        
        processCmd <GdbCmd::qSupported> ("");
        return;
    }
    if (cmd == "Symbol::") {

        processCmd <GdbCmd::qSymbol> ("");
        return;
    }
    if (cmd == "Offsets") {

        processCmd <GdbCmd::qOffset> ("");
        return;
    }
    if (cmd == "TStatus") {
        
        processCmd <GdbCmd::qTStatus> ("");
        return;
    }
    if (cmd == "TfV") {
        
        processCmd <GdbCmd::qTfV> ("");
        return;
    }
    if (cmd == "TfP") {
        
        processCmd <GdbCmd::qTfP> ("");
        return;
    }
    if (cmd == "fThreadInfo") {
        
        processCmd <GdbCmd::qfThreadInfo> ("");
        return;
    }
    if (cmd == "sThreadInfo") {
        
        processCmd <GdbCmd::qsThreadInfo> ("");
        return;
    }
    if (command == "Attached") {
        
        processCmd <GdbCmd::qAttached> ("");
        return;
    }
    if (command == "C") {
        
        processCmd <GdbCmd::qC> ("");
        return;
    }
    
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "q");
}

template <> void
RemoteServer::process <'Q'> (string cmd)
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
RemoteServer::process <'g'> (string cmd)
{
    string result;
    for (int i = 0; i < 18; i++) result += readRegister(i);
    send(result);
}

template <> void
RemoteServer::process <'s'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "s");
}

template <> void
RemoteServer::process <'n'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "n");
}

template <> void
RemoteServer::process <'H'> (string cmd)
{
    send("OK");
}

template <> void
RemoteServer::process <'G'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "G");
}

template <> void
RemoteServer::process <'?'> (string cmd)
{
    send("S05");
}

template <> void
RemoteServer::process <'!'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "!");
}

template <> void
RemoteServer::process <'k'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "k");
}

template <> void
RemoteServer::process <'m'> (string cmd)
{
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 2) {

        string result;

        printf("Token0 = %s\n", tokens[0].c_str());
        printf("Token1 = %s\n", tokens[1].c_str());
        isize addr;
        util::parseHex(tokens[0], &addr);
        isize size;
        util::parseHex(tokens[1], &size);
        printf("addr = %ld\n", addr);
        printf("size = %ld\n", size);

        for (isize i = 0; i < size; i++) {
            result += readMemory(addr + i);
        }
        
        send(result);
    }
}

template <> void
RemoteServer::process <'M'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "M");
}

template <> void
RemoteServer::process <'p'> (string cmd)
{
    isize nr;
    util::parseHex(cmd, &nr);
    
    printf("p command: nr = %ld\n", nr);
    send(readRegister(nr));
}

template <> void
RemoteServer::process <'P'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "P");
}

template <> void
RemoteServer::process <'c'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "c");
}

template <> void
RemoteServer::process <'D'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "D");
}

template <> void
RemoteServer::process <'Z'> (string cmd)
{
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 3) {

        auto type = std::stol(tokens[0]);
        auto addr = std::stol(tokens[1], 0, 16);
        auto kind = std::stol(tokens[2]);
        
        printf("Z: type = %ld addr = $%lx kind = %ld\n", type, addr, kind);
        
        if (type == 0) {
         
            cpu.debugger.breakpoints.addAt((u32)addr);
        }
        
        send("OK");
        return;
    }

    // throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "Z");
    printf("ERROR_GDB_INVALID_FORMAT\n");
    send("OK");
}

template <> void
RemoteServer::process <'z'> (string cmd)
{
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 3) {

        auto type = std::stol(tokens[0]);
        auto addr = std::stol(tokens[1], 0, 16);
        auto kind = std::stol(tokens[2]);
        
        printf("z: type = %ld addr = $%lx kind = %ld\n", type, addr, kind);
        
        if (type == 0) {
         
            cpu.debugger.breakpoints.removeAt((u32)addr);
        }
        
        send("OK");
        return;
    }

    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "z");
}

void
RemoteServer::process(char cmd, string arg)
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
