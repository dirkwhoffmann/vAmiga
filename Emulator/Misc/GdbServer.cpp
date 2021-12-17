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

GdbServer::GdbServer(Amiga& ref) : SubComponent(ref)
{
}

GdbServer::~GdbServer()
{
}

string
GdbServer::checksum(const string &s)
{
    uint8_t chk = 0;
    for(auto &c : s) chk += (uint8_t)c;

    return util::hexstr <2> (chk);
}

std::vector<string>
GdbServer::split(const string &s, char delimiter)
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
GdbServer::readRegister(isize nr)
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
GdbServer::readMemory(isize addr)
{
    auto byte = mem.spypeek8 <ACCESSOR_CPU> ((u32)addr);
    return util::hexstr <2> (byte);
}

string
GdbServer::process(string packet)
{
    // string result;
    
    // Check if the previous package has been rejected
    if (packet[0] == '-') throw VAError(ERROR_GDB_NO_ACK);

    // Strip off the acknowledgment symbol if present
    if (packet[0] == '+') packet.erase(0,1);
        
    if (auto len = packet.length()) {
        
        // Check for Ctrl+C
        if (packet[0] == 0x03) {
            printf("Ctrl+C\n");
            return "";
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
                return process(cmd, arg);
            
            } else {

                throw VAError(ERROR_GDB_INVALID_CHECKSUM);
            }
                            
            return "";
        }
       
        printf("ERROR_GDB_INVALID_FORMAT\n");
        throw VAError(ERROR_GDB_INVALID_FORMAT);
    }
    
    return "";
}

template <> string
GdbServer::processCmd <GdbCmd::qSupported> (string arg)
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
GdbServer::processCmd <GdbCmd::qSymbol> (string arg)
{
    return "OK";
}

template <> string
GdbServer::processCmd <GdbCmd::qOffset> (string arg)
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
GdbServer::processCmd <GdbCmd::qTStatus> (string arg)
{
    return "T0";
}

template <> string
GdbServer::processCmd <GdbCmd::qTfV> (string arg)
{
    return "l";
}

template <> string
GdbServer::processCmd <GdbCmd::qTfP> (string arg)
{
    return "l";
}

template <> string
GdbServer::processCmd <GdbCmd::qfThreadInfo> (string arg)
{
    return "m01";
}

template <> string
GdbServer::processCmd <GdbCmd::qsThreadInfo> (string arg)
{
    return "l";
}

template <> string
GdbServer::processCmd <GdbCmd::qAttached> (string arg)
{
    return "0";
}

template <> string
GdbServer::processCmd <GdbCmd::qC> (string arg)
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
        
        return processCmd <GdbCmd::qSupported> ("");
    }
    if (cmd == "Symbol::") {

        return processCmd <GdbCmd::qSymbol> ("");
    }
    if (cmd == "Offsets") {

        return processCmd <GdbCmd::qOffset> ("");
    }
    if (cmd == "TStatus") {
        
        return processCmd <GdbCmd::qTStatus> ("");
    }
    if (cmd == "TfV") {
        
        return processCmd <GdbCmd::qTfV> ("");
    }
    if (cmd == "TfP") {
        
        return processCmd <GdbCmd::qTfP> ("");
    }
    if (cmd == "fThreadInfo") {
        
        return processCmd <GdbCmd::qfThreadInfo> ("");
    }
    if (cmd == "sThreadInfo") {
        
        return processCmd <GdbCmd::qsThreadInfo> ("");
    }
    if (command == "Attached") {
        
        return processCmd <GdbCmd::qAttached> ("");
    }
    if (command == "C") {
        
        return processCmd <GdbCmd::qC> ("");
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
GdbServer::process(char cmd, string arg)
{
    switch (cmd) {
 
        case 'v' : return process <'v'> (arg);
        case 'q' : return process <'q'> (arg);
        case 'Q' : return process <'Q'> (arg);
        case 'g' : return process <'g'> (arg);
        case 's' : return process <'s'> (arg);
        case 'n' : return process <'n'> (arg);
        case 'H' : return process <'H'> (arg);
        case 'G' : return process <'G'> (arg);
        case '?' : return process <'?'> (arg);
        case '!' : return process <'!'> (arg);
        case 'k' : return process <'k'> (arg);
        case 'm' : return process <'m'> (arg);
        case 'M' : return process <'M'> (arg);
        case 'p' : return process <'p'> (arg);
        case 'P' : return process <'P'> (arg);
        case 'c' : return process <'c'> (arg);
        case 'D' : return process <'D'> (arg);
        case 'Z' : return process <'Z'> (arg);
        case 'z' : return process <'z'> (arg);
            
        default:
            throw VAError(ERROR_GDB_UNRECOGNIZED_CMD, string(1, cmd));
    }
}
