// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AmigaDrive::AmigaDrive()
{
    setDescription("Drive");
}

void
AmigaDrive::_powerOn()
{
    
}

void
AmigaDrive::_powerOff()
{
    
}

void
AmigaDrive::_reset()
{
    
}

void
AmigaDrive::_ping()
{
    // TODO: hasDisk, writeProtected, unsaved
}

void
AmigaDrive::_dump()
{
    msg("Has disk: %s\n", hasDisk() ? "yes" : "no");
}

void
AmigaDrive::ejectDisk()
{
    if (disk) {
        delete disk;
        disk = NULL;
    }
}

void
AmigaDrive::insertDisk(AmigaDisk *newDisk)
{
    if (newDisk) {
        ejectDisk();
        disk = newDisk;
    }
}

void
AmigaDrive::insertDisk(ADFFile *file)
{
    // Convert ADF file into a disk
    // AmigaDisk = new AmigaDisk::makeWithFile(ADFFile *file)
    AmigaDisk *newDisk = new AmigaDisk();
    
    insertDisk(newDisk);
}
