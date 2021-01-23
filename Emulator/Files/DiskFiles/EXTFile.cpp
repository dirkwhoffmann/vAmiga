// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

const u8 EXTFile::extAdfHeaders[2][8] = {

    { 'U', 'A', 'E', '-', '-', 'A', 'D', 'F' },
    { 'U', 'A', 'E', '-', '1', 'A', 'D', 'F' }
};
    
bool
EXTFile::isCompatibleName(const std::string &name)
{
    return true;
}

bool
EXTFile::isCompatibleStream(std::istream &stream)
{
    usize length = streamLength(stream);
    
    size_t len = sizeof(extAdfHeaders[0]);
    size_t cnt = sizeof(extAdfHeaders) / len;

    if (length < len) return false;
    
    for (size_t i = 0; i < cnt; i++) {
        if (matchingStreamHeader(stream, extAdfHeaders[i], len)) return true;
    }
    return false;
}
