// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EXTFile.h"
#include "Disk.h"
#include "Drive.h"
#include "IO.h"

const std::vector<string> EXTFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

bool
EXTFile::isCompatible(const string &path)
{
    return true;
}

bool
EXTFile::isCompatible(std::istream &stream)
{
    for (auto &header : extAdfHeaders) {
        if (util::matchingStreamHeader(stream, header)) return true;
    }

    return false;
}

void
EXTFile::init(Disk &disk)
{
    debug(true, "EXTFILE::init\n");
    // decodeDisk(disk);
}

void
EXTFile::init(Drive &drive)
{
    if (drive.disk == nullptr) throw VAError(ERROR_DISK_MISSING);
    init(*drive.disk);
}

isize
EXTFile::readFromStream(std::istream &stream)
{
    throw VAError(ERROR_NO_EXTADF_SUPPORT);
}
