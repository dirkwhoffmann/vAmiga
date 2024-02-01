// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Parser.h"

namespace util {

bool isBool(string& token)
{
    return 
    token == "1" || token == "true" || token == "yes" ||
    token == "0" || token == "false" || token == "no";
}

bool isOnOff(string& token)
{
    return token == "on" || token == "off";
}

bool isNum(string& token)
{
    // Replace leading '$' by '0x'
    if (!token.empty() && token[0] == '$') token = "0x" + token.erase(0, 1);

    try { (void)stol(token, nullptr, 0); }
    catch (std::exception&) { return false; }

    return true;
}

bool
parseBool(string& token)
{
    if (token == "1" || token == "true" || token == "yes") return true;
    if (token == "0" || token == "false" || token == "no") return false;

    assert(!isBool(token));
    throw ParseBoolError(token);
}

bool
parseOnOff(string& token)
{
    if (token == "on") return true;
    if (token == "off") return false;

    assert(!isOnOff(token));
    throw ParseOnOffError(token);
}

long
parseNum(string& token)
{
    long result;

    // Replace leading '$' by '0x'
    if (!token.empty() && token[0] == '$') token = "0x" + token.erase(0, 1);
    
    try { result = stol(token, nullptr, 0); }
    catch (std::exception&) { throw ParseNumError(token); }

    return result;
}

}
