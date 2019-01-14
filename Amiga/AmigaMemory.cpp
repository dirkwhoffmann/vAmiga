// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include <new>

AmigaMemory::AmigaMemory()
{
    setDescription("Memory");
}

AmigaMemory::~AmigaMemory()
{
    dealloc();
}

void
AmigaMemory::_powerOn()
{
    allocateBootRom();
    allocateKickstartRom();
    allocateChipRam(amiga->config.chipRamSize);
    
}

void
AmigaMemory::_powerOff()
{
    dealloc();
}

void
AmigaMemory::_reset()
{
    
}

void
AmigaMemory::_ping()
{
    
}

void
AmigaMemory::_dump()
{
    plainmsg("     Boot Rom: %d KB\n", bootRomSize >> 10);
    plainmsg("Kickstart Rom: %d KB (%s)\n", kickstartRomSize >> 10,
             kickstartIsWritable ? "unlocked" : "locked");
    plainmsg("     Chip Ram: %d KB\n", chipRamSize >> 10);
    plainmsg("     Slow Ram: %d KB\n", slowRamSize >> 10);
    plainmsg("     Fast Ram: %d KB\n", fastRamSize >> 10);
}

bool
AmigaMemory::allocateBootRom()
{
    return alloc(KB(64), bootRom, bootRomSize);
}

bool
AmigaMemory::allocateKickstartRom()
{
    return alloc(KB(256), bootRom, bootRomSize);
}

bool
AmigaMemory::allocateChipRam(size_t size)
{
    return alloc(size, chipRam, chipRamSize);
}

bool
AmigaMemory::allocateSlowRam(size_t size)
{
    return alloc(size, slowRam, slowRamSize);
}

bool
AmigaMemory::allocateFastRam(size_t size)
{
    return alloc(size, fastRam, fastRamSize);
}

void
AmigaMemory::dealloc()
{
    dealloc(bootRom, bootRomSize);
    dealloc(kickstartRom, kickstartRomSize);
    dealloc(chipRam, chipRamSize);
    dealloc(slowRam, slowRamSize);
    dealloc(fastRam, fastRamSize);
}

bool
AmigaMemory::alloc(size_t size, uint8_t *&ptrref, size_t &sizeref)
{
    // Do some consistency checking
    assert((ptrref == NULL) == (sizeref == 0));
    
    // Delete previously allocated memory
    if (ptrref) delete[] ptrref;
    
    // Try to allocate memory
    if (!(bootRom = new (std::nothrow) uint8_t[KB(64)])) {
        warn("Cannot allocate %X bytes of memory\n", size);
        return false;
    }
    
    sizeref = size;
    return true;
}

void
AmigaMemory::dealloc(uint8_t *&ptrref, size_t &sizeref)
{
    // Do some consistency checking
    assert((ptrref == NULL) == (sizeref == 0));
    
    if (ptrref) {
        delete[] ptrref;
        sizeref = 0;
    }
}

uint8_t
AmigaMemory::peek8(uint32_t addr)
{
    return 42;
}

uint8_t
AmigaMemory::spypeek8(uint32_t addr)
{
    return 43;
}

void
AmigaMemory::poke8(uint32_t addr, uint8_t value)
{
    
}

