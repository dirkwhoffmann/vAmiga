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
    plainmsg("     Boot Rom: %d KB at %p\n", bootRomSize >> 10, bootRom);
    plainmsg("     Kick Rom: %d KB (%s) at %p\n", kickRomSize >> 10,
             kickIsWritable ? "unlocked" : "locked", kickRom);
    plainmsg("     Chip Ram: %d KB at %p\n", chipRamSize >> 10, chipRam);
    plainmsg("     Slow Ram: %d KB at %p\n", slowRamSize >> 10, slowRam);
    plainmsg("     Fast Ram: %d KB at %p\n", fastRamSize >> 10, fastRam);
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

    updateMemSrcTable();
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
    MemorySource mem_boot = bootRom ? MEM_BOOT : MEM_UNMAPPED;
    MemorySource mem_kick = kickRom ? MEM_KICK : MEM_UNMAPPED;

    // Start from scratch
    for (unsigned bank = 0x00; bank <= 0xFF; bank++)
        memSrc[bank] = MEM_UNMAPPED;
    
    // Chip Ram
    for (unsigned bank = 0x00; bank <= 0x19; bank++)
        memSrc[bank] = chipRam ? MEM_CHIP : MEM_UNMAPPED;
    
    // Install Fast Ram
    for (unsigned bank = 0; bank < amiga->config.fastRamSize / 64; bank++)
        memSrc[0x20 + bank] = MEM_FAST;

    // CIA range
    for (unsigned bank = 0xA0; bank <= 0xBF; bank++)
        memSrc[bank] = MEM_CIA;

    // Slow Ram
    for (unsigned bank = 0; bank < amiga->config.slowRamSize / 64; bank++)
        memSrc[0xC0 + bank] = MEM_SLOW;

    // Real-time clock
    for (unsigned bank = 0xDC; bank <= 0xDE; bank++)
        memSrc[bank] = amiga->config.realTimeClock ? MEM_RTC : MEM_UNMAPPED;

    // OCS
    for (unsigned bank = 0xDF; bank <= 0xDF; bank++)
        memSrc[bank] = MEM_OCS;

    // Boot Rom or Kickstart mirror
    for (unsigned bank = 0xF8; bank <= 0xFB; bank++)
        memSrc[bank] = kickIsWritable ? mem_boot : mem_kick;

    // Kickstart
    for (unsigned bank = 0xFC; bank <= 0xFF; bank++)
        memSrc[bank] = mem_kick;

    // Overlay Rom with lower memory area if OVL line is high
    bool ovl = true; // TODO: get from CIA
    if (ovl)
        for (unsigned bank = 0; bank < 8; bank++)
            memSrc[bank] = memSrc[0xF8 + bank];
    
    amiga->putMessage(MSG_MEM_LAYOUT);
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
    addr &= 0xFFFFFF;
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
    addr &= 0xFFFFFF;
    return HI_LO(peek8(addr), peek8(addr+1));
}

uint32_t
AmigaMemory::peek32(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return HI_HI_LO_LO(peek8(addr), peek8(addr+1), peek8(addr+2), peek8(addr+3));
}

uint8_t
AmigaMemory::spypeek8(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek8(addr);
}
uint16_t
AmigaMemory::spypeek16(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek16(addr);
}
uint32_t
AmigaMemory::spypeek32(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek32(addr);
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
void
AmigaMemory::poke32(uint32_t addr, uint32_t value)
{
    assert(is_uint24_t(addr));
    debug("Poking %04X to %06X.", value, addr);
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

const char *
AmigaMemory::hex(uint32_t addr, size_t bytes)
{
    assert(is_uint24_t(addr));
    assert(bytes % 2 == 0);
    assert(bytes <= 32);
    
    char *ptr = str;
    for (unsigned i = 0; i < bytes / 2; i++) {
        
        uint16_t value = peek16(addr + i);
        sprint16x(ptr, value);
        ptr += 4;
        *ptr++ = ' ';
    }
    *ptr = 0;
    return str;
}
