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
    // Allocate memory
    allocateBootRom();
    allocateKickRom();
    allocateChipRam(amiga->config.chipRamSize);
    allocateFastRam(amiga->config.fastRamSize);
    allocateSlowRam(amiga->config.slowRamSize);

    // Set up the memory lookup table
    updateMemSrcTable();
    
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
    plainmsg("     Kick Rom: %d KB (%s)\n", kickRomSize >> 10,
             kickIsWritable ? "unlocked" : "locked");
    plainmsg("     Chip Ram: %d KB\n", chipRamSize >> 10);
    plainmsg("     Slow Ram: %d KB\n", slowRamSize >> 10);
    plainmsg("     Fast Ram: %d KB\n", fastRamSize >> 10);
}

bool
AmigaMemory::allocateBootRom()
{
    // Only the Amiga 1000 has a Boot Rom
    if (amiga->config.model == A1000) {
        return alloc(KB(64), bootRom, bootRomSize);
    }
    dealloc(bootRom, bootRomSize);
    return true;
}

bool
AmigaMemory::allocateKickRom()
{
    return alloc(KB(256), kickRom, kickRomSize);
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
    dealloc(kickRom, kickRomSize);
    dealloc(chipRam, chipRamSize);
    dealloc(slowRam, slowRamSize);
    dealloc(fastRam, fastRamSize);
}

bool
AmigaMemory::alloc(size_t size, uint8_t *&ptrref, size_t &sizeref)
{
    // Do some consistency checking
    assert((ptrref == NULL) == (sizeref == 0));
    assert(size != 0);
    
    // Delete previously allocated memory
    if (ptrref) delete[] ptrref;
    
    // Try to allocate memory
    if (!(ptrref = new (std::nothrow) uint8_t[KB(size)])) {
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
        ptrref = NULL;
        sizeref = 0;
    }
}

void
AmigaMemory::updateMemSrcTable()
{
    long chipRamBanks = amiga->config.chipRamSize / 64;
    assert(chipRamBanks == 4 || chipRamBanks == 8);
    
    // Start from scratch
    for (unsigned bank = 0; bank < 256; bank++) {
        memSrc[bank] = MEM_UNMAPPED;
    }
    
    // TODO: HOW DOES THE MIRRORED CHIP RAM LOOK IF ONLY 256KB are present?
    // Chip Ram
    for (unsigned bank = 0; bank < chipRamBanks; bank++) {
        memSrc[bank] = MEM_CHIP;
    }

    // Chip Ram (mirrored)
    for (unsigned bank = 0; bank < chipRamBanks; bank++) {
        memSrc[0x08 + bank] = MEM_CHIP;
    }

    // Fast Ram
    long fastRamBanks = amiga->getConfig().fastRamSize / 64;
    for (unsigned bank = 0; bank < fastRamBanks; bank++) {
        memSrc[0x20 + bank] = MEM_FAST;
    }

    // CIA
    for (unsigned bank = 0; bank < 32; bank++) {
        memSrc[0xA0 + bank] = MEM_CIA;
    }

    // Slow Ram
    long slowRamBanks = amiga->getConfig().slowRamSize / 64;
    for (unsigned bank = 0; bank < slowRamBanks; bank++) {
        memSrc[0xC0 + bank] = MEM_SLOW;
    }

    // Real-time clock
    if (amiga->getConfig().realTimeClock) {
        for (unsigned bank = 0; bank < 3; bank++) {
            memSrc[0xDC + bank] = MEM_RTC;
        }
    }
    
    // Custom chips
    memSrc[0xDF] = MEM_OCS;
    
    // Kickstart Rom
    for (unsigned bank = 0; bank < 8; bank++) {
        memSrc[0xF8 + bank] = MEM_KICK;
    }
}

MemorySource
AmigaMemory::getMemSrc(uint32_t addr)
{
    assert(is_uint24_t(addr));
    return memSrc[addr >> 16];
}

uint8_t
AmigaMemory::peek8(uint32_t addr)
{
    assert(is_uint24_t(addr));
    
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED:
            return 0;
            
        case MEM_CHIP:
            assert(chipRam != NULL);
            return chipRam[(addr % 0xFFFF) % chipRamSize];
            
        case MEM_FAST:
            assert(fastRam != NULL);
            return fastRam[(addr % 0xFFFF) % fastRamSize];
            
        case MEM_CIA:
            return 42;
            
        case MEM_SLOW:
            assert(slowRam != NULL);
            return slowRam[(addr % 0xFFFF) % slowRamSize];
            
        case MEM_RTC:
            return 1;
            
        case MEM_OCS:
            return 2;
            
        case MEM_BOOT:
        case MEM_KICK:
            assert(kickRom != NULL);
            return kickRom[(addr % 0xFFFF) % kickRomSize];
            
        default:
            assert(false);
    }
    return 0;
}

uint16_t
AmigaMemory::peek16(uint32_t addr)
{
    return HI_LO(peek8(addr), peek8(addr + 1));
}

uint8_t
AmigaMemory::spypeek8(uint32_t addr)
{
    assert(is_uint24_t(addr));
    return peek8(addr);
}
uint16_t
AmigaMemory::spypeek16(uint32_t addr)
{
    assert(is_uint24_t(addr));
    return peek16(addr); 
}

const char *
AmigaMemory::ascii(uint32_t addr)
{
    assert(is_uint24_t(addr));
    
    for (unsigned i = 0; i < 16; i++) {
        uint8_t value = peek8(addr + i);
        str[i] = isprint(value) ? value : '.';
    }
    str[16] = 0;
    return str;
}

void
AmigaMemory::poke8(uint32_t addr, uint8_t value)
{
    assert(is_uint24_t(addr));
    debug("Poking %02X to %06X.", value, addr);
}
void
AmigaMemory::poke16(uint32_t addr, uint16_t value)
{
    assert(is_uint24_t(addr));
    debug("Poking %04X to %06X.", value, addr);
}
