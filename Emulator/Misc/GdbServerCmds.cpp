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
#include "Amiga.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"

template <> string
GdbServer::process <' ', GdbCmd::CtrlC> (string arg)
{
    debug(GDB_DEBUG, "Ctrl+C\n");
    return "";
}

template <> string
GdbServer::process <'q', GdbCmd::Supported> (string arg)
{
    return
    "PacketSize=512;"
    "BreakpointCommands+;"
    "swbreak+;"
    "hwbreak+;"
    "QStartNoAckMode+;"
    "vContSupported+";
}

template <> string
GdbServer::process <'q', GdbCmd::Symbol> (string arg)
{
    return "OK";
}

template <> string
GdbServer::process <'q', GdbCmd::Offset> (string arg)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "qOffset");

    /*
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

    return result;
    */
}

template <> string
GdbServer::process <'q', GdbCmd::TStatus> (string arg)
{
    return "T0";
}

template <> string
GdbServer::process <'q', GdbCmd::TfV> (string arg)
{
    return "l";
}

template <> string
GdbServer::process <'q', GdbCmd::TfP> (string arg)
{
    return "l";
}

template <> string
GdbServer::process <'q', GdbCmd::fThreadInfo> (string arg)
{
    return "m01";
}

template <> string
GdbServer::process <'q', GdbCmd::sThreadInfo> (string arg)
{
    return "l";
}

template <> string
GdbServer::process <'q', GdbCmd::Attached> (string arg)
{
    return "0";
}

template <> string
GdbServer::process <'q', GdbCmd::C> (string arg)
{
    return "QC1";
}


template <> string
GdbServer::process <'v'> (string arg)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "v");

    /*
    if (arg == "MustReplyEmpty") {
        
        return "";
    }
    if (arg == "Cont?") {
        
        return "vCont;c;C;s;S;t;r";
    }
    if (arg == "Cont;c") {
        
        amiga.run();
        return "";
    }
    */
}

template <> string
GdbServer::process <'q'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "q");

    /*
    auto command = cmd.substr(0, cmd.find(":"));
        
    if (command == "Supported") {
        
        return processCmd <GdbCmd::Supported> ("");
    }
    if (cmd == "Symbol::") {

        return processCmd <GdbCmd::Symbol> ("");
    }
    if (cmd == "Offsets") {

        return processCmd <GdbCmd::Offset> ("");
    }
    if (cmd == "TStatus") {
        
        return processCmd <GdbCmd::TStatus> ("");
    }
    if (cmd == "TfV") {
        
        return processCmd <GdbCmd::TfV> ("");
    }
    if (cmd == "TfP") {
        
        return processCmd <GdbCmd::TfP> ("");
    }
    if (cmd == "fThreadInfo") {
        
        return processCmd <GdbCmd::fThreadInfo> ("");
    }
    if (cmd == "sThreadInfo") {
        
        return processCmd <GdbCmd::sThreadInfo> ("");
    }
    if (command == "Attached") {
        
        return processCmd <GdbCmd::Attached> ("");
    }
    if (command == "C") {
        
        return processCmd <GdbCmd::C> ("");
    }
    
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "q");
    */
}

template <> string
GdbServer::process <'Q'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "Q");
}

template <> string
GdbServer::process <'g'> (string cmd)
{
    string result;
    for (int i = 0; i < 18; i++) result += readRegister(i);
    return result;
}

template <> string
GdbServer::process <'s'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "s");
}

template <> string
GdbServer::process <'n'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "n");
}

template <> string
GdbServer::process <'H'> (string cmd)
{
    return "OK";
}

template <> string
GdbServer::process <'G'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "G");
}

template <> string
GdbServer::process <'?'> (string cmd)
{
    return "S05";
}

template <> string
GdbServer::process <'!'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "!");
}

template <> string
GdbServer::process <'k'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "k");
}

template <> string
GdbServer::process <'m'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "m");

    /*
    auto tokens = split(cmd, ',');
    
    if (tokens.size() == 2) {

        string result;

        isize addr;
        util::parseHex(tokens[0], &addr);
        isize size;
        util::parseHex(tokens[1], &size);

        for (isize i = 0; i < size; i++) {
            result += readMemory(addr + i);
        }
        
        send(result);
    }
    */
}

template <> string
GdbServer::process <'M'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "M");
}

template <> string
GdbServer::process <'p'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "p");

    /*
    isize nr;
    util::parseHex(cmd, &nr);
    
    printf("p command: nr = %ld\n", nr);
    send(readRegister(nr));
    */
}

template <> string
GdbServer::process <'P'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "P");
}

template <> string
GdbServer::process <'c'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "c");
}

template <> string
GdbServer::process <'D'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "D");
}

template <> string
GdbServer::process <'Z'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "Z");

    /*
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
    */
}

template <> string
GdbServer::process <'z'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "z");

    /*
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
    */
}

string
GdbServer::process(string payload)
{
    // Check if the previous package has been rejected
    if (payload[0] == '-') throw VAError(ERROR_GDB_NO_ACK);

    // Strip off the acknowledgment symbol if present
    if (payload[0] == '+') payload.erase(0,1);
        
    if (auto len = payload.length()) {
        
        // Check for Ctrl+C
        if (payload[0] == 0x03) {
            return "Ctrl+C";
        }
        
        // Check for '$x[...]#xx'
        if (payload[0] == '$' && len >= 5 && payload[len - 3] == '#') {
                                    
            auto cmd = payload[1];
            auto arg = payload.substr(2, len - 5);
            auto chk = payload.substr(len - 2, 2);

            if (verifyChecksum(payload.substr(1, len - 4), chk)) {
                
                // Remember the command
                latestCmd = payload;
                                
                // Compute the answer string
                auto result = process(cmd, arg);
                
                // Convert the answer string into a packet
                result = "$" + result + computeChecksum(result);

                // Acknowledge the command (if requesteda)
                if (ackMode) result = "+" + result;

            } else {
                
                throw VAError(ERROR_GDB_INVALID_CHECKSUM);
            }
        }
       
        throw VAError(ERROR_GDB_INVALID_FORMAT);
    }
    
    return "";
}

string
GdbServer::process(char cmd, string payload)
{
    switch (cmd) {
 
        case 'v' : return process <'v'> (payload);
        case 'q' : return process <'q'> (payload);
        case 'Q' : return process <'Q'> (payload);
        case 'g' : return process <'g'> (payload);
        case 's' : return process <'s'> (payload);
        case 'n' : return process <'n'> (payload);
        case 'H' : return process <'H'> (payload);
        case 'G' : return process <'G'> (payload);
        case '?' : return process <'?'> (payload);
        case '!' : return process <'!'> (payload);
        case 'k' : return process <'k'> (payload);
        case 'm' : return process <'m'> (payload);
        case 'M' : return process <'M'> (payload);
        case 'p' : return process <'p'> (payload);
        case 'P' : return process <'P'> (payload);
        case 'c' : return process <'c'> (payload);
        case 'D' : return process <'D'> (payload);
        case 'Z' : return process <'Z'> (payload);
        case 'z' : return process <'z'> (payload);
            
        default:
            throw VAError(ERROR_GDB_UNRECOGNIZED_CMD, string(1, cmd));
    }
}

