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

    // Make Rom writable if an A1000 is emulated
    kickIsWritable = amiga->config.model == A1000;
    
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
AmigaMemory::loadRom(AmigaFile *rom, uint8_t *target, size_t length)
{
    if (rom) {
        
        assert(target != NULL);
        memset(target, 0, length);
        
        int c;
        rom->seek(0);
        for (size_t i = 0; i < length; i++) {
            if ((c = rom->read()) == EOF) break;
            *(target++) = c;
        }
    }
}

void
AmigaMemory::updateMemSrcTable()
{
    // Standard layout after booting
    struct {
        
        uint8_t from; uint8_t to; MemorySource src;
        
    } layout[6] = {
        
        { 0x00, 0x19, MEM_CHIP },
        { 0xA0, 0xBF, MEM_CIA  },
        { 0xDC, 0xDE, amiga->config.realTimeClock ? MEM_RTC : MEM_UNMAPPED },
        { 0xDF, 0xDF, MEM_OCS },
        { 0xF8, 0xFB, kickIsWritable ? MEM_BOOT : MEM_KICK },
        { 0xFC, 0xFF, MEM_KICK }
    };
    
    // Start from scratch
    for (unsigned bank = 0; bank < 256; bank++) {
        memSrc[bank] = MEM_UNMAPPED;
    }
    
    // Setup initial configuration
    for (unsigned i = 0; i < 6; i++)
        for (unsigned bank = layout[i].from; bank <= layout[i].to; bank++)
            memSrc[bank] = layout[i].src;

    // Install Fast Ram
    long fastRamBanks = amiga->getConfig().fastRamSize / 64;
    for (unsigned bank = 0; bank < fastRamBanks; bank++) {
        memSrc[0x20 + bank] = MEM_FAST;
    }
    
    // Overlay Rom with lower memory area if OVL line is high
    bool ovl = true; // TODO: get from CIA
    if (ovl)
        for (unsigned bank = 0; bank < 8; bank++)
            memSrc[bank] = memSrc[0xF8 + bank];
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
            assert(bootRom != NULL);
            return bootRom[addr % bootRomSize];
            
        case MEM_KICK:
            assert(kickRom != NULL);
            return kickRom[addr % kickRomSize];
            
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
