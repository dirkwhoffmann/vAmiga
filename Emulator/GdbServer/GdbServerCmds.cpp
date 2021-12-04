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

void
GdbServer::process(char cmd, string arg)
{
    switch (cmd) {
 
            /*
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
             */
            
        default:
            throw VAError(ERROR_GDB_UNSUPPORTED_CMD, string(1, cmd));
    }
}

string
GdbServer::checksum(const string &s)
{
    uint8_t chk = 0;
    for(auto &c : s) chk += (uint8_t)c;

    return util::hexstr <2> (chk);
}
