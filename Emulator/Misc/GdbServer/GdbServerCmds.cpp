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

template <> void
GdbServer::process <' ', GdbCmd::CtrlC> (string arg)
{
    debug(GDB_DEBUG, "Ctrl+C\n");
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "CtrlC");
}

template <> void
GdbServer::process <'q', GdbCmd::Supported> (string arg)
{
    send("PacketSize=512;"
         "BreakpointCommands+;"
         "swbreak+;"
         "hwbreak+;"
         "QStartNoAckMode+;"
         "vContSupported+");
}

template <> void
GdbServer::process <'q', GdbCmd::Symbol> (string arg)
{
    send("OK");
}

template <> void
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

template <> void
GdbServer::process <'q', GdbCmd::TStatus> (string arg)
{
    send("T0");
}

template <> void
GdbServer::process <'q', GdbCmd::TfV> (string arg)
{
    send("l");
}

template <> void
GdbServer::process <'q', GdbCmd::TfP> (string arg)
{
    send("l");
}

template <> void
GdbServer::process <'q', GdbCmd::fThreadInfo> (string arg)
{
    send("m01");
}

template <> void
GdbServer::process <'q', GdbCmd::sThreadInfo> (string arg)
{
    send("l");
}

template <> void
GdbServer::process <'q', GdbCmd::Attached> (string arg)
{
    send("0");
}

template <> void
GdbServer::process <'q', GdbCmd::C> (string arg)
{
    send("QC1");
}

template <> void
GdbServer::process <'Q', GdbCmd::StartNoAckMode> (string arg)
{
    ackMode = false;
    send("OK");
}

template <> void
GdbServer::process <'v'> (string arg)
{
    if (arg == "MustReplyEmpty") {
        
        send("");
        return;
    }

    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "v");

    /*
    if (arg == "Cont?") {
        
        return "vCont;c;C;s;S;t;r";
    }
    if (arg == "Cont;c") {
        
        amiga.run();
        return "";
    }
    */
}

template <> void
GdbServer::process <'q'> (string cmd)
{
    auto command = cmd.substr(0, cmd.find(":"));
        
    if (command == "Supported") {
        
        process <'q', GdbCmd::Supported> ("");
        return;
    }
    if (cmd == "Symbol::") {

        process <'q', GdbCmd::Symbol> ("");
        return;
    }
    if (cmd == "Offsets") {

        process <'q', GdbCmd::Offset> ("");
        return;
    }
    if (cmd == "TStatus") {
        
        process <'q', GdbCmd::TStatus> ("");
        return;
    }
    if (cmd == "TfV") {
        
        process <'q', GdbCmd::TfV> ("");
        return;
    }
    if (cmd == "TfP") {
        
        process <'q', GdbCmd::TfP> ("");
        return;
    }
    if (cmd == "fThreadInfo") {
        
        process <'q', GdbCmd::fThreadInfo> ("");
        return;
    }
    if (cmd == "sThreadInfo") {
        
        process <'q', GdbCmd::sThreadInfo> ("");
        return;
    }
    if (command == "Attached") {
        
        process <'q', GdbCmd::Attached> ("");
        return;
    }
    if (command == "C") {
        
        process <'q', GdbCmd::C> ("");
        return;
    }
    
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "q");
}

template <> void
GdbServer::process <'Q'> (string cmd)
{
    auto tokens = util::split(cmd, ':');
               
     if (tokens[0] == "StartNoAckMode") {

         process <'Q', GdbCmd::StartNoAckMode> ("");
         return;
     }
    
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "Q");
}

template <> void
GdbServer::process <'g'> (string cmd)
{
    string result;
    for (int i = 0; i < 18; i++) result += readRegister(i);
    send(result);
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

template <> void
GdbServer::process <'M'> (string cmd)
{
    throw VAError(ERROR_GDB_UNSUPPORTED_CMD, "M");
}

template <> void
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

template <> void
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

void
GdbServer::process(string package)
{
    debug(GDB_DEBUG, "process(%s)\n", package.c_str());
    
    // Check if the previous package has been rejected
    if (package[0] == '-') throw VAError(ERROR_GDB_NO_ACK);

    // Strip off the acknowledgment symbol if present
    if (package[0] == '+') package.erase(0,1);
        
    if (auto len = package.length()) {
        
        // Check for Ctrl+C
        if (package[0] == 0x03) {
            process <' ', GdbCmd::CtrlC> ("");
            return;
        }
        
        // Check for '$x[...]#xx'
        if (package[0] == '$' && len >= 5 && package[len - 3] == '#') {
                                    
            auto cmd = package[1];
            auto arg = package.substr(2, len - 5);
            auto chk = package.substr(len - 2, 2);

            if (verifyChecksum(package.substr(1, len - 4), chk)) {
                
                latestCmd = package;
                
                if (ackMode) remoteServer.send(SRVMODE_GDB, "+");
                process(cmd, arg);
                
            } else {
                
                if (ackMode) remoteServer.send(SRVMODE_GDB, "-");
                throw VAError(ERROR_GDB_INVALID_CHECKSUM);
            }
            
            return;
        }
        
        throw VAError(ERROR_GDB_INVALID_FORMAT);
    }
}

void
GdbServer::process(char cmd, string package)
{
    switch (cmd) {
 
        case 'v' : process <'v'> (package); break;
        case 'q' : process <'q'> (package); break;
        case 'Q' : process <'Q'> (package); break;
        case 'g' : process <'g'> (package); break;
        case 's' : process <'s'> (package); break;
        case 'n' : process <'n'> (package); break;
        case 'H' : process <'H'> (package); break;
        case 'G' : process <'G'> (package); break;
        case '?' : process <'?'> (package); break;
        case '!' : process <'!'> (package); break;
        case 'k' : process <'k'> (package); break;
        case 'm' : process <'m'> (package); break;
        case 'M' : process <'M'> (package); break;
        case 'p' : process <'p'> (package); break;
        case 'P' : process <'P'> (package); break;
        case 'c' : process <'c'> (package); break;
        case 'D' : process <'D'> (package); break;
        case 'Z' : process <'Z'> (package); break;
        case 'z' : process <'z'> (package); break;
            
        default:
            throw VAError(ERROR_GDB_UNRECOGNIZED_CMD, string(1, cmd));
    }
}

